# Kernel Query Execution and Paging System

**An Operating Systems and Database Management System Integrated Project**

A C++-based system that demonstrates core Operating System concepts and integrates them with Oracle Database for persistent storage, supported by a web-based dashboard for visualization and interaction.

The project connects process monitoring, CPU scheduling, deadlock avoidance, and file allocation with database operations to demonstrate how operating system mechanisms can be represented, executed, and recorded through a unified application.

## 📌 Project Overview

Operating Systems manage processes, CPU resources, and memory, while Database Management Systems provide structured data storage and retrieval.

The **Kernel Query Execution and Paging System** aims to integrate these concepts into a single application. It implements selected OS algorithms in C++, stores execution results in Oracle Database, and provides a browser-based interface for interacting with the system.

The current implementation focuses on process monitoring, CPU scheduling, Banker's algorithm for deadlock avoidance, and contiguous file allocation. Paging and demand paging are planned extensions.

## 🎯 Objectives

- Demonstrate fundamental Operating System algorithms through C++ implementations.
- Monitor running processes and record resource usage.
- Implement CPU scheduling algorithms and compare their execution results.
- Apply Banker's algorithm to analyze safe and unsafe resource-allocation states.
- Simulate contiguous file allocation and demonstrate external fragmentation.
- Integrate Oracle Database for persistent storage and SQL-based data retrieval.
- Develop a web dashboard to interact with the backend and visualize results.
- Apply modular programming and basic system-design principles.

## ✨ Features

### 1. Process Monitoring

- Collect process-related information from the host system.
- Record process and resource-usage information in Oracle Database.
- Use structured database tables to maintain monitoring records.

### 2. CPU Scheduling

- First Come, First Served (FCFS).
- Round Robin (RR).
- Execute scheduling simulations using process input.
- Display scheduling results through the dashboard.
- Persist scheduling runs and their results in Oracle Database.

### 3. Deadlock Avoidance

- Implement the Banker's algorithm.
- Accept process, resource, allocation, maximum-demand, and available-resource information.
- Determine whether the system is in a safe or unsafe state.
- Display the safety result and persist execution records.

### 4. File Allocation

- Simulate contiguous file allocation on a disk represented as a sequence of blocks.
- Allocate disk blocks to files using the implemented allocation strategy.
- Track allocated and free blocks.
- Demonstrate allocation failures caused by insufficient contiguous space and external fragmentation.
- Save allocation attempts and their results in Oracle Database.

### 5. Web Dashboard

- Browser-based interface built with HTML, CSS, and JavaScript.
- Communicates with the C++ backend through HTTP APIs.
- Provides interactive panels for supported OS modules.
- Displays results returned by the backend.

### 6. Oracle Database Integration

- Uses Oracle Database and Oracle Call Interface (OCCI).
- Stores process information, resource usage, scheduling results, deadlock analysis, and file-allocation results.
- Uses SQL tables to maintain persistent execution records.

## 🛠️ Technology Stack

| Component | Technologies |
|---|---|
| Core programming | C++17 |
| Operating System algorithms | C++ |
| Backend | C++ and HTTP API |
| Frontend | HTML5, CSS3, JavaScript |
| Database | Oracle Database |
| Database connectivity | Oracle OCCI |
| Database client | Oracle Instant Client |
| Containerization | Docker |
| Development environment | macOS, VS Code, Clang++ |
| Version control | Git and GitHub |

## 🏗️ System Architecture

```text
                  USER
                   |
                   v
          Web Dashboard
       HTML / CSS / JavaScript
                   |
                HTTP API
                   |
                   v
           C++ Backend Server
                   |
          +--------+--------+
          |        |        |
          v        v        v
       Process   Scheduling Deadlock
       Monitor   Algorithms Avoidance
          |        |        |
          +--------+--------+
                   |
                   v
            File Allocation
                   |
                   v
           OracleDB Data Layer
              (OCCI)
                   |
                   v
           Oracle Database
           Persistent Storage
```

The frontend sends requests to the C++ backend. The backend invokes the relevant module, interacts with the Oracle database through the database layer, and returns results to the frontend.

## 📂 Project Structure

```text
kernel-query-execution-paging-system/
│
├── agent/
│   └── src/
│       └── process_monitor.cpp
│
├── backend/
│   ├── include/
│   │   └── httplib.h
│   └── server.cpp
│
├── database/
│   ├── oracle_db.h
│   └── oracle_db.cpp
│
├── frontend/
│   ├── index.html
│   ├── css/
│   │   └── style.css
│   └── js/
│       └── app.js
│
├── os/
│   ├── scheduling/
│   │   └── scheduler.cpp
│   ├── deadlock/
│   │   └── bankers.cpp
│   └── file-allocation/
│       ├── contiguous.h
│       └── contiguous.cpp
│
├── tests/
├── docs/
├── README.md
├── .gitignore
└── main.cpp
```

*Note: Some directories may contain additional source files and tests as development progresses.*

## ⚙️ Prerequisites

Before running the project, install or configure:

- C++ compiler supporting C++17.
- Docker Desktop.
- Oracle Database Free running in Docker, configured with the `FREEPDB1` service.
- Oracle Instant Client and its SDK.
- Git.
- Python 3 for serving the frontend locally.
- A modern web browser.

## 🚀 Setup and Execution

### 1. Clone the repository

```bash
git clone https://github.com/shashwat123-creator/kernel-query-execution-paging-system.git
cd kernel-query-execution-paging-system
```

### 2. Start Oracle Database

If your existing Oracle container is named `oracle-free`, start it if it is stopped:

```bash
docker start oracle-free
```

Verify the container:

```bash
docker ps
```

Ensure that the Oracle database is ready and that the `FREEPDB1` service is accessible on port `1521`.

### 3. Configure database credentials

Set the Oracle application password in your terminal environment. Use the password configured for your own Oracle application user.

```bash
export ORACLE_APP_PASSWORD='YOUR_ORACLE_PASSWORD'
```

Do not commit passwords, access tokens, or other secrets to GitHub.

The application database user must have the required permissions on the project tables.

### 4. Configure Oracle Instant Client

If Oracle Instant Client is installed in a different directory, update the paths accordingly.

For the development setup used on macOS:

```bash
export ORACLE_HOME="$HOME/Downloads/instantclient_23_26"
export DYLD_LIBRARY_PATH="$ORACLE_HOME"
```

### 5. Compile the C++ backend

Run the following command from the project root, adjusting the Instant Client path if needed:

```bash
clang++ -std=c++17 -Wall -Wextra -pedantic \
  -I"$HOME/Downloads/instantclient_23_26/sdk/include" \
  -Ibackend/include \
  backend/server.cpp \
  database/oracle_db.cpp \
  os/scheduling/scheduler.cpp \
  os/deadlock/bankers.cpp \
  os/file-allocation/contiguous.cpp \
  -L"$HOME/Downloads/instantclient_23_26" \
  -locci -lclntsh \
  -Wl,-rpath,"$HOME/Downloads/instantclient_23_26" \
  -pthread \
  -o backend/server
```

### 6. Run the backend

```bash
export ORACLE_APP_PASSWORD='YOUR_ORACLE_PASSWORD'
./backend/server
```

The backend is configured to listen on:

```text
http://127.0.0.1:8080
```

### 7. Run the frontend

Open another terminal in the project root:

```bash
python3 -m http.server 5500 --directory frontend
```

Open the dashboard in your browser:

**http://localhost:5500**

Keep the backend and frontend terminals running while using the dashboard.

### 8. Run the process monitor

If you want to run the process-monitoring agent separately, compile it using the Oracle OCCI headers and libraries, then run it with the required database configuration.

The source file is:

```text
agent/src/process_monitor.cpp
```

The monitor requires an accessible Oracle database and valid application credentials.

## 🗄️ Database Design

The database layer uses Oracle Database to store information generated by the application.

The current schema includes tables for the following purposes:

| Table | Purpose |
|---|---|
| `COMPUTER` | Stores computer or system information. |
| `PROCESS` | Stores process-related records. |
| `RESOURCE_USAGE` | Stores process resource-usage information. |
| `SCHEDULING_RUN` | Stores CPU scheduling execution records. |
| `SCHEDULING_RESULT` | Stores scheduling results. |
| `DEADLOCK_RUN` | Stores deadlock-analysis execution records. |
| `DEADLOCK_RESOURCE` | Stores resource information for deadlock analysis. |
| `DEADLOCK_PROCESS` | Stores process information for deadlock analysis. |
| `DEADLOCK_PROCESS_RESOURCE` | Stores process-resource relationships. |
| `FILE_ALLOCATION_RUN` | Stores file-allocation execution records. |
| `FILE_ALLOCATION_RESULT` | Stores file-allocation results. |

The schema supports persistent execution records and provides a foundation for relational queries and joins.

## 🧪 Testing

The project includes testing of selected algorithmic modules and database integration.

Current test areas include:

- FCFS and Round Robin scheduling.
- Safe and unsafe states using Banker's algorithm.
- Contiguous file allocation.
- Allocation failures and external fragmentation.
- Persistence of supported module results in Oracle Database.
- Backend API requests and dashboard interactions.

Test results should be verified against the current source code and database configuration before each demonstration.

## 🔒 Security and Configuration

- Keep database credentials outside source files.
- Do not commit `.env` files or local configuration containing secrets.
- Use a dedicated Oracle application account rather than administrative database credentials.
- Restrict the backend to trusted local access during development.
- Validate API inputs before executing database operations.

## 🔮 Future Enhancements

The following features are planned and are not represented as completed functionality in the current implementation:

- Multi-level paging simulation.
- Demand paging and page replacement algorithms.
- Page-fault tracking and page-table visualization.
- Additional CPU scheduling algorithms.
- Enhanced monitoring and performance analytics.
- Expanded test coverage and error handling.
- Improved database reporting and dashboard visualizations.

## 🎓 Academic Relevance

This project integrates concepts from two core computer science subjects:

**Operating Systems**
- Process management and monitoring.
- CPU scheduling.
- Deadlock avoidance.
- File allocation and external fragmentation.
- Planned memory-management and paging simulations.

**Database Management Systems**
- Relational schema design.
- Structured data persistence.
- SQL queries and joins.
- Database connectivity using OCCI.
- Recording and retrieving execution results.

The
