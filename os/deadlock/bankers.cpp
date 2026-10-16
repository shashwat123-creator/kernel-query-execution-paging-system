#include "bankers.h"

#include <stdexcept>

DeadlockResult checkSafeState(
    const std::vector<int>& available,
    const std::vector<std::vector<int>>& allocation,
    const std::vector<std::vector<int>>& maximum
) {
    const int processCount = static_cast<int>(allocation.size());
    const int resourceCount = static_cast<int>(available.size());

    DeadlockResult result{};
    result.safe = false;

    if (processCount == 0 || resourceCount == 0 ||
        maximum.size() != allocation.size()) {
        throw std::invalid_argument("Invalid process or resource dimensions");
    }

    for (int resource : available) {
        if (resource < 0) {
            throw std::invalid_argument("Available resources cannot be negative");
        }
    }

    result.need.resize(processCount, std::vector<int>(resourceCount));

    for (int i = 0; i < processCount; ++i) {
        if (allocation[i].size() != available.size() ||
            maximum[i].size() != available.size()) {
            throw std::invalid_argument("Matrix dimensions do not match");
        }

        for (int j = 0; j < resourceCount; ++j) {
            if (allocation[i][j] < 0 || maximum[i][j] < 0 ||
                maximum[i][j] < allocation[i][j]) {
                throw std::invalid_argument("Invalid allocation or maximum demand");
            }

            result.need[i][j] = maximum[i][j] - allocation[i][j];
        }
    }

    std::vector<int> work = available;
    std::vector<bool> finished(processCount, false);

    while (static_cast<int>(result.safeSequence.size()) < processCount) {
        bool found = false;

        for (int i = 0; i < processCount; ++i) {
            if (finished[i]) {
                continue;
            }

            bool canFinish = true;

            for (int j = 0; j < resourceCount; ++j) {
                if (result.need[i][j] > work[j]) {
                    canFinish = false;
                    break;
                }
            }

            if (canFinish) {
                for (int j = 0; j < resourceCount; ++j) {
                    work[j] += allocation[i][j];
                }

                finished[i] = true;
                result.safeSequence.push_back(i);
                found = true;
            }
        }

        if (!found) {
            result.safe = false;
            result.message = "Unsafe state: no safe sequence exists.";
            return result;
        }
    }

    result.safe = true;
    result.message = "Safe state: a safe sequence exists.";
    return result;
}
