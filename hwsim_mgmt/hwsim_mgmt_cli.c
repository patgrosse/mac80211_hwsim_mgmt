/*
 * mac80211_hwsim_mgmt - management tool for mac80211_hwsim kernel module
 * Copyright (c) 2016, Patrick Grosse <patrick.grosse@uni-muenster.de>
 */

#include <netlink/netlink.h>
#include <argp.h>
#include <stdarg.h>
#include "hwsim_mgmt_cli.h"
#include "hwsim_mgmt_event.h"

const char *argp_program_version = "mac80211_hwsim_mgmt v0.1";
const char *argp_program_bug_address = "<patrick.grosse@uni-muenster.de>";
static char *program_executable = "hwsim_mgmt";
static const char doc[] = "Management tool for mac80211_hwsim kernel module";
static struct argp_option options[] = {
        {0,           0,   0,      0, "Modes: [-c [OPTION...]|-d|-x]",             1},
        {"create",    'c', 0,      0, "Create a new radio",                        1},
        {"delid",     'd', "ID",   0, "Delete an existing radio by its id",        1},
        {"delname",   'x', "NAME", 0, "Delete an existing radio by its name",      1},
        {"setrssi",   'k', "NUM",  0, "Set RSSI to specific radio",                1},
        {0,           0,   0,      0, "Create options:",                           2},
        {"name",      'n', "NAME", 0, "The requested name (may not be available)", 2},
        {"channels",  'o', "NUM",  0, "Number of concurrent channels",             2},
        {"novif",     'v', 0,      0, "No auto vif (flag)",                        2},
        {"chanctx",   't', 0,      0, "Use chantx (flag)",                         2},
        {"alphareg",  'a', "STR",  0, "reg_alpha2 hint",                           2},
        {"customreg", 'r', "REG",  0, "reg_domain ID int",                         2},
        {0,           0,   0,      0, "General:",                                  -1},
        {0,           0,   0,      0, 0,                                           0}
};
static const char *msg_duplicate_mode = "Exactly one parameter out of -c, -d, -x is required\n";

static hwsim_cli_ctx ctx;

static void argp_err_and_usage(const char *err_msg, ...) {
    va_list(args);
    va_start(args, err_msg);
    vfprintf(stderr, err_msg, args);
    argp_help(&ctx.hwsim_argp, stdout, ARGP_HELP_STD_USAGE, program_executable);
    exit(EXIT_SUCCESS);
}

uint32_t cli_get_uint32(const char opt, const char *arg) {
    char *endptr = NULL;
    unsigned long ul = strtoul(arg, &endptr, 10);
    unsigned long parsed_len = endptr - arg;
    if (strlen(arg) != parsed_len || (ul == ULONG_MAX && errno == ERANGE) || ul > UINT32_MAX) {
        argp_err_and_usage("-%c requires a positive integer attribute (max 32 bit)\n", opt);
    }
    return (uint32_t) ul;
}

error_t hwsim_parse_argp(int key, char *arg, struct argp_state *state) {
    hwsim_args *arguments = state->input;
    switch (key) {
        case 'd':
            if (arguments->mode != HWSIM_OP_NONE) {
                argp_err_and_usage(msg_duplicate_mode);
            }
            arguments->mode = HWSIM_OP_DELETE_BY_ID;
            arguments->del_radio_id = cli_get_uint32('d', arg);
            break;
        case 'x':
            if (arguments->mode != HWSIM_OP_NONE) {
                argp_err_and_usage(msg_duplicate_mode);
            }
            arguments->del_radio_name = arg;
            arguments->mode = HWSIM_OP_DELETE_BY_NAME;
            break;
        case 'k':
            if (arguments->mode != HWSIM_OP_NONE) {
                argp_err_and_usage(msg_duplicate_mode);
            }
            arguments->rssi_radio = cli_get_uint32('d', arg);
            arguments->mode = HWSIM_OP_SET_RSSI;
            break;
        case 'c':
            if (arguments->mode != HWSIM_OP_NONE) {
                argp_err_and_usage(msg_duplicate_mode);
            }
            arguments->mode = HWSIM_OP_CREATE;
            break;
        case 'n':
            arguments->c_hwname = arg;
            break;
        case 'o':
            arguments->c_channels = cli_get_uint32('o', arg);
            break;
        case 'v':
            arguments->c_no_vif = true;
            break;
        case 't':
            arguments->c_use_chanctx = true;
            break;
        case 'a':
            arguments->c_reg_alpha2 = arg;
            break;
        case 'r':
            arguments->c_reg_custom_reg = cli_get_uint32('r', arg);
            break;
        case 'h':
            argp_help(&ctx.hwsim_argp, stdout, ARGP_HELP_STD_HELP, program_executable);
            exit(EXIT_SUCCESS);
        case ARGP_KEY_ARG:
            return 0;
        default:
            return ARGP_ERR_UNKNOWN;
    }
    return 0;
}

static int prepareCommand() {
    if (init_netlink(&ctx.nl_ctx)) {
        fprintf(stderr, "Error initializing netlink context!\n");
        return -1;
    }
    if (register_event(&ctx)) {
        fprintf(stderr, "Error registering events!\n");
        return -1;
    }
    return 0;
}

int handleCreate(const hwsim_args *args) {
    int ret;
    if ((ret = prepareCommand())) {
        return ret;
    };
    if ((ret = create_radio(&ctx.nl_ctx, args->c_channels, args->c_no_vif, args->c_hwname, args->c_use_chanctx,
                            args->c_reg_alpha2,
                            args->c_reg_custom_reg))) {
        return ret;
    }
    return wait_for_event();
}

int handleDeleteById(const hwsim_args *args) {
    int ret;
    if ((ret = prepareCommand())) {
        return ret;
    };
    printf("Deleting radio with id '%d'...\n", args->del_radio_id);
    if ((ret = delete_radio_by_id(&ctx.nl_ctx, args->del_radio_id))) {
        return ret;
    }
    return wait_for_event();
}

int handleDeleteByName(const hwsim_args *args) {
    int ret;
    if ((ret = prepareCommand())) {
        return ret;
    };
    printf("Deleting radio with name '%s'...\n", args->del_radio_name);
    if ((ret = delete_radio_by_name(&ctx.nl_ctx, args->del_radio_name))) {
        return ret;
    }
    return wait_for_event();
}

int handleSetRSSI(const hwsim_args *args, char *rssi) {
    int ret;
    if ((ret = prepareCommand())) {
        return ret;
    };

    if ((ret = set_rssi(&ctx.nl_ctx, args->rssi_radio, cli_get_uint32('d', rssi)))) {
        return ret;
    }
    return wait_for_event();
}

void notify_device_creation(int id) {
    printf("Created device with ID %d\n", id);
    exit(EXIT_SUCCESS);
}

void notify_device_deletion() {
    if (ctx.args.mode == HWSIM_OP_DELETE_BY_ID) {
        printf("Successfully deleted device with ID %d\n", ctx.args.del_radio_id);
    } else {
        printf("Successfully deleted device with name '%s'\n", ctx.args.del_radio_name);
    }
    exit(EXIT_SUCCESS);
}

void notify_device_setRSSI() {
    if (ctx.args.mode == HWSIM_OP_SET_RSSI) {
        printf("new SSID defined to interface %d\n", ctx.args.rssi_radio);
    }
    exit(EXIT_SUCCESS);
}

int main(int argc, char **argv) {
    hwsim_args args = {
            .mode = HWSIM_OP_NONE,
            .c_hwname = NULL,
            .c_channels = 0,
            .c_no_vif = false,
            .c_use_chanctx = false,
            .c_reg_alpha2 = NULL,
            .c_reg_custom_reg = 0,
            .del_radio_id = 0,
            .del_radio_name = NULL,
            .rssi_radio = 0
    };

    ctx.args = args;
    struct argp hwsim_argp = {options, hwsim_parse_argp, 0, doc, 0, 0, 0};
    ctx.hwsim_argp = hwsim_argp;

    argp_parse(&hwsim_argp, argc, argv, 0, 0, &ctx.args);
    switch (ctx.args.mode) {
        case HWSIM_OP_CREATE:
            return handleCreate(&ctx.args);
        case HWSIM_OP_DELETE_BY_ID:
            return handleDeleteById(&ctx.args);
        case HWSIM_OP_DELETE_BY_NAME:
            return handleDeleteByName(&ctx.args);
        case HWSIM_OP_SET_RSSI:
            return handleSetRSSI(&ctx.args, argv[3]);
        case HWSIM_OP_NONE:
            argp_err_and_usage(msg_duplicate_mode);
            break;
    }
    return EXIT_FAILURE;
}
