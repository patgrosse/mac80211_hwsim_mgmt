/*
 * mac80211_hwsim_mgmt - management tool for mac80211_hwsim kernel module
 * Copyright (c) 2016, Patrick Grosse <patrick.grosse@uni-muenster.de>
 */

#include <netlink/netlink.h>
#include <argp.h>
#include "hwsim_mgmt_cli.h"
#include "hwsim_mgmt_event.h"

const char *argp_program_version = "mac80211_hwsim_mgmt v0.1";
const char *argp_program_bug_address = "<patrick.grosse@uni-muenster.de>";
char *program_executable = "hwsim_mgmt";
const char doc[] = "Management tool for mac80211_hwsim kernel module";
struct argp_option options[] = {
        {0,         0,   0,      0, "Modes: [-c|-d|-x]",                         1},
        {"create",  'c', 0,      0, "Create a new radio",                        1},
        {"delid",   'd', "ID",   0, "Delete an existing radio by its id",        1},
        {"delname", 'x', "NAME", 0, "Delete an existing radio by its name",      1},
        {0,         0,   0,      0, "Create options: [-n]",                      2},
        {"name",    'n', "NAME", 0, "The requested name (may not be available)", 2},
        {0,         0,   0,      0, "General:",                                  -1},
        {0,         0,   0,      0, 0,                                           0}
};

static hwsim_cli_ctx ctx;

static void argp_err_and_usage(const char *err_msg) {
    fprintf(stderr, "%s\n\n", err_msg);
    argp_help(&ctx.hwsim_argp, stdout, ARGP_HELP_STD_USAGE, program_executable);
    exit(EXIT_SUCCESS);
}

error_t hwsim_parse_argp(int key, char *arg, struct argp_state *state) {
    hwsim_args *arguments = state->input;
    switch (key) {
        case 'd':
            if (arguments->mode != HWSIM_OP_NONE) {
                argp_err_and_usage("Only one parameter out of -c, -d, -x is allowed");
            }
            arguments->mode = HWSIM_OP_DELETE_BY_ID;
            char *endptr = NULL;
            unsigned long ul = strtoul(arg, &endptr, 10);
            unsigned long parsed_len = endptr - arg;
            if (strlen(arg) != parsed_len || (ul == ULONG_MAX && errno == ERANGE) || ul > UINT32_MAX) {
                argp_err_and_usage("-x requires a positive integer attribute (max 32 bit)");
            }
            arguments->del_radio_id = (uint32_t) ul;
            break;
        case 'x':
            if (arguments->mode != HWSIM_OP_NONE) {
                argp_err_and_usage("Only one parameter out of -c, -d, -x is allowed");
            }
            arguments->del_radio_name = arg;
            arguments->mode = HWSIM_OP_DELETE_BY_NAME;
            break;
        case 'n':
            arguments->c_hwname = arg;
            break;
        case 'c':
            if (arguments->mode != HWSIM_OP_NONE) {
                argp_err_and_usage("Only one parameter out of -c, -d, -x is allowed");
            }
            arguments->mode = HWSIM_OP_CREATE;
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
            .del_radio_name = NULL
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
        case HWSIM_OP_NONE:
            argp_err_and_usage("Exactly one of -d or -c is required");
            break;
    }
    return EXIT_FAILURE;
}
