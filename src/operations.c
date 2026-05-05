#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <signal.h>
#include "../include/city_manager.h"

static void make_path(char *out, size_t out_sz, const char *district_id, const char *name) {
    snprintf(out, out_sz, "%s/%s", district_id, name);
}

static void print_report(const Report *r) {
    printf("ID: %d\n", r->report_id);
    printf("Inspector: %s\n", r->inspector_name);
    printf("GPS: %.6f, %.6f\n", r->latitude, r->longitude);
    printf("Category: %s\n", r->category);
    printf("Severity: %d\n", r->severity);
    printf("Timestamp: %ld\n", (long) r->timestamp);
    printf("Description: %s\n", r->description);
    printf("----------------------------------------\n");
}

int add_report(const char *district_id, const char *role, const char *user) {
    char reports_path[256];
    make_path(reports_path, sizeof(reports_path), district_id, "reports.dat");
    if (!check_access(reports_path, role, WANT_WRITE)) {
        return 0;
    }

    int fd = open(reports_path, O_RDWR | O_APPEND);
    if (fd < 0) {
        perror("[ERROR] open reports.dat");
        return 0;
    }

    Report r;
    memset(&r, 0, sizeof(r));
    strncpy(r.inspector_name, user, MAX_STR_LEN - 1);
    r.timestamp = time(NULL);

    struct stat st;
    if (fstat(fd, &st) < 0) {
        perror("[ERROR] fstat reports.dat");
        close(fd);
        return 0;
    }

    r.report_id = (int) (st.st_size / (off_t) sizeof(Report)) + 1;

    printf("Latitude: ");
    if (scanf("%lf", &r.latitude) != 1) {
        close(fd);
        return 0;
    }

    printf("Longitude: ");
    if (scanf("%lf", &r.longitude) != 1) {
        close(fd);
        return 0;
    }

    printf("Category: ");
    if (scanf("%63s", r.category) != 1) {
        close(fd);
        return 0;
    }

    printf("Severity (1-3): ");
    if (scanf("%d", &r.severity) != 1) {
        close(fd);
        return 0;
    }

    printf("Description (single word or underscore): ");
    if (scanf("%255s", r.description) != 1) {
        close(fd);
        return 0;
    }

    if (write(fd, &r, sizeof(r)) != (ssize_t) sizeof(r)) {
        perror("[ERROR] write report");
        close(fd);
        return 0;
    }

    close(fd);
    chmod(reports_path, 0664);
    printf("[OK] Report %d added.\n", r.report_id);

    // Notify monitor
    int monitor_fd = open(".monitor_pid", O_RDONLY);
    int monitor_notified = 0;
    if (monitor_fd >= 0) {
        char pid_buf[32];
        ssize_t n = read(monitor_fd, pid_buf, sizeof(pid_buf) - 1);
        close(monitor_fd);
        if (n > 0) {
            pid_buf[n] = '\0';
            pid_t monitor_pid = atoi(pid_buf);
            if (monitor_pid > 0 && kill(monitor_pid, SIGUSR1) == 0) {
                monitor_notified = 1;
            }
        }
    }

    // Log the notification
    char logpath[256];
    snprintf(logpath, sizeof(logpath), "%s/logged_district", district_id);

    if (strcmp(role, "manager") == 0 && check_access(logpath, role, WANT_WRITE)) {
        int log_fd = open(logpath, O_WRONLY | O_APPEND);
        if (log_fd >= 0) {
            char log_msg[512];
            time_t now = time(NULL);
            if (monitor_notified) {
                snprintf(log_msg, sizeof(log_msg), "%ld role=%s user=%s event=Monitor notified of new report %d\n", (long)now, role, user, r.report_id);
            } else {
                snprintf(log_msg, sizeof(log_msg), "%ld role=%s user=%s event=Monitor could not be informed of the event for report %d\n", (long)now, role, user, r.report_id);
            }
            write(log_fd, log_msg, strlen(log_msg));
            close(log_fd);
        }
    }

    return 1;
}

int list_reports(const char *district_id, const char *role) {
    char reports_path[256];
    make_path(reports_path, sizeof(reports_path), district_id, "reports.dat");
    if (!check_access(reports_path, role, WANT_READ)) {
        return 0;
    }

    scan_district_entries(district_id);
    scan_active_report_links();

    int fd = open(reports_path, O_RDONLY);
    if (fd < 0) {
        perror("[ERROR] open reports.dat");
        return 0;
    }

    struct stat st;
    if (stat(reports_path, &st) < 0) {
        perror("[ERROR] stat reports.dat");
        close(fd);
        return 0;
    }

    char mode_str[10];
    mode_to_string(st.st_mode & 0777, mode_str);
    printf("reports.dat => perms:%s size:%lld mtime:%ld\n",
           mode_str, (long long) st.st_size, (long) st.st_mtime);

    Report r;
    while (read(fd, &r, sizeof(r)) == (ssize_t) sizeof(r)) {
        print_report(&r);
    }

    close(fd);
    return 1;
}

int view_report(const char *district_id, const char *role, int report_id) {
    char reports_path[256];
    make_path(reports_path, sizeof(reports_path), district_id, "reports.dat");
    if (!check_access(reports_path, role, WANT_READ)) {
        return 0;
    }

    int fd = open(reports_path, O_RDONLY);
    if (fd < 0) {
        perror("[ERROR] open reports.dat");
        return 0;
    }

    Report r;
    while (read(fd, &r, sizeof(r)) == (ssize_t) sizeof(r)) {
        if (r.report_id == report_id) {
            print_report(&r);
            close(fd);
            return 1;
        }
    }

    close(fd);
    fprintf(stderr, "[ERROR] Report %d not found\n", report_id);
    return 0;
}

int remove_report_by_id(const char *district_id, const char *role, int report_id) {
    if (strcmp(role, "manager") != 0) {
        fprintf(stderr, "[ERROR] remove_report is manager-only\n");
        return 0;
    }

    char reports_path[256];
    make_path(reports_path, sizeof(reports_path), district_id, "reports.dat");
    if (!check_access(reports_path, role, WANT_WRITE)) {
        return 0;
    }

    int fd = open(reports_path, O_RDWR);
    if (fd < 0) {
        perror("[ERROR] open reports.dat");
        return 0;
    }

    off_t file_pos = 0;
    off_t remove_offset = -1;
    Report current;
    while (read(fd, &current, sizeof(current)) == (ssize_t) sizeof(current)) {
        if (current.report_id == report_id) {
            remove_offset = file_pos;
            break;
        }
        file_pos += (off_t) sizeof(Report);
    }

    if (remove_offset < 0) {
        close(fd);
        fprintf(stderr, "[ERROR] Report %d not found\n", report_id);
        return 0;
    }

    off_t read_offset = remove_offset + (off_t) sizeof(Report);
    Report next;
    while (1) {
        if (lseek(fd, read_offset, SEEK_SET) < 0) {
            perror("[ERROR] lseek read");
            close(fd);
            return 0;
        }

        ssize_t n = read(fd, &next, sizeof(next));
        if (n == 0) {
            break;
        }
        if (n != (ssize_t) sizeof(next)) {
            perror("[ERROR] read shift");
            close(fd);
            return 0;
        }

        if (lseek(fd, read_offset - (off_t) sizeof(Report), SEEK_SET) < 0) {
            perror("[ERROR] lseek write");
            close(fd);
            return 0;
        }
        if (write(fd, &next, sizeof(next)) != (ssize_t) sizeof(next)) {
            perror("[ERROR] write shift");
            close(fd);
            return 0;
        }

        read_offset += (off_t) sizeof(Report);
    }

    struct stat st;
    if (fstat(fd, &st) < 0) {
        perror("[ERROR] fstat before truncate");
        close(fd);
        return 0;
    }

    if (ftruncate(fd, st.st_size - (off_t) sizeof(Report)) < 0) {
        perror("[ERROR] ftruncate");
        close(fd);
        return 0;
    }

    close(fd);
    printf("[OK] Removed report %d\n", report_id);
    return 1;
}

int update_district_threshold(const char *district_id, const char *role, int threshold) {
    if (strcmp(role, "manager") != 0) {
        fprintf(stderr, "[ERROR] update_threshold is manager-only\n");
        return 0;
    }

    char cfg_path[256];
    make_path(cfg_path, sizeof(cfg_path), district_id, "district.cfg");

    if (!validate_exact_permissions(cfg_path, 0640)) {
        return 0;
    }
    if (!check_access(cfg_path, role, WANT_WRITE)) {
        return 0;
    }

    int fd = open(cfg_path, O_WRONLY | O_TRUNC);
    if (fd < 0) {
        perror("[ERROR] open district.cfg");
        return 0;
    }

    char line[64];
    int len = snprintf(line, sizeof(line), "threshold=%d\n", threshold);
    if (write(fd, line, (size_t) len) < 0) {
        perror("[ERROR] write district.cfg");
        close(fd);
        return 0;
    }

    close(fd);
    printf("[OK] Threshold set to %d\n", threshold);
    return 1;
}

int remove_district(const char *district_id, const char *role) {
    if (strcmp(role, "manager") != 0) {
        fprintf(stderr, "[ERROR] remove_district is manager-only\n");
        return 0;
    }

    char linkname[256];
    snprintf(linkname, sizeof(linkname), "active_reports-%s", district_id);
    unlink(linkname);

    pid_t pid = fork();
    if (pid < 0) {
        perror("[ERROR] fork");
        return 0;
    } else if (pid == 0) {
        // Child process
        char dir_path[256];
        snprintf(dir_path, sizeof(dir_path), "%s", district_id);
        execl("/bin/rm", "rm", "-rf", dir_path, (char *)NULL);
        perror("[ERROR] execl");
        exit(1);
    } else {
        // Parent process
        int status;
        waitpid(pid, &status, 0);
        if (WIFEXITED(status) && WEXITSTATUS(status) == 0) {
            printf("[OK] District %s removed\n", district_id);
            return 1;
        } else {
            fprintf(stderr, "[ERROR] Failed to remove district %s\n", district_id);
            return 0;
        }
    }
}
