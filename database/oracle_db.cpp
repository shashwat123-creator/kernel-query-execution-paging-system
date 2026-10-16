#include <stdexcept>
#include <algorithm>
#include <sstream>
#include "oracle_db.h"
#include <iostream>

using namespace std;
using namespace oracle::occi;

OracleDB::OracleDB() : env(nullptr), conn(nullptr) {}

OracleDB::~OracleDB() {
    disconnect();
}

bool OracleDB::connect(const string& username,
                       const string& password,
                       const string& connectString) {
    try {
        env = Environment::createEnvironment();
        conn = env->createConnection(username, password, connectString);

        cout << "Connected to Oracle Database.\n";
        return true;
    }
    catch (SQLException& e) {
        cerr << "Oracle connection failed: " << e.getMessage() << '\n';

        if (env != nullptr) {
            Environment::terminateEnvironment(env);
            env = nullptr;
        }

        conn = nullptr;
        return false;
    }
}

void OracleDB::disconnect() {
    if (env != nullptr) {
        if (conn != nullptr) {
            env->terminateConnection(conn);
            conn = nullptr;
        }

        Environment::terminateEnvironment(env);
        env = nullptr;
    }
}

bool OracleDB::isConnected() const {
    return conn != nullptr;
}

int OracleDB::getOrCreateProcess(int computerId, int pid,
                                 const string& processName) {
    if (conn == nullptr) return -1;

    Statement* stmt = nullptr;
    ResultSet* rs = nullptr;

    try {
        // Look for an existing process.
        stmt = conn->createStatement(
            "SELECT process_id FROM PROCESS "
            "WHERE computer_id = :1 AND pid = :2"
        );
        stmt->setInt(1, computerId);
        stmt->setInt(2, pid);

        rs = stmt->executeQuery();

        if (rs->next()) {
            int processId = rs->getInt(1);
            stmt->closeResultSet(rs);
            rs = nullptr;
            conn->terminateStatement(stmt);
            stmt = nullptr;
            return processId;
        }

        stmt->closeResultSet(rs);
        rs = nullptr;
        conn->terminateStatement(stmt);
        stmt = nullptr;

        // Insert the process if it does not exist.
        stmt = conn->createStatement(
            "INSERT INTO PROCESS "
            "(computer_id, pid, process_name, process_state) "
            "VALUES (:1, :2, :3, :4)"
        );
        stmt->setInt(1, computerId);
        stmt->setInt(2, pid);
        stmt->setString(3, processName);
        stmt->setString(4, "RUNNING");

        stmt->executeUpdate();
        conn->commit();
        conn->terminateStatement(stmt);
        stmt = nullptr;

        // Read back the generated ID.
        stmt = conn->createStatement(
            "SELECT process_id FROM PROCESS "
            "WHERE computer_id = :1 AND pid = :2"
        );
        stmt->setInt(1, computerId);
        stmt->setInt(2, pid);

        rs = stmt->executeQuery();

        if (rs->next()) {
            int processId = rs->getInt(1);
            stmt->closeResultSet(rs);
            rs = nullptr;
            conn->terminateStatement(stmt);
            stmt = nullptr;
            return processId;
        }

        stmt->closeResultSet(rs);
        rs = nullptr;
        conn->terminateStatement(stmt);
        stmt = nullptr;

        cerr << "Process was inserted but its ID could not be retrieved.\n";
        return -1;
    }
    catch (SQLException& e) {
        cerr << "Process insert/find failed: "
             << e.getMessage() << '\n';

        if (rs != nullptr && stmt != nullptr) {
            try {
                stmt->closeResultSet(rs);
            } catch (...) {}
        }

        if (stmt != nullptr) {
            try {
                conn->terminateStatement(stmt);
            } catch (...) {}
        }

        try {
            conn->rollback();
        } catch (...) {}

        return -1;
    }
}

bool OracleDB::insertResourceUsage(int processId,
                                   double cpuUsage,
                                   double memoryUsage) {
    if (conn == nullptr) return false;

    Statement* stmt = nullptr;

    try {
        stmt = conn->createStatement(
            "INSERT INTO RESOURCE_USAGE "
            "(process_id, cpu_usage_percent, memory_usage_mb) "
            "VALUES (:1, :2, :3)"
        );

        stmt->setInt(1, processId);
        stmt->setDouble(2, cpuUsage);
        stmt->setDouble(3, memoryUsage);

        stmt->executeUpdate();
        conn->commit();
        conn->terminateStatement(stmt);

        return true;
    }
    catch (SQLException& e) {
        cerr << "Resource usage insert failed: "
             << e.getMessage() << '\n';

        if (stmt != nullptr) {
            try { conn->terminateStatement(stmt); } catch (...) {}
        }

        try { conn->rollback(); } catch (...) {}
        return false;
    }
}

std::string OracleDB::getProcessesJson(int computerId) {
    using namespace oracle::occi;

    if (conn == nullptr) {
        return R"({"error":"Database not connected"})";
    }

    Statement* stmt = nullptr;
    ResultSet* rs = nullptr;
    std::ostringstream json;
    bool first = true;

    try {
        stmt = conn->createStatement(
            "SELECT p.pid, p.process_name, c.computer_name "
            "FROM PROCESS p "
            "JOIN COMPUTER c ON p.computer_id = c.computer_id "
            "WHERE p.computer_id = :1 "
            "ORDER BY p.pid"
        );

        stmt->setInt(1, computerId);
        rs = stmt->executeQuery();

        json << "{\"count\":";
        std::ostringstream rows;
        int count = 0;

        rows << "\"processes\":[";

        while (rs->next()) {
            if (!first) {
                rows << ",";
            }
            first = false;

            int pid = rs->getInt(1);
            std::string name = rs->getString(2);
            std::string computer = rs->getString(3);

            auto escapeJson = [](const std::string& value) {
                std::string escaped;
                for (char ch : value) {
                    switch (ch) {
                        case '"': escaped += "\\\""; break;
                        case '\\': escaped += "\\\\"; break;
                        case '\n': escaped += "\\n"; break;
                        case '\r': escaped += "\\r"; break;
                        case '\t': escaped += "\\t"; break;
                        default:
                            if (static_cast<unsigned char>(ch) < 0x20) {
                                escaped += ' ';
                            } else {
                                escaped += ch;
                            }
                    }
                }
                return escaped;
            };

            rows << "{\"pid\":" << pid
                 << ",\"process_name\":\"" << escapeJson(name)
                 << "\",\"computer_name\":\"" << escapeJson(computer)
                 << "\"}";

            ++count;
        }

        rows << "]";

        stmt->closeResultSet(rs);
        rs = nullptr;
        conn->terminateStatement(stmt);
        stmt = nullptr;

        json << count << "," << rows.str() << "}";
        return json.str();
    }
    catch (SQLException& e) {
        std::cerr << "Process query failed: "
                  << e.getMessage() << '\n';

        if (stmt != nullptr) {
            try {
                if (rs != nullptr) {
                    stmt->closeResultSet(rs);
                }
            } catch (...) {}

            try {
                conn->terminateStatement(stmt);
            } catch (...) {}
        }

        return R"({"error":"Failed to query processes"})";
    }
}


std::string OracleDB::getMetricsJson(int computerId) {
    using namespace oracle::occi;

    if (conn == nullptr) {
        return R"({"error":"Database not connected"})";
    }

    Statement* stmt = nullptr;
    ResultSet* rs = nullptr;
    std::ostringstream rows;
    int count = 0;

    auto escapeJson = [](const std::string& value) {
        std::string out;
        for (unsigned char ch : value) {
            switch (ch) {
                case '"': out += "\\\""; break;
                case '\\': out += "\\\\"; break;
                case '\n': out += "\\n"; break;
                case '\r': out += "\\r"; break;
                case '\t': out += "\\t"; break;
                default:
                    if (ch < 0x20) out += ' ';
                    else out += static_cast<char>(ch);
            }
        }
        return out;
    };

    try {
        stmt = conn->createStatement(
            "SELECT pid, process_name, cpu_usage_percent, "
            "memory_usage_mb, recorded_at FROM ("
            " SELECT p.pid, p.process_name, r.cpu_usage_percent, "
            " r.memory_usage_mb, "
            " TO_CHAR(r.recorded_at, 'YYYY-MM-DD HH24:MI:SS') AS recorded_at, "
            " ROW_NUMBER() OVER ("
            "   PARTITION BY p.process_id "
            "   ORDER BY r.recorded_at DESC, r.usage_id DESC"
            " ) AS rn "
            " FROM PROCESS p "
            " JOIN RESOURCE_USAGE r ON r.process_id = p.process_id "
            " WHERE p.computer_id = :1"
            ") WHERE rn = 1 ORDER BY pid"
        );

        stmt->setInt(1, computerId);
        rs = stmt->executeQuery();

        while (rs->next()) {
            if (count > 0) rows << ",";

            int pid = rs->getInt(1);
            std::string name = rs->getString(2);
            double cpu = rs->getDouble(3);
            double memory = rs->getDouble(4);
            std::string recordedAt = rs->getString(5);

            rows << "{\"pid\":" << pid
                 << ",\"process_name\":\"" << escapeJson(name) << "\""
                 << ",\"cpu_usage_percent\":" << cpu
                 << ",\"memory_usage_mb\":" << memory
                 << ",\"recorded_at\":\"" << escapeJson(recordedAt) << "\""
                 << "}";

            ++count;
        }

        stmt->closeResultSet(rs);
        rs = nullptr;
        conn->terminateStatement(stmt);
        stmt = nullptr;

        std::ostringstream result;
        result << "{\"count\":" << count
               << ",\"metrics\":[" << rows.str() << "]}";
        return result.str();
    }
    catch (SQLException& e) {
        std::cerr << "Metrics query failed: "
                  << e.getMessage() << '\n';

        if (stmt != nullptr) {
            if (rs != nullptr) {
                try { stmt->closeResultSet(rs); } catch (...) {}
            }
            try { conn->terminateStatement(stmt); } catch (...) {}
        }

        return R"({"error":"Failed to query metrics"})";
    }
}

// Persist a complete scheduling run and its per-process results.
int OracleDB::saveSchedulingRun(
    const std::string& algorithm,
    int timeQuantum,
    const std::vector<Process>& processes,
    const SchedulingReport& report
) {
    using namespace oracle::occi;

    if (conn == nullptr ||
        (algorithm != "FCFS" && algorithm != "ROUND_ROBIN") ||
        processes.size() != report.results.size() ||
        (algorithm == "ROUND_ROBIN" && timeQuantum <= 0)) {
        return -1;
    }

    Statement* runStmt = nullptr;
    Statement* resultStmt = nullptr;
    int runId = -1;

    try {
        runStmt = conn->createStatement(
            "INSERT INTO SCHEDULING_RUN "
            "(algorithm, time_quantum, process_count, "
            "average_waiting_time, average_turnaround_time) "
            "VALUES (:1, :2, :3, :4, :5) "
            "RETURNING run_id INTO :6"
        );

        runStmt->setString(1, algorithm);

        if (algorithm == "FCFS")
            runStmt->setNull(2, oracle::occi::OCCIINT);
        else
            runStmt->setInt(2, timeQuantum);

        runStmt->setInt(3, static_cast<int>(processes.size()));
        runStmt->setDouble(4, report.averageWaitingTime);
        runStmt->setDouble(5, report.averageTurnaroundTime);
        runStmt->registerOutParam(6, oracle::occi::OCCIINT);

        runStmt->executeUpdate();
        runId = runStmt->getInt(6);

        conn->terminateStatement(runStmt);
        runStmt = nullptr;

        resultStmt = conn->createStatement(
            "INSERT INTO SCHEDULING_RESULT "
            "(run_id, pid, arrival_time, burst_time, completion_time, "
            "turnaround_time, waiting_time, response_time) "
            "VALUES (:1, :2, :3, :4, :5, :6, :7, :8)"
        );

        for (const auto& result : report.results) {
            auto it = std::find_if(
                processes.begin(), processes.end(),
                [&](const Process& p) { return p.pid == result.pid; }
            );

            if (it == processes.end())
                throw std::runtime_error("Scheduling result PID not found");

            resultStmt->setInt(1, runId);
            resultStmt->setInt(2, result.pid);
            resultStmt->setInt(3, it->arrivalTime);
            resultStmt->setInt(4, it->burstTime);
            resultStmt->setInt(5, result.completionTime);
            resultStmt->setInt(6, result.turnaroundTime);
            resultStmt->setInt(7, result.waitingTime);
            resultStmt->setInt(8, result.responseTime);
            resultStmt->executeUpdate();
        }

        conn->terminateStatement(resultStmt);
        resultStmt = nullptr;

        conn->commit();
        return runId;
    }
    catch (SQLException& e) {
        std::cerr << "Saving scheduling run failed: "
                  << e.getMessage() << '\n';
    }
    catch (const std::exception& e) {
        std::cerr << "Saving scheduling run failed: "
                  << e.what() << '\n';
    }

    if (runStmt != nullptr) {
        try { conn->terminateStatement(runStmt); } catch (...) {}
    }

    if (resultStmt != nullptr) {
        try { conn->terminateStatement(resultStmt); } catch (...) {}
    }

    try { conn->rollback(); } catch (...) {}
    return -1;
}


// Persist a complete Banker's Algorithm analysis in Oracle.
int OracleDB::saveDeadlockRun(
    const std::vector<int>& available,
    const std::vector<std::vector<int>>& allocation,
    const std::vector<std::vector<int>>& maximum,
    const DeadlockResult& result
) {
    using namespace oracle::occi;

    const int processCount = static_cast<int>(allocation.size());
    const int resourceCount = static_cast<int>(available.size());

    if (conn == nullptr || processCount == 0 || resourceCount == 0 ||
        maximum.size() != allocation.size() ||
        result.need.size() != allocation.size()) {
        return -1;
    }

    for (int i = 0; i < processCount; ++i) {
        if (allocation[i].size() != available.size() ||
            maximum[i].size() != available.size() ||
            result.need[i].size() != available.size()) {
            return -1;
        }

        for (int j = 0; j < resourceCount; ++j) {
            if (available[j] < 0 || allocation[i][j] < 0 ||
                maximum[i][j] < allocation[i][j] ||
                result.need[i][j] != maximum[i][j] - allocation[i][j]) {
                return -1;
            }
        }
    }

    for (int resource : available) {
        if (resource < 0) return -1;
    }

    if (result.safe &&
        result.safeSequence.size() != allocation.size()) {
        return -1;
    }

    std::string sequence;
    if (result.safe) {
        for (size_t i = 0; i < result.safeSequence.size(); ++i) {
            int processIndex = result.safeSequence[i];
            if (processIndex < 0 || processIndex >= processCount) {
                return -1;
            }
            if (i > 0) sequence += " -> ";
            sequence += "P" + std::to_string(processIndex);
        }
    }

    Statement* runStmt = nullptr;
    Statement* resourceStmt = nullptr;
    Statement* processStmt = nullptr;
    Statement* matrixStmt = nullptr;
    int runId = -1;

    try {
        runStmt = conn->createStatement(
            "INSERT INTO DEADLOCK_RUN "
            "(process_count, resource_count, is_safe, safe_sequence) "
            "VALUES (:1, :2, :3, :4) RETURNING run_id INTO :5"
        );
        runStmt->setInt(1, processCount);
        runStmt->setInt(2, resourceCount);
        runStmt->setString(3, result.safe ? "Y" : "N");

        if (result.safe) {
            runStmt->setString(4, sequence);
        } else {
            runStmt->setNull(4, OCCISTRING);
        }

        runStmt->registerOutParam(5, OCCIINT);
        runStmt->executeUpdate();
        runId = runStmt->getInt(5);
        conn->terminateStatement(runStmt);
        runStmt = nullptr;

        resourceStmt = conn->createStatement(
            "INSERT INTO DEADLOCK_RESOURCE "
            "(run_id, resource_index, available_instances) "
            "VALUES (:1, :2, :3)"
        );

        for (int j = 0; j < resourceCount; ++j) {
            resourceStmt->setInt(1, runId);
            resourceStmt->setInt(2, j);
            resourceStmt->setInt(3, available[j]);
            resourceStmt->executeUpdate();
        }
        conn->terminateStatement(resourceStmt);
        resourceStmt = nullptr;

        processStmt = conn->createStatement(
            "INSERT INTO DEADLOCK_PROCESS (run_id, process_index) "
            "VALUES (:1, :2)"
        );

        for (int i = 0; i < processCount; ++i) {
            processStmt->setInt(1, runId);
            processStmt->setInt(2, i);
            processStmt->executeUpdate();
        }
        conn->terminateStatement(processStmt);
        processStmt = nullptr;

        matrixStmt = conn->createStatement(
            "INSERT INTO DEADLOCK_PROCESS_RESOURCE "
            "(run_id, process_index, resource_index, allocation, "
            "maximum_demand, need) VALUES (:1, :2, :3, :4, :5, :6)"
        );

        for (int i = 0; i < processCount; ++i) {
            for (int j = 0; j < resourceCount; ++j) {
                matrixStmt->setInt(1, runId);
                matrixStmt->setInt(2, i);
                matrixStmt->setInt(3, j);
                matrixStmt->setInt(4, allocation[i][j]);
                matrixStmt->setInt(5, maximum[i][j]);
                matrixStmt->setInt(6, result.need[i][j]);
                matrixStmt->executeUpdate();
            }
        }
        conn->terminateStatement(matrixStmt);
        matrixStmt = nullptr;

        conn->commit();
        return runId;
    } catch (SQLException& e) {
        std::cerr << "Saving deadlock run failed: "
                  << e.getMessage() << '\n';
    } catch (const std::exception& e) {
        std::cerr << "Saving deadlock run failed: "
                  << e.what() << '\n';
    }

    if (runStmt) {
        try { conn->terminateStatement(runStmt); } catch (...) {}
    }
    if (resourceStmt) {
        try { conn->terminateStatement(resourceStmt); } catch (...) {}
    }
    if (processStmt) {
        try { conn->terminateStatement(processStmt); } catch (...) {}
    }
    if (matrixStmt) {
        try { conn->terminateStatement(matrixStmt); } catch (...) {}
    }

    try { conn->rollback(); } catch (...) {}
    return -1;
}


// Persist a contiguous file-allocation simulation and all its attempts.
int OracleDB::saveFileAllocationRun(
    int totalBlocks,
    int freeBlocks,
    const std::vector<AllocationResult>& attempts,
    int filesAllocated
) {
    using namespace oracle::occi;

    if (conn == nullptr || totalBlocks <= 0 ||
        freeBlocks < 0 || freeBlocks > totalBlocks ||
        filesAllocated < 0 ||
        filesAllocated > static_cast<int>(attempts.size())) {
        return -1;
    }

    Statement* runStmt = nullptr;
    Statement* resultStmt = nullptr;
    int runId = -1;

    try {
        runStmt = conn->createStatement(
            "INSERT INTO FILE_ALLOCATION_RUN "
            "(total_blocks, free_blocks, files_requested, files_allocated) "
            "VALUES (:1, :2, :3, :4) RETURNING run_id INTO :5"
        );

        runStmt->setInt(1, totalBlocks);
        runStmt->setInt(2, freeBlocks);
        runStmt->setInt(3, static_cast<int>(attempts.size()));
        runStmt->setInt(4, filesAllocated);
        runStmt->registerOutParam(5, OCCIINT);
        runStmt->executeUpdate();
        runId = runStmt->getInt(5);

        conn->terminateStatement(runStmt);
        runStmt = nullptr;

        resultStmt = conn->createStatement(
            "INSERT INTO FILE_ALLOCATION_RESULT "
            "(run_id, file_name, requested_blocks, allocation_success, "
            "start_block, end_block, message) "
            "VALUES (:1, :2, :3, :4, :5, :6, :7)"
        );

        for (const auto& attempt : attempts) {
            if (attempt.file.fileName.empty() ||
                attempt.file.fileName.size() > 100 ||
                attempt.file.blockCount <= 0) {
                throw std::invalid_argument("Invalid file allocation record");
            }

            resultStmt->setInt(1, runId);
            resultStmt->setString(2, attempt.file.fileName);
            resultStmt->setInt(3, attempt.file.blockCount);
            resultStmt->setString(4, attempt.success ? "Y" : "N");

            if (attempt.success) {
                if (attempt.file.startBlock < 0 ||
                    attempt.file.startBlock + attempt.file.blockCount > totalBlocks) {
                    throw std::invalid_argument("Allocated block range is invalid");
                }
                resultStmt->setInt(5, attempt.file.startBlock);
                resultStmt->setInt(
                    6, attempt.file.startBlock + attempt.file.blockCount - 1
                );
            } else {
                resultStmt->setNull(5, OCCIINT);
                resultStmt->setNull(6, OCCIINT);
            }

            std::string message = attempt.message;
            if (message.size() > 250) message.resize(250);
            resultStmt->setString(7, message);
            resultStmt->executeUpdate();
        }

        conn->terminateStatement(resultStmt);
        resultStmt = nullptr;

        conn->commit();
        return runId;
    } catch (SQLException& e) {
        std::cerr << "Saving file allocation run failed: "
                  << e.getMessage() << '\n';
    } catch (const std::exception& e) {
        std::cerr << "Saving file allocation run failed: "
                  << e.what() << '\n';
    }

    if (runStmt) {
        try { conn->terminateStatement(runStmt); } catch (...) {}
    }
    if (resultStmt) {
        try { conn->terminateStatement(resultStmt); } catch (...) {}
    }

    try { conn->rollback(); } catch (...) {}
    return -1;
}
