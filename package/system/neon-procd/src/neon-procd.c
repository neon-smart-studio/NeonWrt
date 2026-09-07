#define _GNU_SOURCE
#include <ctype.h>
#include <errno.h>
#include <dirent.h>
#include <fnmatch.h>
#include <sys/stat.h>
#include <json-c/json.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

#define MAX_ARGS 512
#define UNIT_PREFIX "neon-procd-"
#define REGISTRY_DIR "/run/neon-procd/services"

static void die(const char *msg)
{
    fprintf(stderr, "neon-procd: %s\n", msg);
    exit(1);
}

static char *read_stdin_all(void)
{
    size_t cap = 4096, len = 0;
    char *buf = malloc(cap);
    if (!buf) die("out of memory");
    for (;;) {
        if (len + 2048 + 1 > cap) {
            cap *= 2;
            char *n = realloc(buf, cap);
            if (!n) { free(buf); die("out of memory"); }
            buf = n;
        }
        ssize_t r = read(STDIN_FILENO, buf + len, cap - len - 1);
        if (r < 0) {
            if (errno == EINTR) continue;
            free(buf); die("failed to read stdin");
        }
        if (r == 0) break;
        len += (size_t)r;
    }
    buf[len] = '\0';
    return buf;
}

static void sanitize(const char *in, char *out, size_t outsz)
{
    size_t j = 0;
    if (!in || !*in) in = "unnamed";
    for (size_t i = 0; in[i] && j + 1 < outsz; i++) {
        unsigned char c = (unsigned char)in[i];
        if (isalnum(c) || c == '_' || c == '-' || c == '.') out[j++] = (char)c;
        else out[j++] = '-';
    }
    out[j] = '\0';
}

static void unit_name(const char *service, const char *instance, char *out, size_t outsz)
{
    char s[128], i[128];
    sanitize(service, s, sizeof(s));
    sanitize(instance, i, sizeof(i));
    snprintf(out, outsz, UNIT_PREFIX "%s-%s.service", s, i);
}

static int runv(char *const argv[], bool quiet)
{
    pid_t p = fork();
    if (p < 0) return -1;
    if (p == 0) {
        if (quiet) {
            FILE *f = fopen("/dev/null", "w");
            if (f) {
                dup2(fileno(f), STDOUT_FILENO);
                dup2(fileno(f), STDERR_FILENO);
            }
        }
        execvp(argv[0], argv);
        _exit(127);
    }
    int st;
    while (waitpid(p, &st, 0) < 0 && errno == EINTR) {}
    if (WIFEXITED(st)) return WEXITSTATUS(st);
    return 128 + WTERMSIG(st);
}

static int systemctl_unit(const char *verb, const char *unit, bool quiet)
{
    char *argv[] = { "systemctl", (char *)verb, (char *)unit, NULL };
    return runv(argv, quiet);
}

static void add_arg(char **argv, int *argc, const char *s)
{
    if (*argc >= MAX_ARGS - 1) die("too many arguments");
    argv[(*argc)++] = strdup(s);
    if (!argv[*argc - 1]) die("out of memory");
    argv[*argc] = NULL;
}

static void add_prop(char **argv, int *argc, const char *key, const char *value)
{
    char *s = NULL;
    if (asprintf(&s, "%s=%s", key, value) < 0) die("out of memory");
    add_arg(argv, argc, "--property");
    if (*argc >= MAX_ARGS - 1) die("too many arguments");
    argv[(*argc)++] = s;
    argv[*argc] = NULL;
}

static const char *jstr(json_object *o, const char *key)
{
    json_object *v = NULL;
    if (!json_object_object_get_ex(o, key, &v) || !v) return NULL;
    if (!json_object_is_type(v, json_type_string)) return NULL;
    return json_object_get_string(v);
}

static bool jbool(json_object *o, const char *key)
{
    json_object *v = NULL;
    if (!json_object_object_get_ex(o, key, &v) || !v) return false;
    return json_object_get_boolean(v);
}

static void add_limits(char **argv, int *argc, json_object *limits)
{
    if (!limits || !json_object_is_type(limits, json_type_object)) return;
    struct json_object_iterator it = json_object_iter_begin(limits);
    struct json_object_iterator end = json_object_iter_end(limits);
    for (; !json_object_iter_equal(&it, &end); json_object_iter_next(&it)) {
        const char *k = json_object_iter_peek_name(&it);
        json_object *v = json_object_iter_peek_value(&it);
        const char *val = json_object_get_string(v);
        const char *prop = NULL;
        if (!strcmp(k, "core")) prop = "LimitCORE";
        else if (!strcmp(k, "nofile")) prop = "LimitNOFILE";
        else if (!strcmp(k, "nproc")) prop = "LimitNPROC";
        else if (!strcmp(k, "as")) prop = "LimitAS";
        else if (!strcmp(k, "memlock")) prop = "LimitMEMLOCK";
        else if (!strcmp(k, "stack")) prop = "LimitSTACK";
        if (!prop) continue;

        char tmp[128];
        const char *sp = strchr(val, ' ');
        if (sp) {
            size_t a = (size_t)(sp - val);
            while (*sp == ' ') sp++;
            snprintf(tmp, sizeof(tmp), "%.*s:%s", (int)a, val, sp);
            add_prop(argv, argc, prop, tmp);
        } else add_prop(argv, argc, prop, val);
    }
}

static void warn_unsupported(json_object *inst)
{
    const char *keys[] = { "seccomp", "capabilities", "pidfile", "jail", "netdev", "file", "watch", "watchdog", "data", NULL };
    for (int i = 0; keys[i]; i++) {
        json_object *v = NULL;
        if (json_object_object_get_ex(inst, keys[i], &v))
            fprintf(stderr, "neon-procd: warning: parameter '%s' is not translated yet\n", keys[i]);
    }
}

static int start_instance(const char *service, const char *iname, json_object *inst)
{
    json_object *cmd = NULL;
    if (!json_object_object_get_ex(inst, "command", &cmd) || !json_object_is_type(cmd, json_type_array) || json_object_array_length(cmd) == 0) {
        fprintf(stderr, "neon-procd: %s/%s has no command; skipping\n", service, iname);
        return 0;
    }

    char unit[320];
    unit_name(service, iname, unit, sizeof(unit));
    (void)systemctl_unit("stop", unit, true);
    (void)systemctl_unit("reset-failed", unit, true);

    char *argv[MAX_ARGS] = {0};
    int argc = 0;
    add_arg(argv, &argc, "systemd-run");
    add_arg(argv, &argc, "--unit"); add_arg(argv, &argc, unit);
    add_arg(argv, &argc, "--collect");
    add_arg(argv, &argc, "--service-type=simple");
    add_prop(argv, &argc, "Description", service);

    json_object *respawn = NULL;
    if (json_object_object_get_ex(inst, "respawn", &respawn)) {
        add_prop(argv, &argc, "Restart", "always");
        const char *restart_sec = "5";
        if (json_object_is_type(respawn, json_type_array) && json_object_array_length(respawn) >= 2)
            restart_sec = json_object_get_string(json_object_array_get_idx(respawn, 1));
        add_prop(argv, &argc, "RestartSec", restart_sec);

        if (json_object_is_type(respawn, json_type_array) && json_object_array_length(respawn) >= 3) {
            const char *threshold = json_object_get_string(json_object_array_get_idx(respawn, 0));
            const char *burst = json_object_get_string(json_object_array_get_idx(respawn, 2));
            add_prop(argv, &argc, "StartLimitIntervalSec", threshold);
            add_prop(argv, &argc, "StartLimitBurst", burst);
        }
    } else {
        add_prop(argv, &argc, "Restart", "no");
    }

    const char *user = jstr(inst, "user");
    const char *group = jstr(inst, "group");
    if (user && *user) add_prop(argv, &argc, "User", user);
    if (group && *group) add_prop(argv, &argc, "Group", group);

    json_object *nice = NULL;
    if (json_object_object_get_ex(inst, "nice", &nice))
        add_prop(argv, &argc, "Nice", json_object_get_string(nice));
    if (jbool(inst, "no_new_privs")) add_prop(argv, &argc, "NoNewPrivileges", "yes");

    add_prop(argv, &argc, "StandardOutput", "journal");
    add_prop(argv, &argc, "StandardError", "journal");

    json_object *limits = NULL;
    if (json_object_object_get_ex(inst, "limits", &limits)) add_limits(argv, &argc, limits);

    json_object *env = NULL;
    if (json_object_object_get_ex(inst, "env", &env) && json_object_is_type(env, json_type_object)) {
        struct json_object_iterator it = json_object_iter_begin(env);
        struct json_object_iterator end = json_object_iter_end(env);
        for (; !json_object_iter_equal(&it, &end); json_object_iter_next(&it)) {
            const char *k = json_object_iter_peek_name(&it);
            const char *v = json_object_get_string(json_object_iter_peek_value(&it));
            char *kv = NULL;
            if (asprintf(&kv, "%s=%s", k, v) < 0) die("out of memory");
            add_arg(argv, &argc, "--setenv");
            if (argc >= MAX_ARGS - 1) die("too many arguments");
            argv[argc++] = kv; argv[argc] = NULL;
        }
    }

    warn_unsupported(inst);
    add_arg(argv, &argc, "--");
    int n = (int)json_object_array_length(cmd);
    for (int i = 0; i < n; i++) add_arg(argv, &argc, json_object_get_string(json_object_array_get_idx(cmd, i)));

    int rc = runv(argv, false);
    for (int i = 0; i < argc; i++) free(argv[i]);
    return rc;
}

static void ensure_registry(void)
{
    (void)mkdir("/run/neon-procd", 0755);
    (void)mkdir(REGISTRY_DIR, 0755);
}

static void registry_path(const char *service, char *out, size_t n)
{
    char s[128]; sanitize(service, s, sizeof(s));
    snprintf(out, n, REGISTRY_DIR "/%s.json", s);
}

static int save_registry(const char *service, json_object *root)
{
    ensure_registry();
    char path[384], tmp[400]; registry_path(service, path, sizeof(path));
    snprintf(tmp, sizeof(tmp), "%s.tmp", path);
    FILE *f = fopen(tmp, "w"); if (!f) return -1;
    fputs(json_object_to_json_string_ext(root, JSON_C_TO_STRING_PRETTY), f); fputc('\n', f);
    if (fclose(f)) return -1;
    return rename(tmp, path);
}

static json_object *read_json_file(const char *path)
{
    FILE *f=fopen(path,"r"); if(!f) return NULL;
    fseek(f,0,SEEK_END); long z=ftell(f); rewind(f); if(z<0){fclose(f);return NULL;}
    char *buf=malloc((size_t)z+1); if(!buf){fclose(f);return NULL;}
    size_t n=fread(buf,1,(size_t)z,f); fclose(f); buf[n]=0;
    json_object *o=json_tokener_parse(buf); free(buf); return o;
}

static int cmd_set(void)
{
    char *text = read_stdin_all();
    json_object *root = json_tokener_parse(text);
    free(text);
    if (!root) die("invalid JSON on stdin");
    const char *service = jstr(root, "name");
    if (!service) die("missing service name");
    if (save_registry(service, root)) fprintf(stderr, "neon-procd: warning: cannot save registry for %s\n", service);

    json_object *instances = NULL;
    if (!json_object_object_get_ex(root, "instances", &instances) || !json_object_is_type(instances, json_type_object)) {
        json_object_put(root);
        return 0;
    }

    int rc = 0;
    struct json_object_iterator it = json_object_iter_begin(instances);
    struct json_object_iterator end = json_object_iter_end(instances);
    for (; !json_object_iter_equal(&it, &end); json_object_iter_next(&it)) {
        const char *iname = json_object_iter_peek_name(&it);
        json_object *inst = json_object_iter_peek_value(&it);
        int r = start_instance(service, iname, inst);
        if (r && !rc) rc = r;
    }
    json_object_put(root);
    return rc;
}

static int cmd_delete(const char *service, const char *instance)
{
    if (instance && *instance) {
        char unit[320];
        unit_name(service, instance, unit, sizeof(unit));
        return systemctl_unit("stop", unit, true);
    }
    char s[128], pattern[256];
    sanitize(service, s, sizeof(s));
    snprintf(pattern, sizeof(pattern), UNIT_PREFIX "%s-*.service", s);
    char *argv[] = { "systemctl", "stop", pattern, NULL };
    return runv(argv, true);
}

static int cmd_running(const char *service, const char *instance)
{
    if (instance && *instance) {
        char unit[320];
        unit_name(service, instance, unit, sizeof(unit));
        char *argv[] = { "systemctl", "is-active", "--quiet", unit, NULL };
        return runv(argv, true);
    }
    char s[128], pattern[256];
    sanitize(service, s, sizeof(s));
    snprintf(pattern, sizeof(pattern), UNIT_PREFIX "%s-*.service", s);
    char *argv[] = { "systemctl", "is-active", "--quiet", pattern, NULL };
    return runv(argv, true);
}

static int cmd_status(const char *service, const char *instance)
{
    if (instance && *instance) {
        char unit[320];
        unit_name(service, instance, unit, sizeof(unit));
        char *argv[] = { "systemctl", "status", "--no-pager", unit, NULL };
        return runv(argv, false);
    }
    char s[128], pattern[256];
    sanitize(service, s, sizeof(s));
    snprintf(pattern, sizeof(pattern), UNIT_PREFIX "%s-*.service", s);
    char *argv[] = { "systemctl", "status", "--no-pager", pattern, NULL };
    return runv(argv, false);
}

static int active_unit(const char *service, const char *instance)
{
    char unit[320]; unit_name(service, instance, unit, sizeof(unit));
    char *argv[] = { "systemctl", "is-active", "--quiet", unit, NULL };
    return runv(argv, true) == 0;
}

static int cmd_list_json(void)
{
    char *text=read_stdin_all(); json_object *filter=json_tokener_parse(text); free(text);
    const char *wanted = filter ? jstr(filter,"name") : NULL;
    json_object *out=json_object_new_object(); ensure_registry();
    DIR *d=opendir(REGISTRY_DIR); if(!d){puts("{}"); if(filter)json_object_put(filter); json_object_put(out); return 0;}
    struct dirent *de;
    while((de=readdir(d))) {
        size_t l=strlen(de->d_name); if(l<6 || strcmp(de->d_name+l-5,".json")) continue;
        char path[512]; snprintf(path,sizeof(path),REGISTRY_DIR "/%s",de->d_name);
        json_object *root=read_json_file(path); if(!root) continue;
        const char *svc=jstr(root,"name"); if(!svc || (wanted && *wanted && strcmp(wanted,svc))){json_object_put(root);continue;}
        json_object *so=json_object_new_object(), *io=json_object_new_object(), *instances=NULL;
        json_object_object_add(so,"instances",io);
        if(json_object_object_get_ex(root,"instances",&instances) && json_object_is_type(instances,json_type_object)) {
            struct json_object_iterator it=json_object_iter_begin(instances), end=json_object_iter_end(instances);
            for(;!json_object_iter_equal(&it,&end);json_object_iter_next(&it)) {
                const char *in=json_object_iter_peek_name(&it); json_object *st=json_object_new_object();
                json_object_object_add(st,"running",json_object_new_boolean(active_unit(svc,in)));
                json_object_object_add(io,in,st);
            }
        }
        json_object_object_add(out,svc,so); json_object_put(root);
    }
    closedir(d); puts(json_object_to_json_string(out)); json_object_put(out); if(filter)json_object_put(filter); return 0;
}

static int cmd_delete_json(void)
{
    char *text=read_stdin_all(); json_object *o=json_tokener_parse(text); free(text); if(!o)return 1;
    const char *svc=jstr(o,"name"), *inst=jstr(o,"instance"); if(!svc){json_object_put(o);return 1;}
    int rc=cmd_delete(svc,inst); if(!inst||!*inst){char p[384];registry_path(svc,p,sizeof(p));unlink(p);} json_object_put(o); return rc;
}

static int cmd_signal_json(void)
{
    char *text=read_stdin_all(); json_object *o=json_tokener_parse(text); free(text); if(!o)return 1;
    const char *svc=jstr(o,"name"), *inst=jstr(o,"instance"); json_object *sig=NULL; int signo=15;
    if(json_object_object_get_ex(o,"signal",&sig)) signo=json_object_get_int(sig);
    if(!svc){json_object_put(o);return 1;} char unit[320];
    if(inst&&*inst) unit_name(svc,inst,unit,sizeof(unit)); else {char ss[128];sanitize(svc,ss,sizeof(ss));snprintf(unit,sizeof(unit),UNIT_PREFIX "%s-*.service",ss);}
    char sigarg[32]; snprintf(sigarg,sizeof(sigarg),"--signal=%d",signo);
    char *argv[]={"systemctl","kill",sigarg,unit,NULL}; int rc=runv(argv,true); json_object_put(o); return rc;
}

static void execute_argv(json_object *a)
{
    if (!a || !json_object_is_type(a, json_type_array)) return;
    int n = (int)json_object_array_length(a);
    if (n < 1 || n >= MAX_ARGS) return;
    char *argv[MAX_ARGS] = {0};
    for (int i = 0; i < n; i++)
        argv[i] = (char *)json_object_get_string(json_object_array_get_idx(a, i));
    argv[n] = NULL;
    (void)runv(argv, false);
}

static void inspect_trigger(json_object *tr, const char *etype, const char *package, const char *iface)
{
    if(!json_object_is_type(tr,json_type_array) || json_object_array_length(tr)<2)return;
    const char *pat=json_object_get_string(json_object_array_get_idx(tr,0)); if(!pat || fnmatch(pat,etype,0))return;
    json_object *rules=json_object_array_get_idx(tr,1); if(!json_object_is_type(rules,json_type_array))return;
    int nr=(int)json_object_array_length(rules);
    for(int i=0;i<nr;i++) {
        json_object *r=json_object_array_get_idx(rules,i); if(!json_object_is_type(r,json_type_array)||json_object_array_length(r)<1)continue;
        const char *op=json_object_get_string(json_object_array_get_idx(r,0));
        if(op && !strcmp(op,"if") && json_object_array_length(r)>=3) {
            json_object *cond=json_object_array_get_idx(r,1), *act=json_object_array_get_idx(r,2); bool ok=true;
            if(json_object_is_type(cond,json_type_array)&&json_object_array_length(cond)>=3) {
                const char *cmp=json_object_get_string(json_object_array_get_idx(cond,0)); const char *key=json_object_get_string(json_object_array_get_idx(cond,1)); const char *val=json_object_get_string(json_object_array_get_idx(cond,2));
                if(cmp && !strcmp(cmp,"eq")) { if(key&&!strcmp(key,"package")) ok=package&&val&&!strcmp(package,val); else if(key&&!strcmp(key,"interface")) ok=iface&&val&&!strcmp(iface,val); }
            }
            if(ok && json_object_is_type(act,json_type_array) && json_object_array_length(act)>=2 && !strcmp(json_object_get_string(json_object_array_get_idx(act,0)),"run_script")) execute_argv(json_object_array_get_idx(act,1));
        } else if(op && !strcmp(op,"run_script") && json_object_array_length(r)>=2) execute_argv(json_object_array_get_idx(r,1));
    }
}

static int cmd_event(void)
{
    char *text=read_stdin_all(); json_object *ev=json_tokener_parse(text); free(text); if(!ev)return 1;
    const char *etype=jstr(ev,"type"), *package=NULL, *iface=NULL; json_object *data=NULL;
    if(json_object_object_get_ex(ev,"data",&data)&&json_object_is_type(data,json_type_object)){package=jstr(data,"package");iface=jstr(data,"interface");}
    if(!etype){json_object_put(ev);return 1;} ensure_registry(); DIR *d=opendir(REGISTRY_DIR); if(!d){json_object_put(ev);return 0;} struct dirent *de;
    while((de=readdir(d))){size_t l=strlen(de->d_name);if(l<6||strcmp(de->d_name+l-5,".json"))continue;char p[512];snprintf(p,sizeof(p),REGISTRY_DIR "/%s",de->d_name);json_object *root=read_json_file(p),*trs=NULL;if(root&&json_object_object_get_ex(root,"triggers",&trs)&&json_object_is_type(trs,json_type_array)){int n=(int)json_object_array_length(trs);for(int i=0;i<n;i++)inspect_trigger(json_object_array_get_idx(trs,i),etype,package,iface);}if(root)json_object_put(root);}
    closedir(d); json_object_put(ev); return 0;
}

int main(int argc, char **argv)
{
    if (argc < 2) {
        fprintf(stderr, "usage: neon-procd set | delete SERVICE [INSTANCE] | running SERVICE [INSTANCE] | status SERVICE [INSTANCE]\n");
        return 2;
    }
    if (!strcmp(argv[1], "set")) return cmd_set();
    if (!strcmp(argv[1], "list-json")) return cmd_list_json();
    if (!strcmp(argv[1], "delete-json")) return cmd_delete_json();
    if (!strcmp(argv[1], "signal-json")) return cmd_signal_json();
    if (!strcmp(argv[1], "event")) return cmd_event();
    if (!strcmp(argv[1], "delete") && argc >= 3) return cmd_delete(argv[2], argc >= 4 ? argv[3] : NULL);
    if (!strcmp(argv[1], "running") && argc >= 3) return cmd_running(argv[2], argc >= 4 ? argv[3] : NULL);
    if (!strcmp(argv[1], "status") && argc >= 3) return cmd_status(argv[2], argc >= 4 ? argv[3] : NULL);
    fprintf(stderr, "neon-procd: invalid command\n");
    return 2;
}
