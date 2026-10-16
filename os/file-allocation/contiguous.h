#ifndef CONTIGUOUS_H
#define CONTIGUOUS_H

#include <string>
#include <vector>

struct FileAllocation {
    std::string fileName;
    int startBlock;
    int blockCount;
};

struct AllocationResult {
    bool success;
    std::string message;
    FileAllocation file;
};

class ContiguousAllocator {
private:
    std::vector<int> disk;
    std::vector<FileAllocation> files;

public:
    explicit ContiguousAllocator(int totalBlocks);

    AllocationResult allocate(const std::string& fileName, int blockCount);
    bool deallocate(const std::string& fileName);
    void displayDisk() const;
    void displayFiles() const;

    int totalBlocks() const;
    int freeBlocks() const;
};

#endif
