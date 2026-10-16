#include <iostream>
#include <vector>
#include <string>
#include <unordered_map>
#include <chrono>
#include <thread>
#include <iomanip>
#include <csignal>
#include <unistd.h>
#include <sys/types.h>
#include <sys/sysctl.h>
#include <libproc.h>

#include "../../database/oracle_db.h"

using namespace std;

volatile sig_atomic_t keepRunning = 1;

void handleSignal(int signalNumber) {
    (void)signalNumber;
    keepRunning = 0;
}

struct ProcessInfo {
    int pid;
    string name;
    double cpuTime;
    double memory;
};

vector<ProcessInfo> getProcesses() {
    vector<ProcessInfo> processes;

    int mib[3] = {CTL_KERN, KERN_PROC, KERN_PROC_ALL};
    size_t size = 0;

    if (sysctl(mib, 3, NULL, &size, NULL, 0) == -1) {
        perror("Unable to get process list");
        return processes;
    }

    vector<kinfo_proc> entries(size / sizeof(kinfo_proc) + 1);

    if (sysctl(mib, 3, entries.data(), &size, NULL, 0) == -1) {
        perror("Unable to read process list");
        return processes;
    }

    size_t count = size / sizeof(kinfo_proc);

    for (size_t i = 0; i < count; i++) {
        int pid = entries[i].kp_proc.p_pid;

        if (pid <= 0) {
            continue;
        }

        proc_taskinfo task {};
        char name[256] = {};

        int result = proc_pidinfo(
            pid, PROC_PIDTASKINFO, 0, &task, sizeof(task)
        );

        int nameResult = proc_name(pid, name, sizeof(name));

        if (result != sizeof(task) || nameResult <= 0) {
            continue;
        }

        ProcessInfo p;
        p.pid = pid;
        p.name = name;
        p.cpuTime =
            (task.pti_total_user + task.pti_total_system)
            / 1000000000.0;
        p.memory = task.pti_resident_size / (1024.0 * 1024.0);

        processes.push_back(p);
    }

    return processes;
}

int getCoreCount() {
    int cores = 1;
    size_t size = sizeof(cores);

    if (sysctlbyname("hw.logicalcpu", &cores, &size, NULL, 0) == -1
        || cores < 1) {
        cores = 1;
    }

    return cores;
}

int main() {
    signal(SIGINT, handleSignal);

    char* password = getpass("Enter Oracle password: ");

    if (password == nullptr) {
        cerr << "Could not read Oracle password.\n";
        return 1;
    }

    OracleDB db;

    if (!db.connect(
            "KERNEL_APP",
            password,
            "//localhost:1521/FREEPDB1")) {
        cerr << "Database connection failed. Monitor not started.\n";
        return 1;
    }

    const int computerId = 1;
    const int cores = getCoreCount();
    const int databaseInterval = 5;

    unordered_map<int, double> previousCpuTime;
    int cycle = 0;
    bool firstRun = true;
    auto previousTime = chrono::steady_clock::now();

    cout << "\nLive Process Monitor + Oracle Storage\n";
    cout << "Logical CPU cores: " << cores << '\n';
    cout << "Database sampling interval: "
         << databaseInterval << " seconds\n";
    cout << "Press Ctrl+C to stop.\n";

    while (keepRunning) {
        auto start = chrono::steady_clock::now();
        vector<ProcessInfo> processes = getProcesses();

        double elapsed = chrono::duration<double>(
            start - previousTime
        ).count();

        unordered_map<int, double> currentCpuTime;
        unordered_map<int, double> currentCpuUsage;

        cout << "\n";
        cout << left << setw(9) << "PID"
             << setw(27) << "PROCESS"
             << setw(13) << "CPU (%)"
             << "MEMORY (MB)\n";

        cout << string(65, '-') << '\n';

        for (const auto& p : processes) {
            currentCpuTime[p.pid] = p.cpuTime;

            cout << left << setw(9) << p.pid
                 << setw(27) << p.name;

            auto it = previousCpuTime.find(p.pid);

            if (!firstRun && it != previousCpuTime.end()
                && elapsed > 0) {
                double cpuDifference = p.cpuTime - it->second;

                if (cpuDifference < 0) {
                    cpuDifference = 0;
                }

                double cpuUsage =
                    (cpuDifference / (elapsed * cores)) * 100.0;

                if (cpuUsage > 100.0) {
                    cpuUsage = 100.0;
                }

                currentCpuUsage[p.pid] = cpuUsage;

                cout << setw(13) << fixed << setprecision(2)
                     << cpuUsage;
            } else {
                cout << setw(13) << "N/A";
            }

            cout << fixed << setprecision(2)
                 << p.memory << '\n';
        }

        cycle++;

        // Persist measurements every five monitoring cycles.
        if (cycle % databaseInterval == 0) {
            int saved = 0;

            for (const auto& p : processes) {
                auto cpuIt = currentCpuUsage.find(p.pid);

                // Skip processes without a valid CPU measurement.
                if (cpuIt == currentCpuUsage.end()) {
                    continue;
                }

                int processId = db.getOrCreateProcess(
                    computerId, p.pid, p.name
                );

                if (processId == -1) {
                    continue;
                }

                if (db.insertResourceUsage(
                        processId, cpuIt->second, p.memory)) {
                    saved++;
                }
            }

            cout << "\n[Oracle] Saved " << saved
                 << " resource measurements.\n";
        }

        cout << "\nTotal processes listed: "
             << processes.size() << '\n';

        previousCpuTime = currentCpuTime;
        previousTime = chrono::steady_clock::now();
        firstRun = false;

        for (int i = 0; i < 10 && keepRunning; i++) {
            this_thread::sleep_for(chrono::milliseconds(100));
        }
    }

    cout << "\nStopping monitor and closing Oracle connection...\n";
    db.disconnect();
    cout << "Monitor stopped.\n";

    return 0;
}
