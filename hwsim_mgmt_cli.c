#include <netlink/netlink.h>
#include <argp.h>
#include "hwsim_mgmt_cli.h"
#include "hwsim_mgmt_event.h"

const char *argp_program_version = "mac80211_hwsim_mgmt v0.1";
const char *argp_program_bug_address = "<patrick.grosse@uni-muenster.de>";
char *program_executable = "hwsim_mgmt";
const char doc[] = "Management tool for mac80211_hwsim kernel module";
struct argp_option options[] = {
        {0,        0,   0,      0, "Modes: [-c|-d]",                  1},
        {"create", 'c', 0,      0, "Create a new radio",              1},
        {"delete", 'd', 0,      0, "Delete an existing radio",        1},
        {0,        0,   0,      0, "Create options:",                 2},
        {0,        0,   0,      0, "Delete options: [-i|-n]",         3},
        {"name",   'n', "NAME", 0, "The name of the radio to delete", 3},
        {"id",     'i', "ID",   0, "The id of the radio to delete",   3},
        {0,        0,   0,      0, "General:",                        -1},
        {0,        0,   0,      0, 0,                                 0}
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
                argp_err_and_usage("Only one parameter out of -d and -c is allowed");
            }
            arguments->mode = HWSIM_OP_DELETE;
            break;
        case 'n':
            if (arguments->del_ref_found) {
                argp_err_and_usage("Only one parameter out of -n and -i is allowed");
            }
            arguments->hwname = arg;
            arguments->del_ref_found = true;
            break;
        case 'i':
            if (arguments->del_ref_found) {
                argp_err_and_usage("Only one parameter out of -n and -i is allowed");
            }
            char *endptr = NULL;
            unsigned long ul = strtoul(arg, &endptr, 10);
            unsigned long parsed_len = endptr - arg;
            if (strlen(arg) != parsed_len || (ul == ULONG_MAX && errno == ERANGE) || ul > UINT32_MAX) {
                argp_err_and_usage("-i requires a positive integer attribute (max 32 bit)");
            }
            arguments->del_radio_id = (uint32_t) ul;
            arguments->del_ref_found = true;
            break;
        case 'c':
            if (arguments->mode != HWSIM_OP_NONE) {
                argp_err_and_usage("Only one parameter out of -d and -c is allowed");
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
    if ((ret = create_radio(&ctx.nl_ctx, args->c_channels, args->c_no_vif, args->hwname, args->c_use_chanctx,
                            args->c_reg_alpha2,
                            args->c_reg_custom_reg))) {
        return ret;
    }
    return wait_for_event(&ctx);
}

int handleDelete(const hwsim_args *args) {
    if (!args->del_ref_found) {
        argp_err_and_usage("Exactly one of -d or -c is required");
    }
    int ret;
    if ((ret = prepareCommand())) {
        return ret;
    };
    if (args->hwname) {
        printf("Deleting radio with name '%s'...\n", args->hwname);
        if ((ret = delete_radio_by_name(&ctx.nl_ctx, args->hwname))) {
            return ret;
        }
    } else {
        printf("Deleting radio with id '%d'...\n", args->del_radio_id);
        if ((ret = delete_radio_by_id(&ctx.nl_ctx, args->del_radio_id))) {
            return ret;
        }
    }
    return wait_for_event(&ctx);
}

int main(int argc, char **argv) {
    hwsim_args args = {
            .mode = HWSIM_OP_NONE,
            .hwname = NULL,
            .c_channels = 0,
            .c_no_vif = false,
            .c_use_chanctx = false,
            .c_reg_alpha2 = NULL,
            .c_reg_custom_reg = 0,
            .del_radio_id = 0,
            .del_ref_found = false
    };
    ctx.args = args;
    struct argp hwsim_argp = {options, hwsim_parse_argp, 0, doc, 0, 0, 0};
    ctx.hwsim_argp = hwsim_argp;

    argp_parse(&hwsim_argp, argc, argv, 0, 0, &ctx.args);
    switch (ctx.args.mode) {
        case HWSIM_OP_CREATE:
            return handleCreate(&ctx.args);
        case HWSIM_OP_DELETE:
            return handleDelete(&ctx.args);
        case HWSIM_OP_NONE:
            argp_err_and_usage("Exactly one of -d or -c is required");
            break;
    }
    return EXIT_FAILURE;
}