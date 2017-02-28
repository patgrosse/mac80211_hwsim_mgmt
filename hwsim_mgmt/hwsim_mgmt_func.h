/*
 * mac80211_hwsim_mgmt - management tool for mac80211_hwsim kernel module
 * Copyright (c) 2016, Patrick Grosse <patrick.grosse@uni-muenster.de>
 */

#ifndef MAC80211_HWSIM_MGMT_HWSIM_MGMT_FUNC_H
#define MAC80211_HWSIM_MGMT_HWSIM_MGMT_FUNC_H

#include <stdbool.h>

#define HWSIM_CMD_UNSPEC 0
#define HWSIM_CMD_REGISTER 1
#define HWSIM_CMD_FRAME 2
#define HWSIM_CMD_TX_INFO_FRAME 3
#define HWSIM_CMD_NEW_RADIO 4
#define HWSIM_CMD_DEL_RADIO 5
#define HWSIM_CMD_GET_RADIO 6
#define __HWSIM_CMD_MAX 7

#define HWSIM_ATTR_UNSPEC 0
#define HWSIM_ATTR_ADDR_RECEIVER 1
#define HWSIM_ATTR_ADDR_TRANSMITTER 2
#define HWSIM_ATTR_FRAME 3
#define HWSIM_ATTR_FLAGS 4
#define HWSIM_ATTR_RX_RATE 5
#define HWSIM_ATTR_SIGNAL 6
#define HWSIM_ATTR_TX_INFO 7
#define HWSIM_ATTR_COOKIE 8
#define HWSIM_ATTR_CHANNELS 9
#define HWSIM_ATTR_RADIO_ID 10
#define HWSIM_ATTR_REG_HINT_ALPHA2 11
#define HWSIM_ATTR_REG_CUSTOM_REG 12
#define HWSIM_ATTR_REG_STRICT_REG 13
#define HWSIM_ATTR_SUPPORT_P2P_DEVICE 14
#define HWSIM_ATTR_USE_CHANCTX 15
#define HWSIM_ATTR_DESTROY_RADIO_ON_CLOSE 16
#define HWSIM_ATTR_RADIO_NAME 17
#define HWSIM_ATTR_NO_VIF 18
#define HWSIM_ATTR_FREQ 19
#define HWSIM_ATTR_PAD 20
#define __HWSIM_ATTR_MAX 21

typedef struct {
    struct nl_cb *cb;
    struct nl_sock *sock;
    struct nl_cache *cache;
    struct genl_family *family;
} netlink_ctx;

int init_netlink(netlink_ctx *ctx);

int create_radio(const netlink_ctx *ctx, const uint32_t channels, const bool no_vif, const char *hwname,
                 const bool use_chanctx, const char *reg_alpha2,
                 const uint32_t reg_custom_reg);

int delete_radio_by_id(const netlink_ctx *ctx, const uint32_t radio_id);

int delete_radio_by_name(const netlink_ctx *ctx, const char *radio_name);

int set_rssi(const netlink_ctx *ctx, const uint32_t radio_id, const uint32_t rssi);

#endif //MAC80211_HWSIM_MGMT_HWSIM_MGMT_FUNC_H
