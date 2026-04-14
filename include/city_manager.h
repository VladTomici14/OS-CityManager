#ifndef CITY_MANAGER_H
#define CITY_MANAGER_H

#include <time.h>

#define MAX_STR_LEN 64
#define DESC_LEN 256

typedef struct {
    int report_id;
    char inspector_name[MAX_STR_LEN];
    double latitude;
    double longitude;
    char category[MAX_STR_LEN];
    int severity;
    time_t timestamp;
    char description[DESC_LEN];
} Report;

// ----- filter methods -----
int parse_condition(const char *input, char *field, char *op, char *value);
int match_condition(Report *r, const char *field, const char *op, const char *value);
void execute_filter(const char *district_id, const char *role, int argc, char *argv[], int start_idx);

// ----- permissions & access control -----
#define WANT_READ  1
#define WANT_WRITE 2
#define WANT_EXEC  3

void initialize_district(const char *district_id);
int check_access(const char *filepath, const char *role, int action);
void mode_to_string(mode_t mode, char *str);

#endif