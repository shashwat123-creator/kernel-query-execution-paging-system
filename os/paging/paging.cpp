#include "paging.h"

#include <stdexcept>

PagingSystem::PagingSystem(
    int virtualPages,
    int physicalFrames,
    int pageSize
) {
    if (virtualPages <= 0 || physicalFrames <= 0 || pageSize <= 0) {
        throw std::invalid_argument("Paging dimensions must be positive");
    }

    this->pageSize = pageSize;
    this->frameCount = physicalFrames;

    pageTable.resize(virtualPages, {-1, false});
    frameOccupied.resize(physicalFrames, false);
}

bool PagingSystem::mapPage(int pageNumber, int frameNumber) {
    if (pageNumber < 0 ||
        pageNumber >= static_cast<int>(pageTable.size()) ||
        frameNumber < 0 ||
        frameNumber >= frameCount) {
        return false;
    }

    if (pageTable[pageNumber].valid || frameOccupied[frameNumber]) {
        return false;
    }

    pageTable[pageNumber] = {frameNumber, true};
    frameOccupied[frameNumber] = true;

    return true;
}

bool PagingSystem::unmapPage(int pageNumber) {
    if (pageNumber < 0 ||
        pageNumber >= static_cast<int>(pageTable.size())) {
        return false;
    }

    if (!pageTable[pageNumber].valid) {
        return false;
    }

    int frameNumber = pageTable[pageNumber].frameNumber;

    frameOccupied[frameNumber] = false;
    pageTable[pageNumber] = {-1, false};

    return true;
}

AddressTranslation PagingSystem::translateAddress(
    int logicalAddress
) const {
    AddressTranslation result{};

    result.success = false;
    result.logicalAddress = logicalAddress;
    result.pageNumber = -1;
    result.offset = -1;
    result.frameNumber = -1;
    result.physicalAddress = -1;
    result.message = "Invalid logical address";

    if (logicalAddress < 0) {
        return result;
    }

    long long virtualAddressSpace =
        static_cast<long long>(pageTable.size()) * pageSize;

    if (logicalAddress >= virtualAddressSpace) {
        return result;
    }

    int pageNumber = logicalAddress / pageSize;
    int offset = logicalAddress % pageSize;

    result.pageNumber = pageNumber;
    result.offset = offset;

    if (!pageTable[pageNumber].valid) {
        result.message = "Page fault: page is not in physical memory";
        return result;
    }

    int frameNumber = pageTable[pageNumber].frameNumber;

    result.success = true;
    result.frameNumber = frameNumber;

    result.physicalAddress =
        static_cast<long long>(frameNumber) * pageSize + offset;

    result.message = "Address translated successfully";

    return result;
}

int PagingSystem::getPageSize() const {
    return pageSize;
}

int PagingSystem::getFrameCount() const {
    return frameCount;
}

int PagingSystem::getVirtualPageCount() const {
    return static_cast<int>(pageTable.size());
}

const std::vector<PageTableEntry>& PagingSystem::getPageTable() const {
    return pageTable;
}
