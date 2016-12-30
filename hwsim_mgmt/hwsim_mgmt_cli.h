#ifndef MAC80211_HWSIM_MGMT_HWSIM_MGMT_H
#define MAC80211_HWSIM_MGMT_HWSIM_MGMT_H

#include <stdbool.h>
#include <argp.h>
#include "hwsim_mgmt_func.h"

enum op_mode {
    HWSIM_OP_NONE,
    HWSIM_OP_CREATE,
    HWSIM_OP_DELETE
};

typedef struct {
    enum op_mode mode;
    char *hwname;
    uint32_t c_channels;
    bool c_no_vif;
    bool c_use_chanctx;
    char *c_reg_alpha2;
    uint32_t c_reg_custom_reg;
    uint32_t del_radio_id;
    bool del_ref_found;
} hwsim_args;

typedef struct {
    struct argp hwsim_argp;
    hwsim_args args;
    netlink_ctx nl_ctx;
} hwsim_cli_ctx;

int handleCreate(const hwsim_args *args);

int handleDelete(const hwsim_args *args);

#endif //MAC80211_HWSIM_MGMT_HWSIM_MGMT_H
