#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include "../include/city_manager.h"

int parse_condition(const char *input, char *field, char *op, char *value) {
    int items_parsed = sscanf(input, "%63[^:]:%2[^:]:%63s", field, op, value);
    if (items_parsed == 3) {
        return 1;
    }

    return 0;
}

int match_condition(Report *r, const char *field, const char *op, const char *value) {
    // ----- handling the string field -----
    if (strcmp(field, "category") == 0 || strcmp(field, "inspector") == 0) {
        const char *record_val = (strcmp(field, "category") == 0) ? r->category : r->inspector_name;

        if (strcmp(op, "==") == 0) { return strcmp(record_val, value) == 0; }
        if (strcmp(op, "!=") == 0) { return strcmp(record_val, value) != 0; }

        return 0;
    }

    // ----- handling the integer filed -----
    if (strcmp(field, "severity") == 0) {
        int target_val = atoi(value);

        if (strcmp(op, "==") == 0) { return r->severity == target_val; }
        if (strcmp(op, "!=") == 0) { return r->severity != target_val; }
        if (strcmp(op, "<") == 0) { return r->severity < target_val; }
        if (strcmp(op, "<=") == 0) { return r->severity <= target_val; }
        if (strcmp(op, ">") == 0) { return r->severity > target_val; }
        if (strcmp(op, ">=") == 0) { return r->severity >= target_val; }

        return 0;
    }

    // ----- handling timestamp field -----
    if (strcmp(field, "timestamp") == 0) {
        time_t target_val = (time_t) atol(value);

        if (strcmp(op, "==") == 0) { return r->timestamp == target_val; }
        if (strcmp(op, "!=") == 0) { return r->timestamp != target_val; }
        if (strcmp(op, "<") == 0) { return r->timestamp < target_val; }
        if (strcmp(op, "<=") == 0) { return r->timestamp <= target_val; }
        if (strcmp(op, ">") == 0) { return r->timestamp > target_val; }
        if (strcmp(op, ">=") == 0) { return r->timestamp >= target_val; }

        return 0;
    }

    return 0;
}

void execute_filter(const char *district_id, const char *role, int argc, char *argv[], int start_idx) {
    char reports_path[256];
    snprintf(reports_path, sizeof(reports_path), "%s/reports.dat", district_id);

    if (!check_access(reports_path, role, WANT_READ)) {
        return;
    }

    int fd = open(reports_path, O_RDONLY);
    if (fd < 0) {
        perror("[ERROR] open reports.dat");
        return;
    }

    Report r;
    while (read(fd, &r, sizeof(r)) == (ssize_t)sizeof(r)) {
        int all_match = 1;

        for (int i = start_idx; i < argc; i++) {
            char field[MAX_STR_LEN] = {0};
            char op[3] = {0};
            char value[MAX_STR_LEN] = {0};

            if (!parse_condition(argv[i], field, op, value)) {
                fprintf(stderr, "[ERROR] Invalid condition format: %s\n", argv[i]);
                all_match = 0;
                break;
            }

            if (!match_condition(&r, field, op, value)) {
                all_match = 0;
                break;
            }
        }

        if (all_match) {
            printf("ID: %d | inspector: %s | category: %s | severity: %d | timestamp: %ld\n",
                   r.report_id, r.inspector_name, r.category, r.severity, (long)r.timestamp);
        }
    }

    close(fd);
}
