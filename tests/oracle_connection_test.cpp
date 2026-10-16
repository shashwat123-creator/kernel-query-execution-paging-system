#include <occi.h>
#include <iostream>
#include <unistd.h>

using namespace std;
using namespace oracle::occi;

int main() {
    Environment* env = nullptr;
    Connection* conn = nullptr;
    char* password = getpass("Enter Oracle password: ");

    if (password == nullptr) {
        cerr << "Could not read password.\n";
        return 1;
    }

    try {
        env = Environment::createEnvironment();

        conn = env->createConnection(
            "KERNEL_APP",
            password,
            "//localhost:1521/FREEPDB1"
        );

        Statement* stmt = conn->createStatement("SELECT USER FROM dual");
        ResultSet* rs = stmt->executeQuery();

        if (rs->next()) {
            cout << "Oracle connection successful!\n";
            cout << "Connected user: " << rs->getString(1) << '\n';
        }

        stmt->closeResultSet(rs);
        conn->terminateStatement(stmt);
        env->terminateConnection(conn);
        Environment::terminateEnvironment(env);
    }
    catch (SQLException& e) {
        cerr << "Oracle error: " << e.getMessage() << '\n';

        if (conn != nullptr && env != nullptr)
            env->terminateConnection(conn);

        if (env != nullptr)
            Environment::terminateEnvironment(env);

        return 1;
    }

    return 0;
}
