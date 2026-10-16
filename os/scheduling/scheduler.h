#ifndef SCHEDULER_H
#define SCHEDULER_H

#include <string>
#include <vector>

struct Process {
    int pid;
    int arrivalTime;
    int burstTime;
};

struct ScheduleResult {
    int pid;
    int completionTime;
    int turnaroundTime;
    int waitingTime;
    int responseTime;
};

struct GanttSegment {
    int pid;
    int startTime;
    int endTime;
};

struct SchedulingReport {
    std::vector<ScheduleResult> results;
    std::vector<GanttSegment> gantt;
    double averageWaitingTime = 0;
    double averageTurnaroundTime = 0;
};

SchedulingReport fcfs(std::vector<Process> processes);
SchedulingReport roundRobin(std::vector<Process> processes, int quantum);

#endif
