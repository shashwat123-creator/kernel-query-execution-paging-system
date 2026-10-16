#ifndef ORACLE_DB_H
#define ORACLE_DB_H

#include <occi.h>
#include "../os/scheduling/scheduler.h"
#include "../os/deadlock/bankers.h"
#include "../os/file-allocation/contiguous.h"
#include <string>
#include<vector>

class OracleDB {
private:
    oracle::occi::Environment* env;
    oracle::occi::Connection* conn;

public:
    OracleDB();
    ~OracleDB();

    bool connect(const std::string& username,
                 const std::string& password,
                 const std::string& connectString);

    void disconnect();
    bool isConnected() const;

    int getOrCreateProcess(int computerId, int pid,
                           const std::string& processName);

    bool insertResourceUsage(int processId,
                             double cpuUsage,
                             double memoryUsage);
std::string getProcessesJson(int computerId);
    std::string getMetricsJson(int computerId);

    int saveSchedulingRun(
        const std::string& algorithm,
        int timeQuantum,
        const std::vector<Process>& processes,
        const SchedulingReport& report
    );


    int saveDeadlockRun(
        const std::vector<int>& available,
        const std::vector<std::vector<int>>& allocation,
        const std::vector<std::vector<int>>& maximum,
        const DeadlockResult& result
    );


    int saveFileAllocationRun(
        int totalBlocks,
        int freeBlocks,
        const std::vector<AllocationResult>& attempts,
        int filesAllocated
    );

};

#endif
