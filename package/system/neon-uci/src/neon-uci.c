#define _GNU_SOURCE
#include <uci.h>

#include <ctype.h>
#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <poll.h>
#include <signal.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/inotify.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>

#ifndef PATH_MAX
#define PATH_MAX 4096
#endif

#define NEON_UCI_VERSION "0.1.1"
#define DEFAULT_CONFIG_DIR "/etc/config"
#define DEFAULT_TRANSLATOR_DIR "/etc/neon-uci.d"
#define DEFAULT_SYSTEMCTL "/usr/bin/systemctl"
#define EVENT_BUF_SIZE (64 * 1024)
#define MAX_LINE 2048
#define MAX_TRANSLATORS 128
#define MAX_CSV 4096
#define DEBOUNCE_MS 250

struct translator {
    char manifest_path[PATH_MAX];
    char name[128];
    char packages[1024];
    char command[PATH_MAX];
    char units[1024];
    char action[64];
    bool enabled;
    bool unit_optional;
};

static volatile sig_atomic_t g_stop = 0;
static volatile sig_atomic_t g_reload = 0;
static const char *g_config_dir = DEFAULT_CONFIG_DIR;
static const char *g_translator_dir = DEFAULT_TRANSLATOR_DIR;
static const char *g_systemctl = DEFAULT_SYSTEMCTL;
static bool g_verbose = false;

static void log_msg(const char *level, const char *fmt, ...)
{
    va_list ap;
    fprintf(stderr, "neon-uci[%s]: ", level);
    va_start(ap, fmt);
    vfprintf(stderr, fmt, ap);
    va_end(ap);
    fputc('\n', stderr);
}

static void on_signal(int sig)
{
    if (sig == SIGHUP)
        g_reload = 1;
    else
        g_stop = 1;
}

static char *trim(char *s)
{
    char *end;
    while (isspace((unsigned char)*s))
        s++;
    if (*s == '\0')
        return s;
    end = s + strlen(s) - 1;
    while (end > s && isspace((unsigned char)*end))
        *end-- = '\0';
    return s;
}

static bool parse_bool(const char *s, bool def)
{
    if (!s || !*s)
        return def;
    if (!strcasecmp(s, "1") || !strcasecmp(s, "yes") ||
        !strcasecmp(s, "true") || !strcasecmp(s, "on"))
        return true;
    if (!strcasecmp(s, "0") || !strcasecmp(s, "no") ||
        !strcasecmp(s, "false") || !strcasecmp(s, "off"))
        return false;
    return def;
}

static bool safe_token(const char *s)
{
    if (!s || !*s)
        return false;
    for (; *s; s++) {
        unsigned char c = (unsigned char)*s;
        if (!(isalnum(c) || c == '_' || c == '-' || c == '.' || c == '@'))
            return false;
    }
    return true;
}

static bool package_name_valid(const char *s)
{
    if (!s || !*s)
        return false;
    for (; *s; s++) {
        unsigned char c = (unsigned char)*s;
        if (!(isalnum(c) || c == '_' || c == '-'))
            return false;
    }
    return true;
}

static bool action_valid(const char *s)
{
    static const char *const actions[] = {
        "none", "reload", "restart", "try-reload-or-restart",
        "reload-or-restart", "daemon-reload", NULL
    };
    for (size_t i = 0; actions[i]; i++)
        if (!strcmp(s, actions[i]))
            return true;
    return false;
}

static int parse_manifest(const char *path, struct translator *t)
{
    FILE *f = fopen(path, "r");
    char line[MAX_LINE];
    bool in_translator = false;

    if (!f) {
        log_msg("ERR", "cannot open manifest %s: %s", path, strerror(errno));
        return -1;
    }

    memset(t, 0, sizeof(*t));
    snprintf(t->manifest_path, sizeof(t->manifest_path), "%s", path);
    snprintf(t->action, sizeof(t->action), "none");
    t->enabled = true;
    t->unit_optional = true;

    while (fgets(line, sizeof(line), f)) {
        char *p = trim(line);
        if (!*p || *p == '#' || *p == ';')
            continue;
        if (*p == '[') {
            in_translator = !strcmp(p, "[translator]");
            continue;
        }
        if (!in_translator)
            continue;

        char *eq = strchr(p, '=');
        if (!eq)
            continue;
        *eq++ = '\0';
        char *key = trim(p);
        char *val = trim(eq);

        if (!strcmp(key, "name"))
            snprintf(t->name, sizeof(t->name), "%s", val);
        else if (!strcmp(key, "packages"))
            snprintf(t->packages, sizeof(t->packages), "%s", val);
        else if (!strcmp(key, "command"))
            snprintf(t->command, sizeof(t->command), "%s", val);
        else if (!strcmp(key, "units"))
            snprintf(t->units, sizeof(t->units), "%s", val);
        else if (!strcmp(key, "action"))
            snprintf(t->action, sizeof(t->action), "%s", val);
        else if (!strcmp(key, "enabled"))
            t->enabled = parse_bool(val, true);
        else if (!strcmp(key, "unit_optional"))
            t->unit_optional = parse_bool(val, true);
    }
    fclose(f);

    if (!t->name[0] || !safe_token(t->name)) {
        log_msg("ERR", "%s: invalid or missing name", path);
        return -1;
    }
    if (!t->packages[0]) {
        log_msg("ERR", "%s: missing packages", path);
        return -1;
    }
    if (!t->command[0] || t->command[0] != '/') {
        log_msg("ERR", "%s: command must be an absolute executable path", path);
        return -1;
    }
    if (!action_valid(t->action)) {
        log_msg("ERR", "%s: invalid action '%s'", path, t->action);
        return -1;
    }
    return 0;
}

static int translator_cmp(const void *a, const void *b)
{
    const struct translator *ta = a;
    const struct translator *tb = b;
    return strcmp(ta->manifest_path, tb->manifest_path);
}

static int load_translators(struct translator *list, size_t *count)
{
    DIR *d = opendir(g_translator_dir);
    struct dirent *de;
    size_t n = 0;

    if (!d) {
        log_msg("ERR", "cannot open translator directory %s: %s",
                g_translator_dir, strerror(errno));
        return -1;
    }

    while ((de = readdir(d)) != NULL) {
        if (de->d_name[0] == '.')
            continue;
        size_t len = strlen(de->d_name);
        if (len < 6 || strcmp(de->d_name + len - 5, ".conf"))
            continue;
        if (n >= MAX_TRANSLATORS) {
            log_msg("ERR", "too many translators; maximum is %d", MAX_TRANSLATORS);
            closedir(d);
            return -1;
        }
        char path[PATH_MAX];
        if (snprintf(path, sizeof(path), "%s/%s", g_translator_dir, de->d_name) >= (int)sizeof(path))
            continue;
        if (parse_manifest(path, &list[n]) == 0)
            n++;
        else {
            closedir(d);
            return -1;
        }
    }
    closedir(d);
    qsort(list, n, sizeof(*list), translator_cmp);
    *count = n;
    return 0;
}

static bool csv_contains(const char *csv, const char *needle)
{
    char buf[2048];
    char *save = NULL;
    if (!csv || !needle)
        return false;
    if (strlen(csv) >= sizeof(buf))
        return false;
    strcpy(buf, csv);
    for (char *tok = strtok_r(buf, ",", &save); tok; tok = strtok_r(NULL, ",", &save)) {
        tok = trim(tok);
        if (!strcmp(tok, needle))
            return true;
    }
    return false;
}

static bool translator_matches_packages(const struct translator *t, const char *changed_csv)
{
    char buf[2048];
    char *save = NULL;

    if (!changed_csv || !*changed_csv || !strcmp(changed_csv, "*"))
        return true;
    if (strlen(t->packages) >= sizeof(buf))
        return false;
    strcpy(buf, t->packages);
    for (char *tok = strtok_r(buf, ",", &save); tok; tok = strtok_r(NULL, ",", &save)) {
        tok = trim(tok);
        if (csv_contains(changed_csv, tok))
            return true;
    }
    return false;
}

static int spawn_wait(char *const argv[], char *const envp[])
{
    pid_t pid = fork();
    if (pid < 0)
        return -1;
    if (pid == 0) {
        if (envp)
            execve(argv[0], argv, envp);
        else
            execv(argv[0], argv);
        _exit(127);
    }
    int status;
    while (waitpid(pid, &status, 0) < 0) {
        if (errno == EINTR)
            continue;
        return -1;
    }
    if (WIFEXITED(status))
        return WEXITSTATUS(status);
    if (WIFSIGNALED(status))
        return 128 + WTERMSIG(status);
    return -1;
}

extern char **environ;

static int run_handler(const struct translator *t, const char *changed_csv)
{
    if (access(t->command, X_OK) != 0) {
        log_msg("ERR", "%s: handler %s is not executable: %s",
                t->name, t->command, strerror(errno));
        return -1;
    }

    char env_changed[MAX_CSV + 64];
    snprintf(env_changed, sizeof(env_changed), "NEON_UCI_CHANGED_PACKAGES=%s",
             changed_csv && *changed_csv ? changed_csv : "*");

    size_t env_count = 0;
    while (environ[env_count])
        env_count++;
    char **envp = calloc(env_count + 2, sizeof(char *));
    if (!envp)
        return -1;
    for (size_t i = 0; i < env_count; i++)
        envp[i] = environ[i];
    envp[env_count] = env_changed;
    envp[env_count + 1] = NULL;

    char *argv[] = { (char *)t->command, "apply", NULL };
    int rc = spawn_wait(argv, envp);
    free(envp);

    if (rc != 0) {
        log_msg("ERR", "%s: handler failed with status %d", t->name, rc);
        return -1;
    }
    if (g_verbose)
        log_msg("INFO", "%s: translation succeeded", t->name);
    return 0;
}

static bool unit_exists(const char *unit)
{
    char *argv[] = { (char *)g_systemctl, "show", "--property=LoadState",
                     "--value", (char *)unit, NULL };
    int pipefd[2];
    if (pipe(pipefd) < 0)
        return false;
    pid_t pid = fork();
    if (pid < 0) {
        close(pipefd[0]); close(pipefd[1]);
        return false;
    }
    if (pid == 0) {
        dup2(pipefd[1], STDOUT_FILENO);
        int devnull = open("/dev/null", O_WRONLY);
        if (devnull >= 0)
            dup2(devnull, STDERR_FILENO);
        close(pipefd[0]); close(pipefd[1]);
        execv(argv[0], argv);
        _exit(127);
    }
    close(pipefd[1]);
    char out[64] = {0};
    ssize_t n = read(pipefd[0], out, sizeof(out)-1);
    close(pipefd[0]);
    int status = 0;
    while (waitpid(pid, &status, 0) < 0 && errno == EINTR) {}
    if (n <= 0 || !WIFEXITED(status) || WEXITSTATUS(status) != 0)
        return false;
    char *p = trim(out);
    return strcmp(p, "not-found") != 0 && *p != '\0';
}

static int run_systemctl_action(const struct translator *t)
{
    if (!strcmp(t->action, "none"))
        return 0;

    if (!strcmp(t->action, "daemon-reload")) {
        char *argv[] = { (char *)g_systemctl, "daemon-reload", NULL };
        int rc = spawn_wait(argv, NULL);
        if (rc != 0)
            log_msg("ERR", "%s: systemctl daemon-reload failed (%d)", t->name, rc);
        return rc == 0 ? 0 : -1;
    }

    if (!t->units[0]) {
        log_msg("ERR", "%s: action '%s' requires units=", t->name, t->action);
        return -1;
    }

    char units[2048];
    char *save = NULL;
    if (strlen(t->units) >= sizeof(units))
        return -1;
    strcpy(units, t->units);

    int failures = 0;
    for (char *unit = strtok_r(units, ",", &save); unit; unit = strtok_r(NULL, ",", &save)) {
        unit = trim(unit);
        if (!safe_token(unit)) {
            log_msg("ERR", "%s: invalid unit name '%s'", t->name, unit);
            failures++;
            continue;
        }
        if (t->unit_optional && !unit_exists(unit)) {
            if (g_verbose)
                log_msg("INFO", "%s: optional unit %s not installed; skipping", t->name, unit);
            continue;
        }
        char *argv[] = { (char *)g_systemctl, (char *)t->action, unit, NULL };
        int rc = spawn_wait(argv, NULL);
        if (rc != 0) {
            log_msg("ERR", "%s: systemctl %s %s failed (%d)",
                    t->name, t->action, unit, rc);
            failures++;
        }
    }
    return failures ? -1 : 0;
}

static int apply_translators(const char *changed_csv)
{
    struct translator list[MAX_TRANSLATORS];
    size_t count = 0;
    int failures = 0;

    if (load_translators(list, &count) != 0)
        return -1;

    if (g_verbose)
        log_msg("INFO", "apply request for packages: %s",
                changed_csv && *changed_csv ? changed_csv : "*");

    for (size_t i = 0; i < count; i++) {
        struct translator *t = &list[i];
        if (!t->enabled)
            continue;
        if (!translator_matches_packages(t, changed_csv))
            continue;
        if (run_handler(t, changed_csv) != 0) {
            failures++;
            continue;
        }
        if (run_systemctl_action(t) != 0)
            failures++;
    }
    return failures ? -1 : 0;
}

static bool csv_add_unique(char *csv, size_t size, const char *value)
{
    if (!package_name_valid(value))
        return false;
    if (csv_contains(csv, value))
        return true;
    size_t used = strlen(csv);
    size_t need = strlen(value) + (used ? 1 : 0) + 1;
    if (used + need > size)
        return false;
    if (used)
        strcat(csv, ",");
    strcat(csv, value);
    return true;
}

static int watch_loop(void)
{
    int fd = -1, wd = -1;
    char events[EVENT_BUF_SIZE];
    char pending[MAX_CSV] = {0};

    while (!g_stop) {
        if (fd < 0) {
            fd = inotify_init1(IN_CLOEXEC | IN_NONBLOCK);
            if (fd < 0) {
                log_msg("ERR", "inotify_init1 failed: %s", strerror(errno));
                sleep(1);
                continue;
            }
            wd = inotify_add_watch(fd, g_config_dir,
                    IN_CLOSE_WRITE | IN_MOVED_TO | IN_CREATE | IN_DELETE |
                    IN_ATTRIB | IN_DELETE_SELF | IN_MOVE_SELF);
            if (wd < 0) {
                log_msg("ERR", "cannot watch %s: %s", g_config_dir, strerror(errno));
                close(fd); fd = -1;
                sleep(1);
                continue;
            }
            log_msg("INFO", "watching %s", g_config_dir);
        }

        if (g_reload) {
            g_reload = 0;
            apply_translators("*");
        }

        struct pollfd pfd = { .fd = fd, .events = POLLIN };
        int timeout = pending[0] ? DEBOUNCE_MS : 1000;
        int prc = poll(&pfd, 1, timeout);
        if (prc < 0) {
            if (errno == EINTR)
                continue;
            log_msg("ERR", "poll failed: %s", strerror(errno));
            close(fd); fd = -1;
            continue;
        }
        if (prc == 0) {
            if (pending[0]) {
                char batch[MAX_CSV];
                snprintf(batch, sizeof(batch), "%s", pending);
                pending[0] = '\0';
                apply_translators(batch);
            }
            continue;
        }
        if (!(pfd.revents & POLLIN))
            continue;

        ssize_t len = read(fd, events, sizeof(events));
        if (len < 0) {
            if (errno == EAGAIN || errno == EINTR)
                continue;
            log_msg("ERR", "inotify read failed: %s", strerror(errno));
            close(fd); fd = -1;
            continue;
        }

        for (char *p = events; p < events + len; ) {
            struct inotify_event *ev = (struct inotify_event *)p;
            if (ev->mask & (IN_DELETE_SELF | IN_MOVE_SELF | IN_IGNORED)) {
                close(fd); fd = -1; wd = -1;
                break;
            }
            if (ev->len && ev->name[0] != '.' && package_name_valid(ev->name)) {
                if (!csv_add_unique(pending, sizeof(pending), ev->name))
                    log_msg("WARN", "could not queue changed package '%s'", ev->name);
            }
            p += sizeof(struct inotify_event) + ev->len;
        }
    }

    if (fd >= 0)
        close(fd);
    return 0;
}

static int check_configuration(void)
{
    struct translator list[MAX_TRANSLATORS];
    size_t count = 0;
    int failures = 0;
    if (load_translators(list, &count) != 0)
        return 1;

    for (size_t i = 0; i < count; i++) {
        struct translator *t = &list[i];
        if (!t->enabled)
            continue;
        if (access(t->command, X_OK) != 0) {
            log_msg("ERR", "%s: command %s is not executable", t->name, t->command);
            failures++;
        }
        char pkgs[2048];
        if (strlen(t->packages) >= sizeof(pkgs)) {
            failures++;
            continue;
        }
        strcpy(pkgs, t->packages);
        char *save = NULL;
        for (char *pkg = strtok_r(pkgs, ",", &save); pkg; pkg = strtok_r(NULL, ",", &save)) {
            pkg = trim(pkg);
            if (!package_name_valid(pkg)) {
                log_msg("ERR", "%s: invalid package '%s'", t->name, pkg);
                failures++;
            }
        }
    }
    if (!failures)
        log_msg("INFO", "configuration valid: %zu translator(s)", count);
    return failures ? 1 : 0;
}

static int uci_query(const char *tuple, const char *separator)
{
    struct uci_context *ctx = uci_alloc_context();
    struct uci_ptr ptr;
    char *mutable = NULL;
    int rc = 1;

    if (!ctx)
        return 2;
    if (g_config_dir && strcmp(g_config_dir, UCI_CONFDIR))
        uci_set_confdir(ctx, g_config_dir);

    mutable = strdup(tuple);
    if (!mutable)
        goto out;
    memset(&ptr, 0, sizeof(ptr));

    if (uci_lookup_ptr(ctx, &ptr, mutable, true) != UCI_OK || !ptr.o)
        goto out;

    if (ptr.o->type == UCI_TYPE_STRING) {
        puts(ptr.o->v.string ? ptr.o->v.string : "");
        rc = 0;
    } else if (ptr.o->type == UCI_TYPE_LIST) {
        struct uci_element *e;
        bool first = true;
        const char *sep = separator ? separator : " ";
        uci_foreach_element(&ptr.o->v.list, e) {
            if (!first)
                fputs(sep, stdout);
            fputs(e->name ? e->name : "", stdout);
            first = false;
        }
        fputc('\n', stdout);
        rc = 0;
    }

out:
    free(mutable);
    uci_free_context(ctx);
    return rc;
}

static bool hostname_value_valid(const char *s)
{
    if (!s || !*s)
        return false;

    size_t len = strlen(s);
    long max = sysconf(_SC_HOST_NAME_MAX);
    if (max <= 0)
        max = 64;
    if (len > (size_t)max)
        return false;

    if (s[0] == '.' || s[0] == '-' || s[len - 1] == '-')
        return false;

    for (const unsigned char *p = (const unsigned char *)s; *p; p++) {
        if (!(isalnum(*p) || *p == '.' || *p == '-'))
            return false;
    }
    return true;
}

static int set_kernel_hostname(const char *name)
{
    if (!hostname_value_valid(name)) {
        log_msg("ERR", "invalid hostname '%s'", name ? name : "");
        return 2;
    }

    if (sethostname(name, strlen(name)) != 0) {
        log_msg("ERR", "sethostname('%s') failed: %s", name, strerror(errno));
        return 1;
    }

    if (g_verbose)
        log_msg("INFO", "kernel hostname set to %s", name);
    return 0;
}

static void usage(FILE *f)
{
    fprintf(f,
        "neon-uci %s\n"
        "Usage:\n"
        "  neon-uci [--verbose] [--config-dir DIR] [--translator-dir DIR]\n"
        "  neon-uci --once\n"
        "  neon-uci --apply PACKAGE[,PACKAGE...]\n"
        "  neon-uci --check\n"
        "  neon-uci get UCI.TUPLE\n"
        "  neon-uci list UCI.TUPLE [SEPARATOR]\n"
        "  neon-uci set-hostname NAME\n"
        "  neon-uci --version\n",
        NEON_UCI_VERSION);
}

int main(int argc, char **argv)
{
    const char *apply = NULL;
    bool once = false, check = false;

    for (int i = 1; i < argc; i++) {
        if (!strcmp(argv[i], "--verbose") || !strcmp(argv[i], "-v"))
            g_verbose = true;
        else if (!strcmp(argv[i], "--config-dir") && i + 1 < argc)
            g_config_dir = argv[++i];
        else if (!strcmp(argv[i], "--translator-dir") && i + 1 < argc)
            g_translator_dir = argv[++i];
        else if (!strcmp(argv[i], "--systemctl") && i + 1 < argc)
            g_systemctl = argv[++i];
        else if (!strcmp(argv[i], "--once"))
            once = true;
        else if (!strcmp(argv[i], "--apply") && i + 1 < argc)
            apply = argv[++i];
        else if (!strcmp(argv[i], "--check"))
            check = true;
        else if (!strcmp(argv[i], "--version")) {
            puts(NEON_UCI_VERSION);
            return 0;
        } else if (!strcmp(argv[i], "--help") || !strcmp(argv[i], "-h")) {
            usage(stdout);
            return 0;
        } else if (!strcmp(argv[i], "get") && i + 1 < argc) {
            return uci_query(argv[i + 1], NULL);
        } else if (!strcmp(argv[i], "list") && i + 1 < argc) {
            const char *sep = (i + 2 < argc) ? argv[i + 2] : " ";
            return uci_query(argv[i + 1], sep);
        } else if (!strcmp(argv[i], "set-hostname") && i + 1 < argc) {
            if (i + 2 != argc) {
                usage(stderr);
                return 2;
            }
            return set_kernel_hostname(argv[i + 1]);
        } else {
            usage(stderr);
            return 2;
        }
    }

    if (check)
        return check_configuration();
    if (once)
        return apply_translators("*") == 0 ? 0 : 1;
    if (apply) {
        char copy[MAX_CSV];
        if (strlen(apply) >= sizeof(copy)) {
            log_msg("ERR", "package list too long");
            return 2;
        }
        strcpy(copy, apply);
        char *save = NULL;
        for (char *pkg = strtok_r(copy, ",", &save); pkg; pkg = strtok_r(NULL, ",", &save)) {
            pkg = trim(pkg);
            if (!package_name_valid(pkg)) {
                log_msg("ERR", "invalid package name '%s'", pkg);
                return 2;
            }
        }
        return apply_translators(apply) == 0 ? 0 : 1;
    }

    struct sigaction sa;
    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = on_signal;
    sigemptyset(&sa.sa_mask);
    sigaction(SIGTERM, &sa, NULL);
    sigaction(SIGINT, &sa, NULL);
    sigaction(SIGHUP, &sa, NULL);

    /* Build runtime-generated files before the network/application stack settles. */
    if (apply_translators("*") != 0)
        log_msg("WARN", "one or more initial translations failed");

    return watch_loop();
}
