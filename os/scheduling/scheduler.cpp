#include "scheduler.h"

#include <algorithm>
#include <queue>
#include <stdexcept>
#include <unordered_map>

SchedulingReport fcfs(std::vector<Process> processes) {
    if (processes.empty())
        return {};

    for (const auto& p : processes) {
        if (p.pid <= 0 || p.arrivalTime < 0 || p.burstTime <= 0)
            throw std::invalid_argument("Invalid process input");
    }

    std::stable_sort(processes.begin(), processes.end(),
        [](const Process& a, const Process& b) {
            return a.arrivalTime < b.arrivalTime;
        });

    SchedulingReport report;
    int time = 0;

    for (const auto& p : processes) {
        if (time < p.arrivalTime)
            time = p.arrivalTime;

        int start = time;
        int completion = start + p.burstTime;
        int turnaround = completion - p.arrivalTime;
        int waiting = turnaround - p.burstTime;

        report.gantt.push_back({p.pid, start, completion});
        report.results.push_back({
            p.pid, completion, turnaround, waiting,
            start - p.arrivalTime
        });

        time = completion;
        report.averageWaitingTime += waiting;
        report.averageTurnaroundTime += turnaround;
    }

    report.averageWaitingTime /= processes.size();
    report.averageTurnaroundTime /= processes.size();
    return report;
}

SchedulingReport roundRobin(std::vector<Process> processes, int quantum) {
    if (quantum <= 0)
        throw std::invalid_argument("Time quantum must be positive");

    for (const auto& p : processes) {
        if (p.pid <= 0 || p.arrivalTime < 0 || p.burstTime <= 0)
            throw std::invalid_argument("Invalid process input");
    }

    if (processes.empty())
        return {};

    std::stable_sort(processes.begin(), processes.end(),
        [](const Process& a, const Process& b) {
            return a.arrivalTime < b.arrivalTime;
        });

    const int n = static_cast<int>(processes.size());
    std::vector<int> remaining(n);
    std::vector<int> firstStart(n, -1);
    std::vector<int> completion(n, 0);
    std::queue<int> ready;
    SchedulingReport report;

    for (int i = 0; i < n; ++i)
        remaining[i] = processes[i].burstTime;

    int time = 0;
    int next = 0;
    int finished = 0;

    while (finished < n) {
        if (ready.empty() && next < n && time < processes[next].arrivalTime)
            time = processes[next].arrivalTime;

        while (next < n && processes[next].arrivalTime <= time)
            ready.push(next++);

        if (ready.empty())
            continue;

        int i = ready.front();
        ready.pop();

        if (firstStart[i] == -1)
            firstStart[i] = time;

        int start = time;
        int runTime = std::min(quantum, remaining[i]);
        time += runTime;
        remaining[i] -= runTime;
        report.gantt.push_back({processes[i].pid, start, time});

        while (next < n && processes[next].arrivalTime <= time)
            ready.push(next++);

        if (remaining[i] > 0) {
            ready.push(i);
        } else {
            completion[i] = time;
            ++finished;
        }
    }

    for (int i = 0; i < n; ++i) {
        int turnaround = completion[i] - processes[i].arrivalTime;
        int waiting = turnaround - processes[i].burstTime;
        int response = firstStart[i] - processes[i].arrivalTime;

        report.results.push_back({
            processes[i].pid, completion[i], turnaround, waiting, response
        });

        report.averageWaitingTime += waiting;
        report.averageTurnaroundTime += turnaround;
    }

    report.averageWaitingTime /= n;
    report.averageTurnaroundTime /= n;
    return report;
}
