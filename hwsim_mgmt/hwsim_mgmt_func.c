/*
 * mac80211_hwsim_mgmt - management tool for mac80211_hwsim kernel module
 * Copyright (c) 2016, Patrick Grosse <patrick.grosse@uni-muenster.de>
 */

#include <netlink/netlink.h>
#include <netlink/genl/genl.h>
#include <netlink/genl/ctrl.h>
#include <netlink/genl/family.h>

#include "hwsim_mgmt_func.h"

int init_netlink(netlink_ctx *ctx) {
    int ret;

    ctx->cb = nl_cb_alloc(NL_CB_CUSTOM);
    if (!ctx->cb) {
        fprintf(stderr, "Error allocating netlink callbacks\n");
        return EXIT_FAILURE;
    }

    ctx->sock = nl_socket_alloc_cb(ctx->cb);
    if (!ctx->sock) {
        fprintf(stderr, "Error allocating netlink socket\n");
        return EXIT_FAILURE;
    }

    ret = genl_connect(ctx->sock);
    if (ret < 0) {
        fprintf(stderr, "Error connecting netlink socket ret=%d\n", ret);
        return EXIT_FAILURE;
    }

    ret = genl_ctrl_alloc_cache(ctx->sock, &ctx->cache);
    if (ret < 0) {
        fprintf(stderr, "Error allocationg netlink cache ret=%d\n", ret);
        return EXIT_FAILURE;
    }

    ctx->family = genl_ctrl_search_by_name(ctx->cache, "MAC80211_HWSIM");

    if (!ctx->family) {
        fprintf(stderr, "Family MAC80211_HWSIM not registered\n");
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

int create_radio(const netlink_ctx *ctx, const uint32_t channels, const bool no_vif, const char *hwname,
                 const bool use_chanctx, const char *reg_alpha2,
                 const uint32_t reg_custom_reg) {
    struct nl_msg *msg;
    msg = nlmsg_alloc();

    if (!msg) {
        fprintf(stderr, "Error allocating new message!\n");
        return EXIT_FAILURE;
    }
    int fam_id = genl_family_get_id(ctx->family);
    if (genlmsg_put(msg, NL_AUTO_PID, NL_AUTO_SEQ,
                    fam_id, 0,
                    NLM_F_REQUEST, HWSIM_CMD_NEW_RADIO,
                    1) == NULL) {
        fprintf(stderr, "Error in genlmsg_put!\n");
        nlmsg_free(msg);
        return EXIT_FAILURE;
    }
    if (channels != 0) {
        nla_put_u32(msg, HWSIM_ATTR_CHANNELS, channels);
    }
    if (no_vif) {
        nla_put_flag(msg, HWSIM_ATTR_NO_VIF);
    }
    if (hwname) {
        nla_put_string(msg, HWSIM_ATTR_RADIO_NAME, hwname);
    }
    if (use_chanctx) {
        nla_put_flag(msg, HWSIM_ATTR_USE_CHANCTX);
    }
    if (reg_alpha2 != NULL) {
        nla_put_string(msg, HWSIM_ATTR_REG_HINT_ALPHA2, reg_alpha2);
    }
    if (reg_custom_reg != 0) {
        nla_put_u32(msg, HWSIM_ATTR_REG_CUSTOM_REG, reg_custom_reg);
    }
    if (nl_send_auto(ctx->sock, msg) < 0) {
        fprintf(stderr, "Error sending message!\n");
        nlmsg_free(msg);
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}

int delete_radio_by_id(const netlink_ctx *ctx, const uint32_t radio_id) {
    struct nl_msg *msg;
    msg = nlmsg_alloc();
    if (!msg) {
        fprintf(stderr, "Error allocating new message!\n");
        return EXIT_FAILURE;
    }
    if (genlmsg_put(msg, NL_AUTO_PID, NL_AUTO_SEQ,
                    genl_family_get_id(ctx->family), 0,
                    NLM_F_REQUEST, HWSIM_CMD_DEL_RADIO,
                    1) == NULL) {
        fprintf(stderr, "Error in genlmsg_put!\n");
        nlmsg_free(msg);
        return EXIT_FAILURE;
    }
    nla_put_u32(msg, HWSIM_ATTR_RADIO_ID, radio_id);
    if (nl_send_auto(ctx->sock, msg) < 0) {
        fprintf(stderr, "Error sending message!\n");
        nlmsg_free(msg);
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}

int delete_radio_by_name(const netlink_ctx *ctx, const char *radio_name) {
    struct nl_msg *msg;
    msg = nlmsg_alloc();
    if (!msg) {
        fprintf(stderr, "Error allocating new message!\n");
        return EXIT_FAILURE;
    }
    if (genlmsg_put(msg, NL_AUTO_PID, NL_AUTO_SEQ,
                    genl_family_get_id(ctx->family), 0,
                    NLM_F_REQUEST, HWSIM_CMD_DEL_RADIO,
                    1) == NULL) {
        fprintf(stderr, "Error in genlmsg_put!\n");
        nlmsg_free(msg);
        return EXIT_FAILURE;
    }
    nla_put_string(msg, HWSIM_ATTR_RADIO_NAME, radio_name);
    if (nl_send_auto(ctx->sock, msg) < 0) {
        fprintf(stderr, "Error sending message!\n");
        nlmsg_free(msg);
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;

}

int set_rssi(const netlink_ctx *ctx, const uint32_t radio_id, const uint32_t rssi) {
    struct nl_msg *msg;

    msg = nlmsg_alloc();
    if (!msg) {
        fprintf(stderr, "Error allocating new message!\n");
        return EXIT_FAILURE;
    }
    if (genlmsg_put(msg, NL_AUTO_PID, NL_AUTO_SEQ,
                    genl_family_get_id(ctx->family), 0,
                    NLM_F_REQUEST, HWSIM_CMD_GET_RADIO,
                    1) == NULL) {
        fprintf(stderr, "Error in genlmsg_put!\n");
        nlmsg_free(msg);
        return EXIT_FAILURE;
    }
    nla_put_u32(msg, HWSIM_ATTR_SIGNAL, rssi * -1);
    nla_put_u32(msg, HWSIM_ATTR_RADIO_ID, radio_id);
    if (nl_send_auto(ctx->sock, msg) < 0) {
        fprintf(stderr, "Error sending message!\n");
        nlmsg_free(msg);
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}
