#include <netlink/netlink.h>
#include <netlink/genl/genl.h>
#include <event.h>
#include <pthread.h>

#include "hwsim_mgmt_event.h"

static pthread_mutex_t nl_cb_mutex;

static void notify_device_creation(int id) {
    printf("Created device with ID %d\n", id);
    exit(EXIT_SUCCESS);
}

static void notify_device_deletion() {
    printf("Successfully deleted device\n");
    exit(EXIT_SUCCESS);
}

static int nl_msg_cb(struct nl_msg *msg, void *rctx) {
    hwsim_cli_ctx *ctx = rctx;
    struct nlmsghdr *nlh = nlmsg_hdr(msg);
    struct genlmsghdr *gnlh = nlmsg_data(nlh);
    int retcode = gnlh->cmd;
    if (retcode == 0) {
        if (!pthread_mutex_trylock(&nl_cb_mutex)) {
            if (ctx->args.mode == HWSIM_OP_CREATE) {
                notify_device_creation(0);
            } else if (ctx->args.mode == HWSIM_OP_DELETE) {
                notify_device_deletion();
            }
            // should not be needed
            pthread_mutex_unlock(&nl_cb_mutex);
        }
    }
    return 0;
}

static int nl_err_cb(struct sockaddr_nl *nla, struct nlmsgerr *nlerr, void *rctx) {
    UNUSED(nla);
    hwsim_cli_ctx *ctx = rctx;
    if (!pthread_mutex_trylock(&nl_cb_mutex)) {
        if (ctx->args.mode == HWSIM_OP_DELETE) {
            if (nlerr->error == -ENODEV) {
                fprintf(stderr, "Device not found\n");
            } else {
                fprintf(stderr, "Unknown error with id %d\n", nlerr->error);
            }
            exit(EXIT_FAILURE);
        } else if (ctx->args.mode == HWSIM_OP_CREATE) {
            notify_device_creation(nlerr->error);
        }
        // should not be needed
        pthread_mutex_unlock(&nl_cb_mutex);
    }
    return NL_SKIP;
}

static void nl_event_handler(int fd, short what, void *rctx) {
    UNUSED(fd);
    UNUSED(what);
    hwsim_cli_ctx *ctx = rctx;
    nl_recvmsgs_default(ctx->nl_ctx.sock);
}

static void *run_nl_event_dispatcher(void *rctx) {
    hwsim_cli_ctx *ctx = rctx;
    event_init();
    struct event ev_cmd;
    event_set(&ev_cmd, nl_socket_get_fd(ctx->nl_ctx.sock), EV_READ | EV_PERSIST,
              nl_event_handler, ctx);
    event_add(&ev_cmd, NULL);
    event_dispatch();
    return NULL;
}

int register_event(hwsim_cli_ctx *ctx) {
    if (nl_cb_set(ctx->nl_ctx.cb, NL_CB_MSG_IN, NL_CB_CUSTOM, nl_msg_cb, ctx)) {
        fprintf(stderr, "Error on callback registration\n");
        return -1;
    }
    if (nl_cb_err(ctx->nl_ctx.cb, NL_CB_CUSTOM, nl_err_cb, ctx)) {
        fprintf(stderr, "Error on callback registration (2)\n");
        return -1;
    }

    pthread_t libe_thread;
    return pthread_create(&libe_thread, NULL, run_nl_event_dispatcher, ctx);
}

int wait_for_event() {
    struct timespec halfsecond = {0, 500000000};
    nanosleep(&halfsecond, NULL);
    fprintf(stderr, "Did not receive netlink event after 500 msec\n");
    return EXIT_FAILURE;
}