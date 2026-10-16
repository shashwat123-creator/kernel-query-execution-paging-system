#include "bankers.h"
#include <iostream>
#include <cstddef>
#include <vector>

int main() {
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

    try {
        DeadlockResult result =
            checkSafeState(available, allocation, maximum);

        std::cout << result.message << '\n';

        if (result.safe) {
            std::cout << "Safe sequence: ";
            for (size_t i = 0; i < result.safeSequence.size(); ++i) {
                std::cout << "P" << result.safeSequence[i];
                if (i + 1 < result.safeSequence.size()) {
                    std::cout << " -> ";
                }
            }
            std::cout << '\n';
        }

        std::cout << "\nNeed matrix:\n";
        for (size_t i = 0; i < result.need.size(); ++i) {
            std::cout << "P" << i << ": ";
            for (int value : result.need[i]) {
                std::cout << value << ' ';
            }
            std::cout << '\n';
        }
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << '\n';
        return 1;
    }

    return 0;
}
