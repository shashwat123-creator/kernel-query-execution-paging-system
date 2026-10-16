#include "httplib.h"
#include "../database/oracle_db.h"
#include "../os/deadlock/bankers.h"
#include "../os/file-allocation/contiguous.h"

#include <cstdlib>
#include <iostream>
#include <mutex>
#include <string>
#include <sstream>
#include <vector>

int main() {
    httplib::Server server;
    server.set_default_headers({
    {"Access-Control-Allow-Origin", "http://localhost:5500"},
    {"Access-Control-Allow-Methods", "GET, POST, OPTIONS"},
    {"Access-Control-Allow-Headers", "Content-Type"}
    });
    OracleDB db;
    std::mutex dbMutex;

    const char* password = std::getenv("ORACLE_APP_PASSWORD");

    if (password == nullptr || *password == '\0') {
        std::cerr << "Set ORACLE_APP_PASSWORD before starting the server.\n";
        return 1;
    }

    if (!db.connect("KERNEL_APP", password, "//localhost:1521/FREEPDB1")) {
        std::cerr << "Oracle connection failed.\n";
        return 1;
    }

    server.Get("/api/health",
        [](const httplib::Request&, httplib::Response& res) {
            res.set_content(
                R"({"status":"ok","backend":"C++"})",
                "application/json"
            );
        });

    server.Get("/api/processes",
        [&db, &dbMutex](const httplib::Request& req, httplib::Response& res) {
            int computerId = 1;

            if (req.has_param("computer_id")) {
                try {
                    computerId = std::stoi(
                        req.get_param_value("computer_id")
                    );
                } catch (...) {
                    res.status = 400;
                    res.set_content(
                        R"({"error":"Invalid computer_id"})",
                        "application/json"
                    );
                    return;
                }
            }

            if (computerId <= 0) {
                res.status = 400;
                res.set_content(
                    R"({"error":"computer_id must be positive"})",
                    "application/json"
                );
                return;
            }

            res.set_header("Cache-Control", "no-store");
            res.set_content(
                ([&]() { std::lock_guard<std::mutex> lock(dbMutex); return db.getProcessesJson(computerId); })(),
                "application/json"
            );
        });


    server.Get("/api/metrics",
        [&db, &dbMutex](const httplib::Request& req, httplib::Response& res) {
            int computerId = 1;

            if (req.has_param("computer_id")) {
                try {
                    computerId = std::stoi(
                        req.get_param_value("computer_id")
                    );
                } catch (...) {
                    res.status = 400;
                    res.set_content(
                        R"({"error":"Invalid computer_id"})",
                        "application/json"
                    );
                    return;
                }
            }

            if (computerId <= 0) {
                res.status = 400;
                res.set_content(
                    R"({"error":"computer_id must be positive"})",
                    "application/json"
                );
                return;
            }

            res.set_header("Cache-Control", "no-store");
            res.set_content(
                ([&]() { std::lock_guard<std::mutex> lock(dbMutex); return db.getMetricsJson(computerId); })(),
                "application/json"
            );
        });



    server.Post("/api/deadlock/check",
        [&db, &dbMutex](const httplib::Request& req, httplib::Response& res) {
            auto error = [&](int status, const std::string& message) {
                res.status = status;
                res.set_content("{\"error\":\"" + message + "\"}",
                                "application/json");
            };

            if (!req.has_param("available") ||
                !req.has_param("allocation") ||
                !req.has_param("maximum")) {
                error(400, "Provide available, allocation, and maximum");
                return;
            }

            auto parseRow = [](const std::string& text) {
                std::vector<int> row;
                std::istringstream input(text);
                std::string field;

                while (std::getline(input, field, ',')) {
                    if (field.empty())
                        throw std::invalid_argument("Empty resource value");

                    std::size_t used = 0;
                    int value = std::stoi(field, &used);
                    if (used != field.size() || value < 0)
                        throw std::invalid_argument("Invalid resource value");

                    row.push_back(value);
                }

                if (row.empty())
                    throw std::invalid_argument("Empty resource row");

                return row;
            };

            auto parseMatrix = [&](const std::string& text) {
                std::vector<std::vector<int>> matrix;
                std::istringstream input(text);
                std::string line;

                while (std::getline(input, line, ';')) {
                    if (line.empty())
                        throw std::invalid_argument("Empty matrix row");
                    matrix.push_back(parseRow(line));
                }

                if (matrix.empty())
                    throw std::invalid_argument("Empty matrix");

                return matrix;
            };

            try {
                std::vector<int> available =
                    parseRow(req.get_param_value("available"));
                auto allocation =
                    parseMatrix(req.get_param_value("allocation"));
                auto maximum =
                    parseMatrix(req.get_param_value("maximum"));

                if (allocation.size() > 100 ||
                    available.size() > 20 ||
                    allocation.size() != maximum.size()) {
                    error(400, "Invalid process or resource dimensions");
                    return;
                }

                for (std::size_t i = 0; i < allocation.size(); ++i) {
                    if (allocation[i].size() != available.size() ||
                        maximum[i].size() != available.size()) {
                        error(400, "Matrix dimensions do not match");
                        return;
                    }

                    for (std::size_t j = 0; j < available.size(); ++j) {
                        if (maximum[i][j] < allocation[i][j]) {
                            error(400, "Maximum demand cannot be less than allocation");
                            return;
                        }
                    }
                }

                DeadlockResult result =
                    checkSafeState(available, allocation, maximum);

                int runId;
                {
                    std::lock_guard<std::mutex> lock(dbMutex);
                    runId = db.saveDeadlockRun(
                        available, allocation, maximum, result);
                }

                if (runId < 0) {
                    error(500, "Could not save deadlock analysis to Oracle");
                    return;
                }

                std::ostringstream json;
                json << "{\"run_id\":" << runId
                     << ",\"safe\":" << (result.safe ? "true" : "false")
                     << ",\"message\":\"" << result.message << "\""
                     << ",\"safe_sequence\":[";

                for (std::size_t i = 0; i < result.safeSequence.size(); ++i) {
                    if (i) json << ",";
                    json << result.safeSequence[i];
                }

                json << "],\"need\":[";
                for (std::size_t i = 0; i < result.need.size(); ++i) {
                    if (i) json << ",";
                    json << "[";
                    for (std::size_t j = 0; j < result.need[i].size(); ++j) {
                        if (j) json << ",";
                        json << result.need[i][j];
                    }
                    json << "]";
                }
                json << "]}";

                res.set_header("Cache-Control", "no-store");
                res.set_content(json.str(), "application/json");
            } catch (const std::exception&) {
                error(400, "Invalid resource input");
            }
        });

    server.Post("/api/scheduling/run",
        [&db, &dbMutex](const httplib::Request& req, httplib::Response& res) {
            auto error = [&](int status, const std::string& message) {
                res.status = status;
                res.set_content("{\"error\":\"" + message + "\"}",
                                "application/json");
            };

            if (!req.has_param("algorithm") || !req.has_param("processes")) {
                error(400, "Provide algorithm and processes");
                return;
            }

            std::string algorithm = req.get_param_value("algorithm");
            if (algorithm != "FCFS" && algorithm != "ROUND_ROBIN") {
                error(400, "Algorithm must be FCFS or ROUND_ROBIN");
                return;
            }

            int quantum = 0;
            if (algorithm == "ROUND_ROBIN") {
                if (!req.has_param("quantum")) {
                    error(400, "Round Robin requires quantum");
                    return;
                }
                try {
                    quantum = std::stoi(req.get_param_value("quantum"));
                } catch (...) {
                    error(400, "Invalid quantum");
                    return;
                }
                if (quantum <= 0) {
                    error(400, "Quantum must be positive");
                    return;
                }
            }

            std::vector<Process> processes;
            std::istringstream input(req.get_param_value("processes"));
            std::string item;

            try {
                while (std::getline(input, item, ';')) {
                    if (item.empty()) continue;
                    std::istringstream fields(item);
                    std::string pidText, arrivalText, burstText, extra;

                    if (!std::getline(fields, pidText, ',') ||
                        !std::getline(fields, arrivalText, ',') ||
                        !std::getline(fields, burstText, ',') ||
                        std::getline(fields, extra, ',')) {
                        error(400, "Each process must be pid,arrival,burst");
                        return;
                    }

                    std::size_t used = 0;
                    int pid = std::stoi(pidText, &used);
                    if (used != pidText.size()) throw std::invalid_argument("pid");
                    int arrival = std::stoi(arrivalText, &used);
                    if (used != arrivalText.size()) throw std::invalid_argument("arrival");
                    int burst = std::stoi(burstText, &used);
                    if (used != burstText.size()) throw std::invalid_argument("burst");

                    processes.push_back({pid, arrival, burst});
                }
            } catch (...) {
                error(400, "Invalid process input");
                return;
            }

            if (processes.empty() || processes.size() > 100) {
                error(400, "Provide between 1 and 100 processes");
                return;
            }

            for (std::size_t i = 0; i < processes.size(); ++i) {
                const auto& p = processes[i];
                if (p.pid <= 0 || p.arrivalTime < 0 || p.burstTime <= 0) {
                    error(400, "PID must be positive; arrival nonnegative; burst positive");
                    return;
                }
                for (std::size_t j = 0; j < i; ++j) {
                    if (processes[j].pid == p.pid) {
                        error(400, "Process PIDs must be unique");
                        return;
                    }
                }
            }

            try {
                SchedulingReport report =
                    algorithm == "FCFS" ? fcfs(processes)
                                         : roundRobin(processes, quantum);

                int runId;
                {
                    std::lock_guard<std::mutex> lock(dbMutex);
                    runId = db.saveSchedulingRun(
                        algorithm, quantum, processes, report);
                }

                if (runId < 0) {
                    error(500, "Could not save scheduling run to Oracle");
                    return;
                }

                std::ostringstream json;
                json << "{\"run_id\":" << runId
                     << ",\"algorithm\":\"" << algorithm << "\""
                     << ",\"average_waiting_time\":" << report.averageWaitingTime
                     << ",\"average_turnaround_time\":" << report.averageTurnaroundTime
                     << ",\"results\":[";

                for (std::size_t i = 0; i < report.results.size(); ++i) {
                    const auto& r = report.results[i];
                    if (i) json << ",";
                    json << "{\"pid\":" << r.pid
                         << ",\"completion_time\":" << r.completionTime
                         << ",\"turnaround_time\":" << r.turnaroundTime
                         << ",\"waiting_time\":" << r.waitingTime
                         << ",\"response_time\":" << r.responseTime << "}";
                }

                json << "],\"gantt\":[";
                for (std::size_t i = 0; i < report.gantt.size(); ++i) {
                    const auto& g = report.gantt[i];
                    if (i) json << ",";
                    json << "{\"pid\":" << g.pid
                         << ",\"start_time\":" << g.startTime
                         << ",\"end_time\":" << g.endTime << "}";
                }
                json << "]}";

                res.set_content(json.str(), "application/json");
            } catch (const std::exception&) {
                error(400, "Scheduling failed; check process input");
            }
        });


    server.Post("/api/file-allocation/run",
        [&db, &dbMutex](const httplib::Request& req,
                        httplib::Response& res) {
            auto error = [&](int status, const std::string& message) {
                res.status = status;
                res.set_content("{\"error\":\"" + message + "\"}",
                                "application/json");
            };

            if (!req.has_param("total_blocks") || !req.has_param("files")) {
                error(400, "Provide total_blocks and files");
                return;
            }

            int totalBlocks;
            try {
                const std::string value = req.get_param_value("total_blocks");
                std::size_t used = 0;
                totalBlocks = std::stoi(value, &used);
                if (used != value.size()) throw std::invalid_argument("blocks");
            } catch (...) {
                error(400, "Invalid total_blocks");
                return;
            }

            if (totalBlocks < 1 || totalBlocks > 10000) {
                error(400, "Disk size must be between 1 and 10000 blocks");
                return;
            }

            try {
                ContiguousAllocator allocator(totalBlocks);
                std::vector<AllocationResult> attempts;
                std::istringstream input(req.get_param_value("files"));
                std::string item;

                while (std::getline(input, item, ';')) {
                    if (item.empty()) continue;
                    if (attempts.size() >= 100) {
                        error(400, "Maximum 100 file requests allowed");
                        return;
                    }

                    std::istringstream fields(item);
                    std::string name, blocksText, extra;
                    if (!std::getline(fields, name, ',') ||
                        !std::getline(fields, blocksText, ',') ||
                        std::getline(fields, extra, ',')) {
                        error(400, "Each file must be name,blocks");
                        return;
                    }

                    if (name.empty() || name.size() > 100) {
                        error(400, "File names must contain 1 to 100 characters");
                        return;
                    }

                    std::size_t used = 0;
                    int blocks = std::stoi(blocksText, &used);
                    if (used != blocksText.size() ||
                        blocks <= 0 || blocks > totalBlocks) {
                        error(400, "Invalid file block count");
                        return;
                    }

                    attempts.push_back(allocator.allocate(name, blocks));
                }

                if (attempts.empty()) {
                    error(400, "Provide at least one file request");
                    return;
                }

                int allocated = 0;
                for (const auto& a : attempts) {
                    if (a.success) ++allocated;
                }

                int runId;
                {
                    std::lock_guard<std::mutex> lock(dbMutex);
                    runId = db.saveFileAllocationRun(
                        totalBlocks, allocator.freeBlocks(), attempts, allocated);
                }

                if (runId < 0) {
                    error(500, "Could not save file allocation run to Oracle");
                    return;
                }

                std::ostringstream json;
                json << "{\"run_id\":" << runId
                     << ",\"total_blocks\":" << totalBlocks
                     << ",\"free_blocks\":" << allocator.freeBlocks()
                     << ",\"files_allocated\":" << allocated
                     << ",\"files_requested\":" << attempts.size()
                     << ",\"results\":[";

                for (std::size_t i = 0; i < attempts.size(); ++i) {
                    const auto& a = attempts[i];
                    if (i) json << ",";
                    json << "{\"file_name\":\"" << a.file.fileName
                         << "\",\"requested_blocks\":" << a.file.blockCount
                         << ",\"success\":" << (a.success ? "true" : "false")
                         << ",\"start_block\":";
                    if (a.success) json << a.file.startBlock;
                    else json << "null";
                    json << ",\"end_block\":";
                    if (a.success)
                        json << a.file.startBlock + a.file.blockCount - 1;
                    else
                        json << "null";
                    json << ",\"message\":\"" << a.message << "\"}";
                }

                json << "]}";
                res.set_header("Cache-Control", "no-store");
                res.set_content(json.str(), "application/json");
            } catch (const std::exception&) {
                error(400, "Invalid file-allocation input");
            }
        });

    std::cout << "Kernel Query API running at http://localhost:8080\n";

    if (!server.listen("127.0.0.1", 8080)) {
        std::cerr << "Failed to start HTTP server on port 8080.\n";
        return 1;
    }

    return 0;
}
