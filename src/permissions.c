
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../include/city_manager.h"
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>

// ----- method for creating directories / files and setting permissions -----
void initialize_district(const char *district_id) {
    mkdir(district_id, 0750);
    chmod(district_id, 0750);

    char filepath[256];

    snprintf(filepath, sizeof(filepath), "%s/reports.dat", district_id);
    int fd1 = open(filepath, O_CREAT | O_WRONLY, 0664);
    if (fd1 != -1) { close(fd1); }
    chmod(filepath, 0664);

    snprintf(filepath, sizeof(filepath), "%s/logged_district.cfg", district_id);
    int fd2 = open(filepath, O_CREAT | O_WRONLY, 0640);
    if (fd2 != -1) { close(fd2); }
    chmod(filepath, 0640);

    snprintf(filepath, sizeof(filepath), "%s/logged_district", district_id);
    int fd3 = open(filepath, O_CREAT | O_WRONLY, 0644);
    if (fd3 != -1) { close(fd3); }
    chmod(filepath, 0644);
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
