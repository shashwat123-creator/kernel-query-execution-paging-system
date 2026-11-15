#include "demand_paging.h"

#include <cassert>
#include <iostream>
#include <vector>

void displayReport(const PagingReport& report) {
    std::cout << "\n========== " << report.algorithm
              << " PAGE REPLACEMENT ==========\n";

    std::cout << "Page\tFrames\t\tStatus\t\tReplaced\n";

    for (const auto& step : report.steps) {
        std::cout << step.pageNumber << "\t";

        for (int frame : step.frames) {
            if (frame == -1)
                std::cout << "- ";
            else
                std::cout << frame << " ";
        }

        std::cout << "\t\t";

        if (step.pageFault)
            std::cout << "PAGE FAULT";
        else
            std::cout << "PAGE HIT";

        std::cout << "\t";

        if (step.replacedPage == -1)
            std::cout << "-";
        else
            std::cout << step.replacedPage;

        std::cout << '\n';
    }

    std::cout << "\nTotal References: " << report.totalReferences;
    std::cout << "\nPage Hits: " << report.pageHits;
    std::cout << "\nPage Faults: " << report.pageFaults;
    std::cout << "\nPage Fault Ratio: "
              << report.pageFaultRatio * 100 << "%\n";
}

int main() {
    std::vector<int> pages = {
        7, 0, 1, 2, 0, 3, 0, 4, 2, 3, 0, 3, 2
    };

    PagingReport fifo = simulatePaging(
        pages, 3, ReplacementAlgorithm::FIFO
    );

    PagingReport lru = simulatePaging(
        pages, 3, ReplacementAlgorithm::LRU
    );

    PagingReport optimal = simulatePaging(
        pages, 3, ReplacementAlgorithm::OPTIMAL
    );

    assert(fifo.pageFaults == 10);
    assert(lru.pageFaults == 9);
    assert(optimal.pageFaults == 7);

    assert(fifo.pageHits + fifo.pageFaults == fifo.totalReferences);
    assert(lru.pageHits + lru.pageFaults == lru.totalReferences);
    assert(optimal.pageHits + optimal.pageFaults == optimal.totalReferences);

    displayReport(fifo);
    displayReport(lru);
    displayReport(optimal);

    std::cout << "\nAll demand paging tests passed!\n";

    return 0;
}
