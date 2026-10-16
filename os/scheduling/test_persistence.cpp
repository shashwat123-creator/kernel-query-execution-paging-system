#include "../../database/oracle_db.h"
#include "scheduler.h"
#include <cstdlib>
#include <iostream>

int main() {
    const char* password = std::getenv("ORACLE_APP_PASSWORD");
    if (!password) {
        std::cerr << "ORACLE_APP_PASSWORD is not set\n";
        return 1;
    }

    OracleDB db;
    if (!db.connect("KERNEL_APP", password,
                    "//localhost:1521/FREEPDB1")) {
        return 1;
    }

    std::vector<Process> processes = {
        {1, 0, 5},
        {2, 1, 3},
        {3, 2, 1}
    };

    SchedulingReport report = fcfs(processes);
    int runId = db.saveSchedulingRun(
        "FCFS", 0, processes, report
    );

    if (runId < 0) {
        std::cerr << "Scheduling run was not saved\n";
        db.disconnect();
        return 1;
    }

    std::cout << "Saved FCFS scheduling run. Run ID: "
              << runId << '\n';

    db.disconnect();
    return 0;
}
