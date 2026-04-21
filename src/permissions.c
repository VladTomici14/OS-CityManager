
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../include/city_manager.h"
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>
#include <dirent.h>

// ----- method for creating directories / files and setting permissions -----
void initialize_district(const char *district_id) {
    if (mkdir(district_id, 0750) < 0 && errno != EEXIST) {
        perror("[ERROR] mkdir");
        return;
    }

    if (chmod(district_id, 0750) < 0) {
        perror("[ERROR] chmod district");
        return;
    }

    char filepath[256];

    snprintf(filepath, sizeof(filepath), "%s/reports.dat", district_id);
    int fd1 = open(filepath, O_CREAT | O_WRONLY, 0664);
    if (fd1 != -1) { close(fd1); }
    chmod(filepath, 0664);

    snprintf(filepath, sizeof(filepath), "%s/district.cfg", district_id);
    int fd2 = open(filepath, O_CREAT | O_WRONLY, 0640);
    if (fd2 != -1) {
        struct stat st;
        if (fstat(fd2, &st) == 0 && st.st_size == 0) {
            const char *default_threshold = "threshold=2\n";
            write(fd2, default_threshold, strlen(default_threshold));
        }
        close(fd2);
    }
    chmod(filepath, 0640);

    snprintf(filepath, sizeof(filepath), "%s/logged_district", district_id);
    int fd3 = open(filepath, O_CREAT | O_WRONLY, 0644);
    if (fd3 != -1) { close(fd3); }
    chmod(filepath, 0644);

    update_active_reports_symlink(district_id);
}

// ----- method for checking access -----
int check_access(const char *filepath, const char *role, int action) {
    struct stat file_stat;

    if (stat(filepath, &file_stat) < 0) {
        perror("Error stating file");
        return 0;
    }

    mode_t mode = file_stat.st_mode;

    if (strcmp(role, "manager") == 0) {
        if (action == WANT_READ  && (mode & S_IRUSR)) return 1;
        if (action == WANT_WRITE && (mode & S_IWUSR)) return 1;
        if (action == WANT_EXEC  && (mode & S_IXUSR)) return 1;
    }
    else if (strcmp(role, "inspector") == 0) {
        if (action == WANT_READ  && (mode & S_IRGRP)) return 1;
        if (action == WANT_WRITE && (mode & S_IWGRP)) return 1;
        if (action == WANT_EXEC  && (mode & S_IXGRP)) return 1;
    }

    fprintf(stderr, "Access Denied: Role '%s' does not have permission for this action on %s.\n", role, filepath);
    return 0;
}

int validate_exact_permissions(const char *filepath, mode_t expected_mode) {
    struct stat st;
    if (stat(filepath, &st) < 0) {
        perror("[ERROR] stat for permission validation");
        return 0;
    }

    mode_t effective_mode = st.st_mode & 0777;
    if (effective_mode != expected_mode) {
        char got[10], expected[10];
        mode_to_string(effective_mode, got);
        mode_to_string(expected_mode, expected);
        fprintf(stderr, "[ERROR] Wrong permissions on %s. Expected %s (%03o), got %s (%03o)\n",
                filepath, expected, expected_mode, got, effective_mode);
        return 0;
    }

    return 1;
}

// ----- method for mode to string -----
void mode_to_string(mode_t mode, char *str) {
    strcpy(str, "---------"); 

    if (mode & S_IRUSR) { str[0] = 'r'; }
    if (mode & S_IWUSR) { str[1] = 'w'; }
    if (mode & S_IXUSR) { str[2] = 'x'; }

    if (mode & S_IRGRP) { str[3] = 'r'; }
    if (mode & S_IWGRP) { str[4] = 'w'; }
    if (mode & S_IXGRP) { str[5] = 'x'; }

    if (mode & S_IROTH) { str[6] = 'r'; }
    if (mode & S_IWOTH) { str[7] = 'w'; }
    if (mode & S_IXOTH) { str[8] = 'x'; }
    
    str[9] = '\0';
}

void update_active_reports_symlink(const char *district_id) {
    char target[256];
    char linkname[256];

    snprintf(target, sizeof(target), "%s/reports.dat", district_id);
    snprintf(linkname, sizeof(linkname), "active_reports-%s", district_id);

    struct stat st;
    if (lstat(linkname, &st) == 0) {
        unlink(linkname);
    }

    if (symlink(target, linkname) < 0) {
        perror("[WARN] Could not create active reports symlink");
    }
}

void scan_district_entries(const char *district_id) {
    DIR *dir = opendir(district_id);
    if (!dir) {
        perror("[ERROR] opendir district");
        return;
    }

    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL) {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
            continue;
        }

        char fullpath[512];
        snprintf(fullpath, sizeof(fullpath), "%s/%s", district_id, entry->d_name);

        struct stat st;
        if (lstat(fullpath, &st) < 0) {
            perror("[WARN] lstat failed");
            continue;
        }

        if (S_ISLNK(st.st_mode)) {
            struct stat target_st;
            if (stat(fullpath, &target_st) < 0) {
                fprintf(stderr, "[WARN] Dangling symlink detected: %s\n", fullpath);
            }
        }
    }

    closedir(dir);
}

void scan_active_report_links(void) {
    DIR *dir = opendir(".");
    if (!dir) {
        perror("[ERROR] opendir .");
        return;
    }

    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL) {
        if (strncmp(entry->d_name, "active_reports-", 15) != 0) {
            continue;
        }

        struct stat st;
        if (lstat(entry->d_name, &st) < 0) {
            perror("[WARN] lstat active link failed");
            continue;
        }

        if (!S_ISLNK(st.st_mode)) {
            continue;
        }

        struct stat target_st;
        if (stat(entry->d_name, &target_st) < 0) {
            fprintf(stderr, "[WARN] Dangling symlink detected: %s\n", entry->d_name);
        }
    }

    closedir(dir);
}

int log_action(const char *district_id, const char *role, const char *user, const char *command) {
    char logpath[256];
    snprintf(logpath, sizeof(logpath), "%s/logged_district", district_id);

    if (strcmp(role, "inspector") == 0) {
        fprintf(stderr, "[INFO] Inspector action not logged because logged_district is manager-writable only.\n");
        return 1;
    }

    if (!check_access(logpath, role, WANT_WRITE)) {
        return 0;
    }

    int fd = open(logpath, O_WRONLY | O_APPEND);
    if (fd < 0) {
        perror("[ERROR] open logged_district");
        return 0;
    }

    time_t now = time(NULL);
    char line[512];
    int len = snprintf(line, sizeof(line), "%ld role=%s user=%s command=%s\n",
                       (long)now, role, user, command);
    if (write(fd, line, (size_t)len) < 0) {
        perror("[ERROR] write logged_district");
        close(fd);
        return 0;
    }

    close(fd);
    return 1;
}
