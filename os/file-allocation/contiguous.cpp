#include "contiguous.h"

#include <iostream>
#include <stdexcept>

ContiguousAllocator::ContiguousAllocator(int totalBlocks)
    : disk(totalBlocks, -1) {
    if (totalBlocks <= 0) {
        throw std::invalid_argument("Disk must contain at least one block");
    }
}

AllocationResult ContiguousAllocator::allocate(
    const std::string& fileName, int blockCount) {

    if (fileName.empty()) {
        return {false, "File name cannot be empty", {}};
    }

    if (blockCount <= 0 || blockCount > static_cast<int>(disk.size())) {
        return {false, "Invalid block count", {}};
    }

    for (const auto& file : files) {
        if (file.fileName == fileName) {
            return {false, "File already exists", {}};
        }
    }

    int consecutiveFree = 0;
    int start = -1;

    for (int i = 0; i < static_cast<int>(disk.size()); ++i) {
        if (disk[i] == -1) {
            if (consecutiveFree == 0) {
                start = i;
            }

            ++consecutiveFree;

            if (consecutiveFree == blockCount) {
                for (int j = start; j < start + blockCount; ++j) {
                    disk[j] = static_cast<int>(files.size());
                }

                files.push_back({fileName, start, blockCount});

                return {
                    true,
                    "File allocated successfully",
                    {fileName, start, blockCount}
                };
            }
        } else {
            consecutiveFree = 0;
            start = -1;
        }
    }

    return {
        false,
        "No sufficiently large contiguous free region exists",
        {fileName, -1, blockCount}
    };
}

bool ContiguousAllocator::deallocate(const std::string& fileName) {
    for (std::size_t i = 0; i < files.size(); ++i) {
        if (files[i].fileName == fileName) {
            int start = files[i].startBlock;
            int count = files[i].blockCount;

            for (int j = start; j < start + count; ++j) {
                disk[j] = -1;
            }

            files.erase(files.begin() + static_cast<long>(i));

            // Rebuild ownership IDs after removing a file.
            for (int block = 0; block < static_cast<int>(disk.size()); ++block) {
                if (disk[block] == -1) continue;

                for (std::size_t k = 0; k < files.size(); ++k) {
                    const auto& file = files[k];
                    if (block >= file.startBlock &&
                        block < file.startBlock + file.blockCount) {
                        disk[block] = static_cast<int>(k);
                        break;
                    }
                }
            }

            return true;
        }
    }

    return false;
}

void ContiguousAllocator::displayDisk() const {
    std::cout << "Disk blocks:\n";

    for (std::size_t i = 0; i < disk.size(); ++i) {
        std::cout << "[" << i << ":";
        if (disk[i] == -1) {
            std::cout << "Free";
        } else {
            std::cout << files[disk[i]].fileName;
        }
        std::cout << "] ";

        if ((i + 1) % 10 == 0) std::cout << '\n';
    }

    std::cout << '\n';
}

void ContiguousAllocator::displayFiles() const {
    std::cout << "\nAllocated files:\n";
    std::cout << "Name\tStart\tBlocks\tEnd\n";

    for (const auto& file : files) {
        std::cout << file.fileName << '\t'
                  << file.startBlock << '\t'
                  << file.blockCount << '\t'
                  << file.startBlock + file.blockCount - 1
                  << '\n';
    }
}

int ContiguousAllocator::totalBlocks() const {
    return static_cast<int>(disk.size());
}

int ContiguousAllocator::freeBlocks() const {
    int count = 0;
    for (int block : disk) {
        if (block == -1) ++count;
    }
    return count;
}
