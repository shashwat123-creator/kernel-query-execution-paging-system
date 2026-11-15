
#ifndef ORACLE_DB_H
#define ORACLE_DB_H

#include <occi.h>
#include <string>
#include <vector>

#include "../os/scheduling/scheduler.h"
#include "../os/deadlock/bankers.h"
#include "../os/file-allocation/contiguous.h"
#include "../os/paging/demand_paging.h"

class OracleDB {
private:
    oracle::occi::Environment* env;
    oracle::occi::Connection* conn;

public:
    OracleDB();
    ~OracleDB();

    bool connect(
        const std::string& username,
        const std::string& password,
        const std::string& connectString
    );

    void disconnect();
    bool isConnected() const;

    int getOrCreateProcess(
        int computerId,
        int pid,
        const std::string& processName
    );

    bool insertResourceUsage(
        int processId,
        double cpuUsage,
        double memoryUsage
    );

    std::string getProcessesJson(int computerId);
    std::string getMetricsJson(int computerId);

    // CPU Scheduling
    int saveSchedulingRun(
        const std::string& algorithm,
        int timeQuantum,
        const std::vector<Process>& processes,
        const SchedulingReport& report
    );

    // Deadlock Detection and Avoidance
    int saveDeadlockRun(
        const std::vector<int>& available,
        const std::vector<std::vector<int>>& allocation,
        const std::vector<std::vector<int>>& maximum,
        const DeadlockResult& result
    );

    // File Allocation
    int saveFileAllocationRun(
        int totalBlocks,
        int freeBlocks,
        const std::vector<AllocationResult>& attempts,
        int filesAllocated
    );

    // Paging Simulation
    int savePagingRun(
        int frameCount,
        const std::vector<int>& referenceString,
        const PagingReport& report
    );
};

#endif
