#ifndef DEMAND_PAGING_H
#define DEMAND_PAGING_H

#include <string>
#include <vector>

enum class ReplacementAlgorithm {
    FIFO,
    LRU,
    OPTIMAL
};

struct PageStep {
    int pageNumber;
    std::vector<int> frames;
    bool pageFault;
    int replacedPage;
};

struct PagingReport {
    std::string algorithm;
    int totalReferences = 0;
    int pageHits = 0;
    int pageFaults = 0;
    double pageFaultRatio = 0.0;
    std::vector<PageStep> steps;
};

PagingReport simulatePaging(
    const std::vector<int>& referenceString,
    int frameCount,
    ReplacementAlgorithm algorithm
);

#endif
