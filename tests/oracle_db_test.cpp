#include "../database/oracle_db.h"
#include <iostream>
#include <string>
#include <unistd.h>

using namespace std;

int main() {
    char* password = getpass("Enter Oracle password: ");

    if (password == nullptr) {
        cerr << "Could not read password.\n";
        return 1;
    }

    OracleDB db;

    if (!db.connect(
            "KERNEL_APP",
            password,
            "//localhost:1521/FREEPDB1")) {
        return 1;
    }

    // Uses the existing computer_id = 1 and PID = 399.
    int processId = db.getOrCreateProcess(1, 399, "loginwindow");

    if (processId == -1) {
        cerr << "Could not find or insert the process.\n";
        return 1;
    }

    cout << "Process ID in database: " << processId << '\n';

    if (db.insertResourceUsage(processId, 0.0, 12.3)) {
        cout << "Resource usage inserted successfully!\n";
    } else {
        cerr << "Resource usage insertion failed.\n";
        return 1;
    }

    return 0;
}
