/*
 * mac80211_hwsim_mgmt - management tool for mac80211_hwsim kernel module
 * Copyright (c) 2016, Patrick Grosse <patrick.grosse@uni-muenster.de>
 */

#include <netlink/netlink.h>
#include <netlink/genl/genl.h>
#include <event.h>
#include <pthread.h>
#include <unistd.h>

#include "hwsim_mgmt_event.h"

static pthread_mutex_t nl_cb_mutex;

static int nl_msg_cb(struct nl_msg *msg, void *rctx) {
    hwsim_cli_ctx *ctx = rctx;
    struct nlmsghdr *nlh = nlmsg_hdr(msg);
    struct genlmsghdr *gnlh = nlmsg_data(nlh);
    int retcode = gnlh->cmd;
    if (retcode == 0) {
        if (!pthread_mutex_trylock(&nl_cb_mutex)) {
            if (ctx->args.mode == HWSIM_OP_CREATE) {
                notify_device_creation(0);
            } else if (ctx->args.mode == HWSIM_OP_DELETE_BY_ID || ctx->args.mode == HWSIM_OP_DELETE_BY_NAME) {
                notify_device_deletion();
            } else if (ctx->args.mode == HWSIM_OP_SET_RSSI) {
                notify_device_setRSSI();
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
        if (ctx->args.mode == HWSIM_OP_DELETE_BY_ID || ctx->args.mode == HWSIM_OP_DELETE_BY_NAME) {
            if (nlerr->error == -ENODEV) {
                fprintf(stderr, "Device not found\n");
            } else {
                fprintf(stderr, "Unknown error on device deletion with errid %d\nstrerror: %s\n", nlerr->error,
                        strerror(abs(nlerr->error)));
            }
            exit(EXIT_FAILURE);
        } else if (ctx->args.mode == HWSIM_OP_CREATE) {
            if (nlerr->error < 0) {
                fprintf(stderr, "Unknown error on device creation with errid %d\nstrerror: %s\n", nlerr->error,
                        strerror(abs(nlerr->error)));
                exit(EXIT_FAILURE);
            }
            notify_device_creation(nlerr->error);
        } else if (ctx->args.mode == HWSIM_OP_SET_RSSI) {
            if (nlerr->error == -ENODEV) {
                fprintf(stderr, "Device not found\n");
            } else {
                fprintf(stderr, "Unknown error while setting RSSI with errid %d\nstrerror: %s\n", nlerr->error,
                        strerror(abs(nlerr->error)));
            }
            exit(EXIT_FAILURE);
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
    struct event_base *ev_base = event_base_new();
    struct event *ev_cmd = event_new(ev_base, nl_socket_get_fd(ctx->nl_ctx.sock), EV_READ | EV_PERSIST,
                                     nl_event_handler, ctx);
    event_add(ev_cmd, NULL);
    event_base_dispatch(ev_base);
    event_base_free(ev_base);
    event_free(ev_cmd);
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
    sleep(2);
    fprintf(stderr, "Did not receive netlink event after 2 sec\n");
    return EXIT_FAILURE;
}
