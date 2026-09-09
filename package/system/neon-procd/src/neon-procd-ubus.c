#define _GNU_SOURCE
#include <errno.h>
#include <fcntl.h>
#include <libubox/blobmsg_json.h>
#include <libubox/uloop.h>
#include <libubus.h>
#include <signal.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <sys/socket.h>
#include <sys/statvfs.h>
#include <sys/sysinfo.h>
#include <sys/un.h>
#include <sys/utsname.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>

static struct ubus_context *ctx;
static struct blob_buf b;
static struct ubus_event_handler trigger_event_handler;

/*
 * Minimal sd_notify(3)-compatible sender.
 *
 * This keeps neon-procd-ubus independent of libsystemd. When the service
 * unit uses Type=notify, systemd will not consider it ready until the ubus
 * connection is established and the OpenWrt "service" object is registered.
 */
static int systemd_notify(const char *message)
{
    const char *path = getenv("NOTIFY_SOCKET");
    if (!path || !*path || !message)
        return 0;

    if (path[0] != '/' && path[0] != '@') {
        errno = EINVAL;
        return -1;
    }

    int fd = socket(AF_UNIX, SOCK_DGRAM | SOCK_CLOEXEC, 0);
    if (fd < 0)
        return -1;

    struct sockaddr_un sa;
    memset(&sa, 0, sizeof(sa));
    sa.sun_family = AF_UNIX;

    socklen_t salen;

    if (path[0] == '@') {
        size_t n = strlen(path + 1);
        if (n + 1 > sizeof(sa.sun_path)) {
            close(fd);
            errno = ENAMETOOLONG;
            return -1;
        }

        sa.sun_path[0] = '\0';
        memcpy(sa.sun_path + 1, path + 1, n);
        salen = (socklen_t)(offsetof(struct sockaddr_un, sun_path) + 1 + n);
    } else {
        size_t n = strlen(path);
        if (n >= sizeof(sa.sun_path)) {
            close(fd);
            errno = ENAMETOOLONG;
            return -1;
        }

        memcpy(sa.sun_path, path, n + 1);
        salen = (socklen_t)(offsetof(struct sockaddr_un, sun_path) + n + 1);
    }

    ssize_t rc;
    do {
        rc = sendto(fd, message, strlen(message), MSG_NOSIGNAL,
                    (struct sockaddr *)&sa, salen);
    } while (rc < 0 && errno == EINTR);

    int saved_errno = errno;
    close(fd);
    errno = saved_errno;

    return rc < 0 ? -1 : 0;
}

static int write_all(int fd, const char *buf, size_t len)
{
    size_t off = 0;

    while (off < len) {
        ssize_t n = write(fd, buf + off, len - off);

        if (n < 0) {
            if (errno == EINTR)
                continue;
            return -1;
        }

        if (n == 0)
            return -1;

        off += (size_t)n;
    }

    return 0;
}

static int feed_neon(const char *verb, const char *json)
{
    int pfd[2];
    if (pipe(pfd)) return UBUS_STATUS_UNKNOWN_ERROR;
    pid_t p = fork();
    if (p < 0) { close(pfd[0]); close(pfd[1]); return UBUS_STATUS_UNKNOWN_ERROR; }
    if (!p) {
        dup2(pfd[0], STDIN_FILENO);
        close(pfd[0]); close(pfd[1]);
        execl("/usr/sbin/neon-procd", "neon-procd", verb, NULL);
        _exit(127);
    }
    close(pfd[0]);
    int write_rc = 0;
    if (json && write_all(pfd[1], json, strlen(json)) < 0)
        write_rc = -1;
    close(pfd[1]);
    int st = 0;
    while (waitpid(p, &st, 0) < 0 && errno == EINTR) {}
    if (write_rc < 0)
        return UBUS_STATUS_UNKNOWN_ERROR;
    return WIFEXITED(st) && WEXITSTATUS(st) == 0 ? 0 : UBUS_STATUS_UNKNOWN_ERROR;
}


/*
 * Event-trigger actions may call back into the OpenWrt "service" ubus object
 * (for example when an init script reload re-registers procd instances).
 * Running them synchronously from an ubus event callback would block this
 * daemon's uloop and can deadlock that callback path.  Double-fork the event
 * worker so the ubus loop stays responsive and no long-lived zombie remains.
 */
static int feed_neon_async(const char *verb, const char *json)
{
    int pfd[2];
    if (pipe(pfd))
        return -1;

    pid_t p = fork();
    if (p < 0) {
        close(pfd[0]);
        close(pfd[1]);
        return -1;
    }

    if (p == 0) {
        pid_t g = fork();
        if (g < 0)
            _exit(127);
        if (g > 0)
            _exit(0);

        dup2(pfd[0], STDIN_FILENO);
        close(pfd[0]);
        close(pfd[1]);
        execl("/usr/sbin/neon-procd", "neon-procd", verb, NULL);
        _exit(127);
    }

    close(pfd[0]);
    int rc = 0;
    if (json && write_all(pfd[1], json, strlen(json)) < 0)
        rc = -1;
    close(pfd[1]);

    int st = 0;
    while (waitpid(p, &st, 0) < 0 && errno == EINTR) {}
    if (!WIFEXITED(st) || WEXITSTATUS(st) != 0)
        rc = -1;

    return rc;
}

static int service_set(struct ubus_context *c, struct ubus_object *o,
                       struct ubus_request_data *req, const char *method,
                       struct blob_attr *msg)
{
    (void)c; (void)o; (void)req; (void)method;
    char *json = blobmsg_format_json(msg, true);
    if (!json) return UBUS_STATUS_UNKNOWN_ERROR;
    int rc = feed_neon("set", json);
    free(json);
    return rc;
}

static int service_delete(struct ubus_context *c, struct ubus_object *o,
                          struct ubus_request_data *req, const char *method,
                          struct blob_attr *msg)
{
    (void)c; (void)o; (void)req; (void)method;
    char *json = blobmsg_format_json(msg, true);
    if (!json) return UBUS_STATUS_UNKNOWN_ERROR;
    int rc = feed_neon("delete-json", json);
    free(json);
    return rc;
}

static int service_signal(struct ubus_context *c, struct ubus_object *o,
                          struct ubus_request_data *req, const char *method,
                          struct blob_attr *msg)
{
    (void)c; (void)o; (void)req; (void)method;
    char *json = blobmsg_format_json(msg, true);
    if (!json) return UBUS_STATUS_UNKNOWN_ERROR;
    int rc = feed_neon("signal-json", json);
    free(json);
    return rc;
}

static int capture_neon(const char *verb, const char *json, char **out)
{
    int in[2] = { -1, -1 };
    int op[2] = { -1, -1 };

    if (pipe(in))
        return -1;

    if (pipe(op)) {
        close(in[0]);
        close(in[1]);
        return -1;
    }

    pid_t p = fork();
    if (p < 0) {
        close(in[0]);
        close(in[1]);
        close(op[0]);
        close(op[1]);
        return -1;
    }
    if (!p) {
        dup2(in[0], STDIN_FILENO); dup2(op[1], STDOUT_FILENO);
        close(in[0]); close(in[1]); close(op[0]); close(op[1]);
        execl("/usr/sbin/neon-procd", "neon-procd", verb, NULL);
        _exit(127);
    }
    close(in[0]); close(op[1]);
    int write_rc = 0;
    if (json && write_all(in[1], json, strlen(json)) < 0)
        write_rc = -1;
    close(in[1]);
    size_t cap=4096,len=0; char *buf=malloc(cap);
    if (!buf) { close(op[0]); return -1; }
    for (;;) {
        if (len+2048+1>cap) { cap*=2; char *n=realloc(buf,cap); if(!n){free(buf);close(op[0]);return -1;} buf=n; }
        ssize_t n=read(op[0],buf+len,cap-len-1);
        if(n<0){if(errno==EINTR)continue;break;} if(!n)break; len+=(size_t)n;
    }
    close(op[0]); buf[len]=0;
    int st=0; while(waitpid(p,&st,0)<0 && errno==EINTR){}
    if(write_rc < 0 || !WIFEXITED(st)||WEXITSTATUS(st)){free(buf);return -1;}
    *out=buf; return 0;
}

static int service_list(struct ubus_context *c, struct ubus_object *o,
                        struct ubus_request_data *req, const char *method,
                        struct blob_attr *msg)
{
    (void)o; (void)method;
    char *filter = msg ? blobmsg_format_json(msg, true) : strdup("{}");
    char *out = NULL;
    if (!filter || capture_neon("list-json", filter, &out)) { free(filter); return UBUS_STATUS_UNKNOWN_ERROR; }
    free(filter);
    blob_buf_init(&b, 0);
    if (!blobmsg_add_json_from_string(&b, out)) { free(out); return UBUS_STATUS_UNKNOWN_ERROR; }
    free(out);
    ubus_send_reply(c, req, b.head);
    return 0;
}

static int service_event(struct ubus_context *c, struct ubus_object *o,
                         struct ubus_request_data *req, const char *method,
                         struct blob_attr *msg)
{
    (void)c; (void)o; (void)req; (void)method;
    char *json = blobmsg_format_json(msg, true);
    if (!json) return UBUS_STATUS_UNKNOWN_ERROR;
    int rc = feed_neon("event", json);
    free(json);
    return rc;
}


/* -------------------------------------------------------------------------
 * OpenWrt-compatible "system" ubus object
 *
 * LuCI and a number of OpenWrt utilities expect procd to expose at least
 *     ubus call system board
 *     ubus call system info
 *
 * NeonWrt intentionally does not run upstream procd as a second PID 1/service
 * manager.  Implement these read-only compatibility calls here instead.
 * ------------------------------------------------------------------------- */

struct release_info {
    char distribution[128];
    char version[128];
    char revision[128];
    char codename[128];
    char target[128];
    char description[256];
    char builddate[64];
    char firmware_url[256];
};

static char *trim_ws(char *s)
{
    char *end;

    if (!s)
        return s;

    while (*s == ' ' || *s == '\t' || *s == '\r' || *s == '\n')
        s++;

    end = s + strlen(s);
    while (end > s && (end[-1] == ' ' || end[-1] == '\t' ||
                       end[-1] == '\r' || end[-1] == '\n'))
        *--end = '\0';

    return s;
}

static void copy_string(char *dst, size_t dstsz, const char *src)
{
    if (!dst || dstsz == 0)
        return;

    if (!src)
        src = "";

    snprintf(dst, dstsz, "%s", src);
}

static void unquote_value(char *value)
{
    char *src;
    char *dst;
    char quote = 0;
    size_t len;

    if (!value)
        return;

    value = trim_ws(value);
    len = strlen(value);

    if (len >= 2 && (value[0] == '\'' || value[0] == '"') &&
        value[len - 1] == value[0]) {
        quote = value[0];
        value[len - 1] = '\0';
        memmove(value, value + 1, len - 1);
    }

    src = value;
    dst = value;

    while (*src) {
        if (*src == '\\' && src[1] != '\0') {
            src++;
            *dst++ = *src++;
            continue;
        }

        *dst++ = *src++;
    }

    *dst = '\0';
    (void)quote;
}

static void release_assign(struct release_info *ri, const char *key,
                           const char *value, bool only_if_empty)
{
    char *dst = NULL;
    size_t dstsz = 0;

    if (!ri || !key || !value)
        return;

    if (!strcasecmp(key, "NAME") || !strcasecmp(key, "DISTRIB_ID")) {
        dst = ri->distribution; dstsz = sizeof(ri->distribution);
    } else if (!strcasecmp(key, "VERSION") || !strcasecmp(key, "DISTRIB_RELEASE")) {
        dst = ri->version; dstsz = sizeof(ri->version);
    } else if (!strcasecmp(key, "BUILD_ID") || !strcasecmp(key, "DISTRIB_REVISION")) {
        dst = ri->revision; dstsz = sizeof(ri->revision);
    } else if (!strcasecmp(key, "VERSION_CODENAME") || !strcasecmp(key, "DISTRIB_CODENAME")) {
        dst = ri->codename; dstsz = sizeof(ri->codename);
    } else if (!strcasecmp(key, "OPENWRT_BOARD") || !strcasecmp(key, "DISTRIB_TARGET")) {
        dst = ri->target; dstsz = sizeof(ri->target);
    } else if (!strcasecmp(key, "OPENWRT_RELEASE") || !strcasecmp(key, "DISTRIB_DESCRIPTION")) {
        dst = ri->description; dstsz = sizeof(ri->description);
    } else if (!strcasecmp(key, "OPENWRT_BUILD_DATE")) {
        dst = ri->builddate; dstsz = sizeof(ri->builddate);
    } else if (!strcasecmp(key, "FIRMWARE_URL")) {
        dst = ri->firmware_url; dstsz = sizeof(ri->firmware_url);
    }

    if (!dst || !dstsz)
        return;

    if (only_if_empty && dst[0])
        return;

    copy_string(dst, dstsz, value);
}

static void parse_release_file(struct release_info *ri, const char *path,
                               bool only_if_empty)
{
    FILE *f;
    char line[512];

    f = fopen(path, "r");
    if (!f)
        return;

    while (fgets(line, sizeof(line), f)) {
        char *key;
        char *value;
        char *eq;

        key = trim_ws(line);
        if (!*key || *key == '#')
            continue;

        eq = strchr(key, '=');
        if (!eq)
            continue;

        *eq = '\0';
        value = trim_ws(eq + 1);
        key = trim_ws(key);
        unquote_value(value);
        release_assign(ri, key, value, only_if_empty);
    }

    fclose(f);
}

static void add_release_table(void)
{
    struct release_info ri;
    void *t;

    memset(&ri, 0, sizeof(ri));

    /* Match current procd first, then fill OpenWrt-specific gaps. */
    parse_release_file(&ri, "/usr/lib/os-release", false);
    if (!ri.distribution[0] && access("/etc/os-release", R_OK) == 0)
        parse_release_file(&ri, "/etc/os-release", true);
    parse_release_file(&ri, "/etc/openwrt_release", true);

    if (!ri.distribution[0])
        copy_string(ri.distribution, sizeof(ri.distribution), "NeonWrt");

    t = blobmsg_open_table(&b, "release");
    if (ri.distribution[0]) blobmsg_add_string(&b, "distribution", ri.distribution);
    if (ri.version[0]) blobmsg_add_string(&b, "version", ri.version);
    if (ri.revision[0]) blobmsg_add_string(&b, "revision", ri.revision);
    if (ri.codename[0]) blobmsg_add_string(&b, "codename", ri.codename);
    if (ri.target[0]) blobmsg_add_string(&b, "target", ri.target);
    if (ri.description[0]) blobmsg_add_string(&b, "description", ri.description);
    if (ri.builddate[0]) blobmsg_add_string(&b, "builddate", ri.builddate);
    if (ri.firmware_url[0]) blobmsg_add_string(&b, "firmware_url", ri.firmware_url);
    blobmsg_close_table(&b, t);
}

static int read_text_file(const char *path, char *buf, size_t bufsz)
{
    FILE *f;
    size_t n;
    char *s;

    if (!path || !buf || bufsz < 2)
        return -1;

    f = fopen(path, "rb");
    if (!f)
        return -1;

    n = fread(buf, 1, bufsz - 1, f);
    fclose(f);
    if (n == 0)
        return -1;

    buf[n] = '\0';
    s = trim_ws(buf);
    if (s != buf)
        memmove(buf, s, strlen(s) + 1);

    return buf[0] ? 0 : -1;
}

static const char *system_rootfs_type(void)
{
    static char fstype[32];
    char *mountstr = NULL;
    size_t len = 0;
    const char *mountpoint = "/";
    FILE *mounts;

    if (fstype[0])
        return fstype;

    mounts = fopen("/proc/self/mounts", "r");
    if (!mounts)
        return NULL;

    while (getline(&mountstr, &len, mounts) != -1) {
        char *saveptr = NULL;
        char *dev;
        char *mp;
        char *type;

        dev = strtok_r(mountstr, " ", &saveptr);
        mp = strtok_r(NULL, " ", &saveptr);
        type = strtok_r(NULL, " ", &saveptr);
        (void)dev;

        if (!mp || !type || strcmp(mp, mountpoint))
            continue;

        if (!strcmp(type, "overlay") && strcmp(mountpoint, "/rom")) {
            mountpoint = "/rom";
            rewind(mounts);
            continue;
        }

        copy_string(fstype, sizeof(fstype), type);
        break;
    }

    free(mountstr);
    fclose(mounts);

    return fstype[0] ? fstype : NULL;
}

static void add_cpu_system_string(void)
{
    FILE *f;
    char line[256];

    f = fopen("/proc/cpuinfo", "r");
    if (f) {
        while (fgets(line, sizeof(line), f)) {
            char *colon = strchr(line, ':');
            char *key;
            char *val;

            if (!colon)
                continue;

            *colon = '\0';
            key = trim_ws(line);
            val = trim_ws(colon + 1);

#ifdef __aarch64__
            if (!strcasecmp(key, "CPU revision")) {
                char desc[128];
                unsigned long rev = strtoul(val, NULL, 16);
                snprintf(desc, sizeof(desc), "ARMv8 Processor rev %lu", rev);
                blobmsg_add_string(&b, "system", desc);
                fclose(f);
                return;
            }
#else
            if (!strcasecmp(key, "system type") ||
                !strcasecmp(key, "processor") ||
                !strcasecmp(key, "cpu") ||
                !strcasecmp(key, "model name")) {
                if (*val) {
                    blobmsg_add_string(&b, "system", val);
                    fclose(f);
                    return;
                }
            }
#endif
        }
        fclose(f);
    }

#ifdef __aarch64__
    blobmsg_add_string(&b, "system", "ARMv8 Processor");
#else
    {
        struct utsname uts;
        if (uname(&uts) == 0)
            blobmsg_add_string(&b, "system", uts.machine);
    }
#endif
}

static int system_board(struct ubus_context *c, struct ubus_object *o,
                        struct ubus_request_data *req, const char *method,
                        struct blob_attr *msg)
{
    struct utsname uts;
    const char *rootfs_type;
    char text[256];

    (void)o;
    (void)method;
    (void)msg;

    blob_buf_init(&b, 0);

    if (uname(&uts) == 0) {
        blobmsg_add_string(&b, "kernel", uts.release);
        blobmsg_add_string(&b, "hostname", uts.nodename);
    }

    add_cpu_system_string();

    if (read_text_file("/tmp/sysinfo/model", text, sizeof(text)) == 0 ||
        read_text_file("/proc/device-tree/model", text, sizeof(text)) == 0) {
        blobmsg_add_string(&b, "model", text);
    }

    if (read_text_file("/tmp/sysinfo/board_name", text, sizeof(text)) == 0) {
        blobmsg_add_string(&b, "board_name", text);
    } else if (read_text_file("/proc/device-tree/compatible", text, sizeof(text)) == 0) {
        char *p;
        for (p = text; *p; p++) {
            if (*p == ',')
                *p = '-';
        }
        blobmsg_add_string(&b, "board_name", text);
    }

    rootfs_type = system_rootfs_type();
    if (rootfs_type)
        blobmsg_add_string(&b, "rootfs_type", rootfs_type);

    add_release_table();

    ubus_send_reply(c, req, b.head);
    return UBUS_STATUS_OK;
}

static uint64_t kscale_u64(uint64_t blocks, uint64_t block_size)
{
    return (blocks * block_size + 512ULL) / 1024ULL;
}

static int system_info(struct ubus_context *c, struct ubus_object *o,
                       struct ubus_request_data *req, const char *method,
                       struct blob_attr *msg)
{
    struct sysinfo info;
    struct tm local_tm;
    time_t now;
    FILE *f;
    char line[256];
    uint64_t available = 0;
    uint64_t cached = 0;
    void *t;
    struct statvfs sv;
    static const char *const fslist[][2] = {
        { "/", "root" },
        { "/tmp", "tmp" },
    };

    (void)o;
    (void)method;
    (void)msg;

    if (sysinfo(&info) != 0)
        return UBUS_STATUS_UNKNOWN_ERROR;

    f = fopen("/proc/meminfo", "r");
    if (!f)
        return UBUS_STATUS_UNKNOWN_ERROR;

    while (fgets(line, sizeof(line), f)) {
        char *colon = strchr(line, ':');
        char *key;
        char *val;

        if (!colon)
            continue;

        *colon = '\0';
        key = trim_ws(line);
        val = trim_ws(colon + 1);

        if (!strcasecmp(key, "MemAvailable"))
            available = strtoull(val, NULL, 10) * 1024ULL;
        else if (!strcasecmp(key, "Cached"))
            cached = strtoull(val, NULL, 10) * 1024ULL;
    }
    fclose(f);

    now = time(NULL);
    if (localtime_r(&now, &local_tm) == NULL)
        return UBUS_STATUS_UNKNOWN_ERROR;

    blob_buf_init(&b, 0);

    /* Keep the OpenWrt/procd API semantics exactly. */
    blobmsg_add_u32(&b, "localtime", (uint32_t)(now + local_tm.tm_gmtoff));
    blobmsg_add_u32(&b, "uptime", (uint32_t)info.uptime);

    t = blobmsg_open_array(&b, "load");
    blobmsg_add_u32(&b, NULL, (uint32_t)info.loads[0]);
    blobmsg_add_u32(&b, NULL, (uint32_t)info.loads[1]);
    blobmsg_add_u32(&b, NULL, (uint32_t)info.loads[2]);
    blobmsg_close_array(&b, t);

    t = blobmsg_open_table(&b, "memory");
    blobmsg_add_u64(&b, "total", (uint64_t)info.mem_unit * (uint64_t)info.totalram);
    blobmsg_add_u64(&b, "free", (uint64_t)info.mem_unit * (uint64_t)info.freeram);
    blobmsg_add_u64(&b, "shared", (uint64_t)info.mem_unit * (uint64_t)info.sharedram);
    blobmsg_add_u64(&b, "buffered", (uint64_t)info.mem_unit * (uint64_t)info.bufferram);
    blobmsg_add_u64(&b, "available", available);
    blobmsg_add_u64(&b, "cached", cached);
    blobmsg_close_table(&b, t);

    for (size_t i = 0; i < ARRAY_SIZE(fslist); i++) {
        if (statvfs(fslist[i][0], &sv) != 0)
            continue;

        uint64_t block_size = sv.f_frsize ? sv.f_frsize : sv.f_bsize;
        t = blobmsg_open_table(&b, fslist[i][1]);
        blobmsg_add_u64(&b, "total", kscale_u64((uint64_t)sv.f_blocks, block_size));
        blobmsg_add_u64(&b, "free", kscale_u64((uint64_t)sv.f_bfree, block_size));
        blobmsg_add_u64(&b, "used", kscale_u64((uint64_t)(sv.f_blocks - sv.f_bfree), block_size));
        blobmsg_add_u64(&b, "avail", kscale_u64((uint64_t)sv.f_bavail, block_size));
        blobmsg_close_table(&b, t);
    }

    t = blobmsg_open_table(&b, "swap");
    blobmsg_add_u64(&b, "total", (uint64_t)info.mem_unit * (uint64_t)info.totalswap);
    blobmsg_add_u64(&b, "free", (uint64_t)info.mem_unit * (uint64_t)info.freeswap);
    blobmsg_close_table(&b, t);

    ubus_send_reply(c, req, b.head);
    return UBUS_STATUS_OK;
}

static const struct ubus_method system_methods[] = {
    UBUS_METHOD_NOARG("board", system_board),
    UBUS_METHOD_NOARG("info", system_info),
};

static struct ubus_object_type system_type = UBUS_OBJECT_TYPE("system", system_methods);
static struct ubus_object system_object = {
    .name = "system",
    .type = &system_type,
    .methods = system_methods,
    .n_methods = ARRAY_SIZE(system_methods),
};

static const struct ubus_method methods[] = {
    UBUS_METHOD_NOARG("list", service_list),
    UBUS_METHOD_NOARG("set", service_set),
    UBUS_METHOD_NOARG("delete", service_delete),
    UBUS_METHOD_NOARG("signal", service_signal),
    UBUS_METHOD_NOARG("event", service_event),
};

static struct ubus_object_type service_type = UBUS_OBJECT_TYPE("service", methods);
static struct ubus_object service_object = {
    .name = "service",
    .type = &service_type,
    .methods = methods,
    .n_methods = ARRAY_SIZE(methods),
};


/*
 * Bridge real ubus broadcast events (interface.*, config.change, custom raw
 * events, ...) into neon-procd's trigger evaluator.  Registering "*" mirrors
 * `ubus listen` semantics and lets procd_add_raw_trigger() keep working too.
 */
static void trigger_event_cb(struct ubus_context *c,
                             struct ubus_event_handler *ev,
                             const char *type,
                             struct blob_attr *msg)
{
    (void)c;
    (void)ev;

    if (!type || !*type)
        return;

    struct blob_buf eb = {};
    blob_buf_init(&eb, 0);
    blobmsg_add_string(&eb, "type", type);

    void *t = blobmsg_open_table(&eb, "data");
    if (msg) {
        struct blob_attr *cur;
        int rem;
        blobmsg_for_each_attr(cur, msg, rem)
            blobmsg_add_blob(&eb, cur);
    }
    blobmsg_close_table(&eb, t);

    char *json = blobmsg_format_json(eb.head, true);
    if (json) {
        if (feed_neon_async("event", json) < 0)
            fprintf(stderr,
                    "neon-procd-ubus: failed to dispatch ubus event '%s'\n",
                    type);
        free(json);
    }

    blob_buf_free(&eb);
}

static int connect_and_register(void)
{
    /*
     * ubusd.service is Type=simple. "After=ubusd.service" only means the
     * ubusd process has been exec'd; its socket may not be ready yet.
     *
     * Retry both connect and object registration for up to 30 seconds.
     */
    const int attempts = 300;
    const useconds_t delay_us = 100 * 1000;

    for (int i = 0; i < attempts; i++) {
        ctx = ubus_connect(NULL);
        if (!ctx) {
            usleep(delay_us);
            continue;
        }

        ubus_add_uloop(ctx);

        int rc = ubus_add_object(ctx, &service_object);
        if (rc == 0) {
            rc = ubus_add_object(ctx, &system_object);
            if (rc == 0) {
                memset(&trigger_event_handler, 0, sizeof(trigger_event_handler));
                trigger_event_handler.cb = trigger_event_cb;
                rc = ubus_register_event_handler(ctx, &trigger_event_handler, "*");
                if (rc == 0)
                    return 0;

                fprintf(stderr,
                        "neon-procd-ubus: ubus trigger event registration "
                        "failed (%d), retrying\n", rc);
            } else {
                fprintf(stderr,
                        "neon-procd-ubus: ubus connected but system object "
                        "registration failed (%d), retrying\n",
                        rc);
            }
        } else {
            fprintf(stderr,
                    "neon-procd-ubus: ubus connected but service object "
                    "registration failed (%d), retrying\n",
                    rc);
        }

        ubus_free(ctx);
        ctx = NULL;
        usleep(delay_us);
    }

    return -1;
}

int main(void)
{
    /*
     * A child neon-procd can exit before the parent has finished writing the
     * JSON request. Ignore SIGPIPE so write_all() reports EPIPE instead of
     * terminating this long-running ubus daemon.
     */
    signal(SIGPIPE, SIG_IGN);

    if (uloop_init() != 0) {
        fprintf(stderr, "neon-procd-ubus: uloop_init failed\n");
        return 1;
    }

    if (connect_and_register() != 0) {
        fprintf(stderr,
                "neon-procd-ubus: cannot connect/register ubus service/system "
                "objects after 30 seconds\n");
        uloop_done();
        return 1;
    }

    if (systemd_notify(
            "READY=1\n"
            "STATUS=Connected to ubus; service/system objects and trigger bridge ready\n") < 0) {
        fprintf(stderr,
                "neon-procd-ubus: failed to notify systemd readiness: %s\n",
                strerror(errno));
        ubus_free(ctx);
        ctx = NULL;
        uloop_done();
        return 1;
    }

    uloop_run();

    (void)systemd_notify("STOPPING=1\n");

    if (ctx) {
        ubus_free(ctx);
        ctx = NULL;
    }

    blob_buf_free(&b);
    uloop_done();
    return 0;
}
