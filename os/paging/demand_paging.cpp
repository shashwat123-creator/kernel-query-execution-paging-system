#include "demand_paging.h"

#include <stdexcept>

static std::string algorithmName(ReplacementAlgorithm algorithm) {
    switch (algorithm) {
        case ReplacementAlgorithm::FIFO: return "FIFO";
        case ReplacementAlgorithm::LRU: return "LRU";
        case ReplacementAlgorithm::OPTIMAL: return "OPTIMAL";
    }

    throw std::invalid_argument("Unknown replacement algorithm");
}

PagingReport simulatePaging(
    const std::vector<int>& referenceString,
    int frameCount,
    ReplacementAlgorithm algorithm
) {
    if (frameCount <= 0)
        throw std::invalid_argument("Frame count must be positive");

    for (int page : referenceString) {
        if (page < 0)
            throw std::invalid_argument("Page numbers cannot be negative");
    }

    PagingReport report;
    report.algorithm = algorithmName(algorithm);
    report.totalReferences = static_cast<int>(referenceString.size());

    std::vector<int> frames(frameCount, -1);
    std::vector<int> lastUsed(frameCount, -1);

    int fifoPointer = 0;

    for (int i = 0; i < report.totalReferences; ++i) {
        int page = referenceString[i];
        int frameIndex = -1;

        // Check for a page hit.
        for (int j = 0; j < frameCount; ++j) {
            if (frames[j] == page) {
                frameIndex = j;
                break;
            }
        }

        bool pageFault = (frameIndex == -1);
        int replacedPage = -1;

        if (!pageFault) {
            ++report.pageHits;
            lastUsed[frameIndex] = i;
        } else {
            ++report.pageFaults;

            // First use an empty frame.
            for (int j = 0; j < frameCount; ++j) {
                if (frames[j] == -1) {
                    frameIndex = j;
                    break;
                }
            }

            // If no empty frame exists, replace a page.
            if (frameIndex == -1) {
                if (algorithm == ReplacementAlgorithm::FIFO) {
                    frameIndex = fifoPointer;
                    fifoPointer = (fifoPointer + 1) % frameCount;
                }
                else if (algorithm == ReplacementAlgorithm::LRU) {
                    frameIndex = 0;

                    for (int j = 1; j < frameCount; ++j) {
                        if (lastUsed[j] < lastUsed[frameIndex])
                            frameIndex = j;
                    }
                }
                else {
                    int farthest = -1;
                    frameIndex = 0;

                    for (int j = 0; j < frameCount; ++j) {
                        int nextUse = report.totalReferences + 1;

                        for (int k = i + 1; k < report.totalReferences; ++k) {
                            if (referenceString[k] == frames[j]) {
                                nextUse = k;
                                break;
                            }
                        }

                        if (nextUse > farthest) {
                            farthest = nextUse;
                            frameIndex = j;
                        }
                    }
                }

                replacedPage = frames[frameIndex];
            }

            frames[frameIndex] = page;
            lastUsed[frameIndex] = i;
        }

        report.steps.push_back({
            page,
            frames,
            pageFault,
            replacedPage
        });
    }

    if (report.totalReferences > 0) {
        report.pageFaultRatio =
            static_cast<double>(report.pageFaults) /
            report.totalReferences;
    }

    return report;
}
