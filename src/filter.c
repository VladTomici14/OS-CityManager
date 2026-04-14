#include <stdio.h>
#include <stdlib.h>
#include <string.h>
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

}
