#ifndef PAGING_H
#define PAGING_H

#include <vector>

struct PageTableEntry {
    int frameNumber;
    bool valid;
};

struct AddressTranslation {
    bool success;
    int logicalAddress;
    int pageNumber;
    int offset;
    int frameNumber;
    long long physicalAddress;
    const char* message;
};

class PagingSystem {
private:
    int pageSize;
    int frameCount;
    std::vector<PageTableEntry> pageTable;
    std::vector<bool> frameOccupied;

public:
    PagingSystem(int virtualPages, int physicalFrames, int pageSize);

    bool mapPage(int pageNumber, int frameNumber);
    bool unmapPage(int pageNumber);

    AddressTranslation translateAddress(int logicalAddress) const;

    int getPageSize() const;
    int getFrameCount() const;
    int getVirtualPageCount() const;

    const std::vector<PageTableEntry>& getPageTable() const;
};

#endif
