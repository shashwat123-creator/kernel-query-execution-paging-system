#include "contiguous.h"
#include "../../database/oracle_db.h"

#include <cstdlib>
#include <iostream>
#include <vector>

int main() {
    const char* password = std::getenv("ORACLE_APP_PASSWORD");
    if (password == nullptr) {
        std::cerr << "Set ORACLE_APP_PASSWORD first.\n";
        return 1;
    }

    ContiguousAllocator disk(10);
    std::vector<AllocationResult> attempts;

    attempts.push_back(disk.allocate("FileA", 3));
    attempts.push_back(disk.allocate("FileB", 3));
    attempts.push_back(disk.allocate("FileC", 3));

    disk.deallocate("FileA");
    disk.deallocate("FileC");

    attempts.push_back(disk.allocate("FileD", 5));

    int allocated = 0;
    for (const auto& attempt : attempts) {
        if (attempt.success) ++allocated;
        std::cout << attempt.file.fileName << ": "
                  << attempt.message << '\n';
    }

    OracleDB db;
    if (!db.connect("KERNEL_APP", password,
                    "//localhost:1521/FREEPDB1")) {
        std::cerr << "Oracle connection failed.\n";
        return 1;
    }

    int runId = db.saveFileAllocationRun(
        disk.totalBlocks(),
        disk.freeBlocks(),
        attempts,
        allocated
    );

    db.disconnect();

    if (runId < 0) {
        std::cerr << "Failed to save file-allocation run.\n";
        return 1;
    }

    std::cout << "Free blocks: " << disk.freeBlocks() << '\n';
    std::cout << "Files allocated: " << allocated << '\n';
    std::cout << "Saved Oracle run ID: " << runId << '\n';

    return 0;
}
