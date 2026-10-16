#include "bankers.h"
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

    std::vector<int> available = {3, 3, 2};

    std::vector<std::vector<int>> allocation = {
        {0, 1, 0},
        {2, 0, 0},
        {3, 0, 2},
        {2, 1, 1},
        {0, 0, 2}
    };

    std::vector<std::vector<int>> maximum = {
        {7, 5, 3},
        {3, 2, 2},
        {9, 0, 2},
        {2, 2, 2},
        {4, 3, 3}
    };

    DeadlockResult result =
        checkSafeState(available, allocation, maximum);

    OracleDB db;
    if (!db.connect("KERNEL_APP", password,
                    "//localhost:1521/FREEPDB1")) {
        std::cerr << "Oracle connection failed.\n";
        return 1;
    }

    int runId = db.saveDeadlockRun(
        available, allocation, maximum, result
    );

    db.disconnect();

    if (runId < 0) {
        std::cerr << "Failed to save deadlock analysis.\n";
        return 1;
    }

    std::cout << "Safety result: "
              << (result.safe ? "SAFE" : "UNSAFE") << '\n';
    std::cout << "Saved Oracle run ID: " << runId << '\n';

    return 0;
}
