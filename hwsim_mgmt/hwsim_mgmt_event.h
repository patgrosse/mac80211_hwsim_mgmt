/*
 * mac80211_hwsim_mgmt - management tool for mac80211_hwsim kernel module
 * Copyright (c) 2016, Patrick Grosse <patrick.grosse@uni-muenster.de>
 */

#ifndef MAC80211_HWSIM_MGMT_HWSIM_MGMT_EVENT_H
#define MAC80211_HWSIM_MGMT_HWSIM_MGMT_EVENT_H

#include "hwsim_mgmt_cli.h"

#define UNUSED(x) (void)(x)

int register_event(hwsim_cli_ctx *ctx);

int wait_for_event();

#endif //MAC80211_HWSIM_MGMT_HWSIM_MGMT_EVENT_H
