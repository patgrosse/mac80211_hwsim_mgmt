#ifndef MAC80211_HWSIM_MGMT_HWSIM_MGMT_H
#define MAC80211_HWSIM_MGMT_HWSIM_MGMT_H

#include <stdbool.h>

enum op_mode {
    HWSIM_OP_NONE,
    HWSIM_OP_CREATE,
    HWSIM_OP_DELETE
};

struct arguments {
    enum op_mode mode;
    char *hwname;
    uint32_t c_channels;
    bool c_no_vif;
    bool c_use_chanctx;
    char *c_reg_alpha2;
    uint32_t c_reg_custom_reg;
    uint32_t del_radio_id;
    bool del_ref_found;
};

struct argp hwsim_argp;

int handleCreate(const struct arguments *args);

int handleDelete(const struct arguments *args);

#endif //MAC80211_HWSIM_MGMT_HWSIM_MGMT_H
