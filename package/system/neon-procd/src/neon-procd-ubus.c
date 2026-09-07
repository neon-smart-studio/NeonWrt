#define _GNU_SOURCE
#include <errno.h>
#include <fcntl.h>
#include <libubox/blobmsg_json.h>
#include <libubox/uloop.h>
#include <libubus.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

static struct ubus_context *ctx;
static struct blob_buf b;

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
    int in[2], op[2];
    if (pipe(in) || pipe(op)) return -1;
    pid_t p = fork();
    if (p < 0) return -1;
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

int main(void)
{
    uloop_init();
    ctx = ubus_connect(NULL);
    if (!ctx) { fprintf(stderr, "neon-procd-ubus: cannot connect to ubus\n"); return 1; }
    ubus_add_uloop(ctx);
    if (ubus_add_object(ctx, &service_object)) {
        fprintf(stderr, "neon-procd-ubus: cannot register ubus service object (is procd still running?)\n");
        ubus_free(ctx); uloop_done(); return 1;
    }
    uloop_run();
    ubus_free(ctx); uloop_done();
    return 0;
}
