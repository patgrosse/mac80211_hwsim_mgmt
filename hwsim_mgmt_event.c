#include <netlink/netlink.h>
#include <netlink/genl/genl.h>
#include <event.h>
#include <pthread.h>

#include "hwsim_mgmt_event.h"

static void notify_device_creation(int id) {
    printf("Created device with ID %d\n", id);
    exit(EXIT_SUCCESS);
}

static void notify_device_deletion() {
    printf("Successfully deleted device\n");
    exit(EXIT_SUCCESS);
}

static int nl_err_cb(struct sockaddr_nl *nla, struct nlmsgerr *nlerr, void *rctx) {
    UNUSED(nla);
    hwsim_cli_ctx *ctx = rctx;
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
    struct genlmsghdr *gnlh = nlmsg_data(&nlerr->msg);
    fprintf(stderr, "ERR ID WAS %d\n", nlerr->error);
    fprintf(stderr, "nl: cmd %d, seq %d: %s\n", gnlh->cmd,
            nlerr->msg.nlmsg_seq, strerror(abs(nlerr->error)));
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
    if (nl_cb_err(ctx->nl_ctx.cb, NL_CB_CUSTOM, nl_err_cb, ctx)) {
        fprintf(stderr, "Error on callback registration\n");
        return -1;
    }
    pthread_t libe_thread;
    return pthread_create(&libe_thread, NULL, run_nl_event_dispatcher, ctx);
}

int wait_for_event(hwsim_cli_ctx *ctx) {
    struct timespec halfsecond = {0, 500000000};
    nanosleep(&halfsecond, NULL);
    if (ctx->args.mode == HWSIM_OP_CREATE) {
        notify_device_creation(0);
    } else if (ctx->args.mode == HWSIM_OP_DELETE) {
        notify_device_deletion();
    }
    fprintf(stderr, "Unknown error occurred\n");
    return EXIT_FAILURE;
}