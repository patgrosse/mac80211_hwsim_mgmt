#include <netlink/netlink.h>
#include <argp.h>
#include "hwsim_mgmt_cli.h"
#include "hwsim_mgmt_func.h"

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

void argp_err_and_usage(const char *err_msg) {
    fprintf(stderr, "%s\n\n", err_msg);
    argp_help(&hwsim_argp, stdout, ARGP_HELP_STD_USAGE, program_executable);
    exit(EXIT_SUCCESS);
}

error_t hwsim_parse_argp(int key, char *arg, struct argp_state *state) {
    struct arguments *arguments = state->input;
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
            argp_help(&hwsim_argp, stdout, ARGP_HELP_STD_HELP, program_executable);
            exit(EXIT_SUCCESS);
        case ARGP_KEY_ARG:
            return 0;
        default:
            return ARGP_ERR_UNKNOWN;
    }
    return 0;
}

struct argp hwsim_argp = {options, hwsim_parse_argp, 0, doc, 0, 0, 0};

int handleCreate(const struct arguments *args) {
    netlink_ctx ctx;
    if (init_netlink(&ctx)) {
        fprintf(stderr, "Error initializing netlink context!\n");
        return EXIT_FAILURE;
    }
    if (create_radio(&ctx, args->c_channels, args->c_no_vif, args->hwname, args->c_use_chanctx, args->c_reg_alpha2,
                     args->c_reg_custom_reg)) {
        return EXIT_FAILURE;
    } else {
        return EXIT_SUCCESS;
    }
}

int handleDelete(const struct arguments *args) {
    if (!args->del_ref_found) {
        fprintf(stderr, "Exactly one of -i or -n is required when using -d");
        return EXIT_FAILURE;
    }

    netlink_ctx ctx;
    if (init_netlink(&ctx)) {
        fprintf(stderr, "Error initializing netlink context!\n");
        return -1;
    }
    if (args->hwname) {
        printf("Deleting radio with name '%s'...", args->hwname);
        if (delete_radio_by_name(&ctx, args->hwname)) {
            return EXIT_FAILURE;
        }
    } else {
        printf("Deleting radio with id '%d'...", args->del_radio_id);
        if (delete_radio_by_id(&ctx, args->del_radio_id)) {
            return EXIT_FAILURE;
        }
    }
    return EXIT_SUCCESS;
}

int main(int argc, char **argv) {
    struct arguments args = {
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

    argp_parse(&hwsim_argp, argc, argv, 0, 0, &args);
    switch (args.mode) {
        case HWSIM_OP_CREATE:
            return handleCreate(&args);
        case HWSIM_OP_DELETE:
            return handleDelete(&args);
        case HWSIM_OP_NONE:
            argp_err_and_usage("Exactly one of -d or -c is required");
            break;
    }
    exit(EXIT_FAILURE);
}