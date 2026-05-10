# OS City Manager

**AUTHOR**: Vlad Tomici  
**GROUP**: 2.2  

## Overview
OS City Manager is a system programming project built in C that simulates a basic report management system for city districts. It features role-based access control (manager vs. inspector), file-based data persistence, and inter-process communication using signals. 

The application is split into two executables:
1. `OS-CityManager`: The main CLI tool for managing districts, creating reports, filtering data, and modifying configurations.
2. `monitor_reports`: A background daemon process that listens for new reports via signals (`SIGUSR1`) and can be stopped gracefully (`SIGINT`).

---

## Components & Architecture

- **`src/main.c`**: Handles command-line argument parsing, basic validation, and routes commands to the appropriate operation functions.
- **`src/operations.c`**: Contains the core logic for the system's commands:
  - Adding, viewing, listing, and removing reports.
  - Deleting an entire district directory using a child process (`fork` + `exec`).
  - Notifying the monitor daemon via signals when a new report is added.
- **`src/permissions.c`**: Manages the file system layer:
  - Creates directories and enforces strict POSIX file permissions (`chmod`).
  - Validates role-based access control for reading, writing, and executing.
  - Manages logging (`logged_district`) and symbolic links (`active_reports-*`).
- **`src/filter.c`**: Parses and executes dynamic filter queries on the reports (e.g., filtering by severity, category, or inspector).
- **`src/monitor_reports.c`**: A standalone background process that writes its PID to `.monitor_pid`, blocks until signals arrive, and outputs a notification when a new report is added.

---

## How to Build

The project uses a `Makefile` to compile both executables cleanly.

```sh
# Compile all binaries
make

# Clean up build artifacts, executables, and symlinks
make clean
```

---

## How to Run

### 1. Starting the Monitor (Phase 2 Requirement)
Before adding reports, start the monitor program in the background. It will create a `.monitor_pid` file to let the CLI tool know its process ID.

```sh
./monitor_reports &
```

### 2. Using the City Manager CLI
The basic command syntax for `OS-CityManager` is:
```sh
./OS-CityManager --role <inspector|manager> --user <name> --<command> <district_id> [args...]
```

#### Roles:
- **`manager`**: Has full administrative privileges (add, view, remove reports, delete districts, change thresholds).
- **`inspector`**: Has limited privileges (can only add, list, view, and filter reports).

### Examples of Common Commands

**Add a new report to a district:**
```sh
./OS-CityManager --role inspector --user Alice --add d1
```
*(You will be prompted via standard input to enter the latitude, longitude, category, severity, and description).*

**List all reports in a district:**
```sh
./OS-CityManager --role manager --user Alice --list d1
```

**View a specific report:**
```sh
./OS-CityManager --role inspector --user Bob --view d1 1
```

**Filter reports:**
```sh
./OS-CityManager --role manager --user Alice --filter d1 severity:>=:2 category:=:pothole
```

**Remove a specific report (Manager only):**
```sh
./OS-CityManager --role manager --user Admin --remove_report d1 1
```

**Remove an entire district (Manager only):**
```sh
./OS-CityManager --role manager --user Admin --remove_district d1
```

### 3. Stopping the Monitor
To gracefully shut down the monitor, send a `SIGINT` (Ctrl+C if in foreground, or `kill` if in background).

```sh
kill -INT %1
```
*(The monitor will cleanly delete its `.monitor_pid` file before exiting).*