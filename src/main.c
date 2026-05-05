#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../include/city_manager.h"

// ----- helper method to print usage -----
void printUsage(const char *program_name) {
    printf("Usage: %s --role <inspector|manager> --user <name> --<command> <district_id> [args...]\n\n", program_name);
    printf("Commands:\n");
    printf("  --add <district_id>\n");
    printf("  --list <district_id>\n");
    printf("  --view <district_id> <report_id>\n");
    printf("  --remove_report <district_id> <report_id>\n");
    printf("  --update_threshold <district_id> <value>\n");
    printf("  --filter <district_id> <condition> [condition2...]\n");
    printf("  --remove_district <district_id>\n");
}

int main(int argc, char *argv[]) {
    // ----- state variables parsed from the command line v
    char *role = NULL;
    char *user = NULL;
    char *command = NULL;
    char *district_id = NULL;

    // ----- optional variables depending on the specific command -----
    int report_id = -1;
    int threshold = -1;
    int filter_start_index = -1;

    // ----- parsing the command line arguments -----
    if (argc < 6) {
        printUsage(argv[0]);
        return EXIT_FAILURE;
    }

    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--role") == 0 && i + 1 < argc) {
            role = argv[++i];
        } else if (strcmp(argv[i], "--user") == 0 && i + 1 < argc) {
            user = argv[++i];
        } else if (strcmp(argv[i], "--add") == 0 && i + 1 < argc) {
            command = "add";
            district_id = argv[++i];
        } else if (strcmp(argv[i], "--list") == 0 && i + 1 < argc) {
            command = "list";
            district_id = argv[++i];
        } else if (strcmp(argv[i], "--view") == 0 && i + 2 < argc) {
            command = "view";
            district_id = argv[++i];
            report_id = atoi(argv[++i]);
        } else if (strcmp(argv[i], "--remove_report") == 0 && i + 2 < argc) {
            command = "remove_report";
            district_id = argv[++i];
            report_id = atoi(argv[++i]);
        } else if (strcmp(argv[i], "--update_threshold") == 0 && i + 2 < argc) {
            command = "update_threshold";
            district_id = argv[++i];
            threshold = atoi(argv[++i]);
        } else if (strcmp(argv[i], "--filter") == 0 && i + 2 < argc) {
            command = "filter";
            district_id = argv[++i];
            filter_start_index = i + 1;
            break;
        } else if (strcmp(argv[i], "--remove_district") == 0 && i + 1 < argc) {
            command = "remove_district";
            district_id = argv[++i];
        }
    }

    // ----- validating inputs -----
    if (!role || !user || !command || !district_id) {
        perror("[ERROR]: Missing required arguments.\n");
        printUsage(argv[0]);
        return EXIT_FAILURE;
    }

    if (strcmp(role, "inspector") != 0 && strcmp(role, "manager") != 0) {
        perror("[ERROR]: Invalid role. Must be 'inspector' or 'manager'.\n");
        return EXIT_FAILURE;
    }

    if (strcmp(command, "remove_district") != 0) {
        initialize_district(district_id);
        update_active_reports_symlink(district_id);
    }

    // ----- command routing -----
    if (strcmp(command, "add") == 0) {
        if (!add_report(district_id, role, user)) {
            return EXIT_FAILURE;
        }
    } else if (strcmp(command, "list") == 0) {
        if (!list_reports(district_id, role)) {
            return EXIT_FAILURE;
        }
    } else if (strcmp(command, "view") == 0) {
        if (!view_report(district_id, role, report_id)) {
            return EXIT_FAILURE;
        }
    } else if (strcmp(command, "remove_report") == 0) {
        if (!remove_report_by_id(district_id, role, report_id)) {
            return EXIT_FAILURE;
        }
    } else if (strcmp(command, "update_threshold") == 0) {
        if (!update_district_threshold(district_id, role, threshold)) {
            return EXIT_FAILURE;
        }
    } else if (strcmp(command, "filter") == 0) {
        execute_filter(district_id, role, argc, argv, filter_start_index);
    } else if (strcmp(command, "remove_district") == 0) {
        if (!remove_district(district_id, role)) {
            return EXIT_FAILURE;
        }
    } else {
        perror("[ERROR]: Unknown command sequence!");
        return EXIT_FAILURE;
    }

    if (strcmp(command, "remove_district") != 0) {
        if (!log_action(district_id, role, user, command)) {
            return EXIT_FAILURE;
        }
    }

    return EXIT_SUCCESS;
}
