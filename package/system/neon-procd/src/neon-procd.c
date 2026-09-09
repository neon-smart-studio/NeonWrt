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

struct limit_map {
    const char *procd_name;
    const char *systemd_name;
};

static const struct limit_map limit_maps[] = {
    { "cpu",        "LimitCPU" },
    { "fsize",      "LimitFSIZE" },
    { "data",       "LimitDATA" },
    { "stack",      "LimitSTACK" },
    { "core",       "LimitCORE" },
    { "rss",        "LimitRSS" },
    { "nofile",     "LimitNOFILE" },
    { "as",         "LimitAS" },
    { "nproc",      "LimitNPROC" },
    { "memlock",    "LimitMEMLOCK" },
    { "locks",      "LimitLOCKS" },
    { "sigpending", "LimitSIGPENDING" },
    { "msgqueue",   "LimitMSGQUEUE" },
    { "nice",       "LimitNICE" },
    { "rtprio",     "LimitRTPRIO" },
    { "rttime",     "LimitRTTIME" },
    { NULL, NULL }
};

static const char *limit_property(const char *name)
{
    if (!name) return NULL;

    for (size_t i = 0; limit_maps[i].procd_name; i++) {
        if (!strcmp(name, limit_maps[i].procd_name))
            return limit_maps[i].systemd_name;
    }

    return NULL;
}

static void normalize_limit_token(const char *in, char *out, size_t outsz)
{
    if (!in || !*in) {
        snprintf(out, outsz, "infinity");
        return;
    }

    if (!strcasecmp(in, "unlimited") || !strcasecmp(in, "infinity")) {
        snprintf(out, outsz, "infinity");
        return;
    }

    snprintf(out, outsz, "%s", in);
}

static void normalize_limit_value(const char *value, char *out, size_t outsz)
{
    char buf[256];
    char soft[128];
    char hard[128];

    if (!value) value = "";

    snprintf(buf, sizeof(buf), "%s", value);

    char *p = buf;
    while (*p && isspace((unsigned char)*p))
        p++;

    char *end = p + strlen(p);
    while (end > p && isspace((unsigned char)end[-1]))
        *--end = '\0';

    /*
     * procd accepts either a single value:
     *
     *     "unlimited"
     *     "1024"
     *
     * or a soft/hard pair:
     *
     *     "1024 4096"
     *     "unlimited unlimited"
     *
     * systemd wants the pair as "soft:hard" and spells unlimited as
     * "infinity".
     */
    char *sep = strpbrk(p, " \t:");
    if (!sep) {
        normalize_limit_token(p, out, outsz);
        return;
    }

    *sep++ = '\0';
    while (*sep && (isspace((unsigned char)*sep) || *sep == ':'))
        sep++;

    char *second_end = sep + strlen(sep);
    while (second_end > sep && isspace((unsigned char)second_end[-1]))
        *--second_end = '\0';

    normalize_limit_token(p, soft, sizeof(soft));
    normalize_limit_token(*sep ? sep : p, hard, sizeof(hard));
    snprintf(out, outsz, "%s:%s", soft, hard);
}

static void add_limits(char **argv, int *argc, json_object *limits)
{
    if (!limits || !json_object_is_type(limits, json_type_object))
        return;

    struct json_object_iterator it = json_object_iter_begin(limits);
    struct json_object_iterator end = json_object_iter_end(limits);

    for (; !json_object_iter_equal(&it, &end); json_object_iter_next(&it)) {
        const char *name = json_object_iter_peek_name(&it);
        json_object *value_obj = json_object_iter_peek_value(&it);
        const char *prop = limit_property(name);

        if (!prop) {
            fprintf(stderr,
                    "neon-procd: warning: unsupported resource limit '%s'\n",
                    name ? name : "(null)");
            continue;
        }

        const char *value = json_object_get_string(value_obj);
        char normalized[256];
        normalize_limit_value(value, normalized, sizeof(normalized));

        add_prop(argv, argc, prop, normalized);
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

    /*
     * OpenWrt init scripts assume the traditional system PATH.  A transient
     * service must not depend on whatever PATH happened to be inherited by
     * neon-procd/systemd-run, so provide a deterministic default.  An
     * explicit procd env PATH overrides this default.
     */
    bool have_path = false;
    json_object *env = NULL;
    if (json_object_object_get_ex(inst, "env", &env) && json_object_is_type(env, json_type_object)) {
        struct json_object_iterator it = json_object_iter_begin(env);
        struct json_object_iterator end = json_object_iter_end(env);
        for (; !json_object_iter_equal(&it, &end); json_object_iter_next(&it)) {
            const char *k = json_object_iter_peek_name(&it);
            const char *v = json_object_get_string(json_object_iter_peek_value(&it));

            if (k && !strcmp(k, "PATH"))
                have_path = true;

            char *kv = NULL;
            if (asprintf(&kv, "%s=%s", k, v) < 0) die("out of memory");
            add_arg(argv, &argc, "--setenv");
            if (argc >= MAX_ARGS - 1) die("too many arguments");
            argv[argc++] = kv;
            argv[argc] = NULL;
        }
    }

    if (!have_path) {
        add_arg(argv, &argc, "--setenv");
        add_arg(argv, &argc, "PATH=/usr/sbin:/usr/bin:/sbin:/bin");
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

static void execute_argv_slice(json_object *a, int start)
{
    if (!a || !json_object_is_type(a, json_type_array))
        return;

    int n = (int)json_object_array_length(a);
    if (start < 0 || start >= n || n - start >= MAX_ARGS)
        return;

    char *argv[MAX_ARGS] = {0};
    int argc = 0;

    for (int i = start; i < n; i++) {
        json_object *v = json_object_array_get_idx(a, i);
        if (!v)
            return;
        argv[argc++] = (char *)json_object_get_string(v);
    }
    argv[argc] = NULL;

    if (argc > 0)
        (void)runv(argv, false);
}

/*
 * procd trigger expressions generated by procd.sh have this shape:
 *
 *   [ "config.change",
 *     [ "if",
 *       [ "eq", "package", "firewall" ],
 *       [ "run_script", "/etc/init.d/firewall", "reload" ] ] ]
 *
 * and interface triggers are identical except that the condition key is
 * "interface" and the event pattern is usually "interface.*".
 *
 * Raw triggers may wrap one or more action expressions in another array:
 *
 *   [ "event.name", [ [ "run_script", ... ] ], timeout ]
 *
 * Keep the evaluator generic enough for all of those forms instead of
 * assuming that tr[1] is always an array *of* rules.
 */
static const char *event_data_value(json_object *data, const char *key)
{
    json_object *v = NULL;

    if (!data || !key || !json_object_is_type(data, json_type_object))
        return NULL;
    if (!json_object_object_get_ex(data, key, &v) || !v)
        return NULL;

    return json_object_get_string(v);
}

static bool eval_condition(json_object *cond, json_object *data)
{
    if (!cond || !json_object_is_type(cond, json_type_array) ||
        json_object_array_length(cond) < 1)
        return false;

    const char *op = json_object_get_string(json_object_array_get_idx(cond, 0));
    if (!op)
        return false;

    if ((!strcmp(op, "eq") || !strcmp(op, "ne")) &&
        json_object_array_length(cond) >= 3) {
        const char *key = json_object_get_string(json_object_array_get_idx(cond, 1));
        const char *want = json_object_get_string(json_object_array_get_idx(cond, 2));
        const char *got = event_data_value(data, key);
        bool equal = got && want && !strcmp(got, want);
        return !strcmp(op, "eq") ? equal : !equal;
    }

    if (!strcmp(op, "and")) {
        int n = (int)json_object_array_length(cond);
        for (int i = 1; i < n; i++)
            if (!eval_condition(json_object_array_get_idx(cond, i), data))
                return false;
        return true;
    }

    if (!strcmp(op, "or")) {
        int n = (int)json_object_array_length(cond);
        for (int i = 1; i < n; i++)
            if (eval_condition(json_object_array_get_idx(cond, i), data))
                return true;
        return false;
    }

    if (!strcmp(op, "not") && json_object_array_length(cond) >= 2)
        return !eval_condition(json_object_array_get_idx(cond, 1), data);

    return false;
}

static void execute_trigger_expr(json_object *expr, json_object *data)
{
    if (!expr || !json_object_is_type(expr, json_type_array) ||
        json_object_array_length(expr) < 1)
        return;

    json_object *first = json_object_array_get_idx(expr, 0);

    /* A raw trigger can contain a list of action expressions. */
    if (first && json_object_is_type(first, json_type_array)) {
        int n = (int)json_object_array_length(expr);
        for (int i = 0; i < n; i++)
            execute_trigger_expr(json_object_array_get_idx(expr, i), data);
        return;
    }

    const char *op = first ? json_object_get_string(first) : NULL;
    if (!op)
        return;

    if (!strcmp(op, "run_script")) {
        /* argv is stored directly after the opcode, not in a nested array. */
        execute_argv_slice(expr, 1);
        return;
    }

    if (!strcmp(op, "if") && json_object_array_length(expr) >= 3) {
        json_object *cond = json_object_array_get_idx(expr, 1);
        json_object *action = json_object_array_get_idx(expr, 2);
        if (eval_condition(cond, data))
            execute_trigger_expr(action, data);
        return;
    }
}

static void inspect_trigger(json_object *tr, const char *etype, json_object *data)
{
    if (!tr || !etype || !json_object_is_type(tr, json_type_array) ||
        json_object_array_length(tr) < 2)
        return;

    const char *pattern = json_object_get_string(json_object_array_get_idx(tr, 0));
    if (!pattern || fnmatch(pattern, etype, 0) != 0)
        return;

    execute_trigger_expr(json_object_array_get_idx(tr, 1), data);
}

static int cmd_event(void)
{
    char *text = read_stdin_all();
    json_object *ev = json_tokener_parse(text);
    free(text);
    if (!ev)
        return 1;

    const char *etype = jstr(ev, "type");
    json_object *data = NULL;
    if (!etype) {
        json_object_put(ev);
        return 1;
    }

    if (!json_object_object_get_ex(ev, "data", &data) ||
        !json_object_is_type(data, json_type_object))
        data = NULL;

    ensure_registry();
    DIR *d = opendir(REGISTRY_DIR);
    if (!d) {
        json_object_put(ev);
        return 0;
    }

    struct dirent *de;
    while ((de = readdir(d))) {
        size_t l = strlen(de->d_name);
        if (l < 6 || strcmp(de->d_name + l - 5, ".json"))
            continue;

        char path[512];
        snprintf(path, sizeof(path), REGISTRY_DIR "/%s", de->d_name);

        json_object *root = read_json_file(path);
        json_object *triggers = NULL;
        if (root && json_object_object_get_ex(root, "triggers", &triggers) &&
            json_object_is_type(triggers, json_type_array)) {
            int n = (int)json_object_array_length(triggers);
            for (int i = 0; i < n; i++)
                inspect_trigger(json_object_array_get_idx(triggers, i), etype, data);
        }

        if (root)
            json_object_put(root);
    }

    closedir(d);
    json_object_put(ev);
    return 0;
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
