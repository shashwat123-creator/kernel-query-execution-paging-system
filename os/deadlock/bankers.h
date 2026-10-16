#ifndef BANKERS_H
#define BANKERS_H

#include <string>
#include <vector>

struct DeadlockResult {
    bool safe;
    std::vector<int> safeSequence;
    std::vector<std::vector<int>> need;
    std::string message;
};

DeadlockResult checkSafeState(
    const std::vector<int>& available,
    const std::vector<std::vector<int>>& allocation,
    const std::vector<std::vector<int>>& maximum
);

#endif
