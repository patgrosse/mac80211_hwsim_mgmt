#ifndef MAC80211_HWSIM_MGMT_HWSIM_MGMT_FUNC_H
#define MAC80211_HWSIM_MGMT_HWSIM_MGMT_FUNC_H

#include <stdbool.h>

#define HWSIM_CMD_CREATE_RADIO 4
#define HWSIM_CMD_DESTROY_RADIO 5

enum {
    HWSIM_ATTR_UNSPEC,
    HWSIM_ATTR_ADDR_RECEIVER,
    HWSIM_ATTR_ADDR_TRANSMITTER,
    HWSIM_ATTR_FRAME,
    HWSIM_ATTR_FLAGS,
    HWSIM_ATTR_RX_RATE,
    HWSIM_ATTR_SIGNAL,
    HWSIM_ATTR_TX_INFO,
    HWSIM_ATTR_COOKIE,
    HWSIM_ATTR_CHANNELS,
    HWSIM_ATTR_RADIO_ID,
    HWSIM_ATTR_REG_HINT_ALPHA2,
    HWSIM_ATTR_REG_CUSTOM_REG,
    HWSIM_ATTR_REG_STRICT_REG,
    HWSIM_ATTR_SUPPORT_P2P_DEVICE,
    HWSIM_ATTR_USE_CHANCTX,
    HWSIM_ATTR_DESTROY_RADIO_ON_CLOSE,
    HWSIM_ATTR_RADIO_NAME,
    HWSIM_ATTR_NO_VIF,
    HWSIM_ATTR_FREQ,
    HWSIM_ATTR_PAD,
    __HWSIM_ATTR_MAX,
};

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

#endif //MAC80211_HWSIM_MGMT_HWSIM_MGMT_FUNC_H
