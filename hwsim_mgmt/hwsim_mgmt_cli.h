/*
 * mac80211_hwsim_mgmt - management tool for mac80211_hwsim kernel module
 * Copyright (c) 2016, Patrick Grosse <patrick.grosse@uni-muenster.de>
 */

#ifndef MAC80211_HWSIM_MGMT_HWSIM_MGMT_H
#define MAC80211_HWSIM_MGMT_HWSIM_MGMT_H

#include <stdbool.h>
#include <argp.h>
#include "hwsim_mgmt_func.h"

enum op_mode {
    HWSIM_OP_NONE,
    HWSIM_OP_CREATE,
    HWSIM_OP_DELETE_BY_ID,
    HWSIM_OP_DELETE_BY_NAME,
    HWSIM_OP_SET_RSSI
};

typedef struct {
    enum op_mode mode;
    char *c_hwname;
    uint32_t c_channels;
    bool c_no_vif;
    bool c_use_chanctx;
    char *c_reg_alpha2;
    uint32_t c_reg_custom_reg;
    uint32_t del_radio_id;
    char *del_radio_name;
    uint32_t rssi_radio;
} hwsim_args;

typedef struct {
    struct argp hwsim_argp;
    hwsim_args args;
    netlink_ctx nl_ctx;
} hwsim_cli_ctx;

int handleCreate(const hwsim_args *args);

int handleDeleteById(const hwsim_args *args);

int handleDeleteByName(const hwsim_args *args);

int handleSetRSSI(const hwsim_args *args, char *rssi);

void notify_device_creation(int id);

void notify_device_deletion();

void notify_device_setRSSI();

#endif //MAC80211_HWSIM_MGMT_HWSIM_MGMT_H
