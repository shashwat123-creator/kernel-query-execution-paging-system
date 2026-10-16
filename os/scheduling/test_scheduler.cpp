#include "scheduler.h"
#include <iomanip>
#include <iostream>

void printReport(const char* name, const SchedulingReport& r) {
    std::cout << "\n=== " << name << " ===\n";
    std::cout << "PID\tCT\tTAT\tWT\tRT\n";

    for (const auto& p : r.results) {
        std::cout << p.pid << '\t'
                  << p.completionTime << '\t'
                  << p.turnaroundTime << '\t'
                  << p.waitingTime << '\t'
                  << p.responseTime << '\n';
    }

    std::cout << std::fixed << std::setprecision(2)
              << "Average waiting time: " << r.averageWaitingTime << '\n'
              << "Average turnaround time: "
              << r.averageTurnaroundTime << '\n';

    std::cout << "Gantt chart segments:\n";
    for (const auto& g : r.gantt)
        std::cout << "| P" << g.pid << " [" << g.startTime
                  << "-" << g.endTime << "] ";
    std::cout << "|\n";
}

int main() {
    std::vector<Process> processes = {
        {1, 0, 5},
        {2, 1, 3},
        {3, 2, 1}
    };

    printReport("FCFS", fcfs(processes));
    printReport("Round Robin (quantum = 2)",
                roundRobin(processes, 2));
    return 0;
}
