#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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
            report_id = atoi(argv[++i]);
        } else if (strcmp(argv[i], "--filter") == 0 && i + 2 < argc) {
            command = "filter";
            district_id = argv[++i];
            filter_start_index = i + 1;
            break;
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

    // ----- system calls -----
    // TODO: create the district folder and files if they don't exist
    // initialize_district(district_id);

    // TODO: write to logged_district
    // log_action(district_id, role, user, command);

    // ----- command routing -----
    if (strcmp(command, "add") == 0) {
        printf("[COMMAND]: Adding...\n");
        // add_report();
    } else if (strcmp(command, "list") == 0) {
        printf("[COMMAND]: Listing reports...\n");
        // list_repots();
    } else if (strcmp(command, "view") == 0) {
        printf("[COMMAND]: Viewing report...\n");
        // view_report();
    } else if (strcmp(command, "remove_report") == 0) {
        printf("[COMMAND]: Removing report...\n");
        // remove_report();
    } else if (strcmp(command, "update_threshold") == 0) {
        printf("[COMMAND]: Updating threshold...\n");
        // update_threshold();
    } else if (strcmp(command, "filter") == 0) {
        printf("[COMMAND]: Filtering...\n");
        // filter();
    } else {
        perror("[ERROR]: Unknown command sequence!");
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
