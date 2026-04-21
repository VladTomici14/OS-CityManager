#ifndef CITY_MANAGER_H
#define CITY_MANAGER_H

#include <time.h>
#include <sys/types.h>

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

int validate_exact_permissions(const char *filepath, mode_t expected_mode);
void update_active_reports_symlink(const char *district_id);
void scan_district_entries(const char *district_id);
void scan_active_report_links(void);
int log_action(const char *district_id, const char *role, const char *user, const char *command);

int add_report(const char *district_id, const char *role, const char *user);
int list_reports(const char *district_id, const char *role);
int view_report(const char *district_id, const char *role, int report_id);
int remove_report_by_id(const char *district_id, const char *role, int report_id);
int update_district_threshold(const char *district_id, const char *role, int threshold);

#endif