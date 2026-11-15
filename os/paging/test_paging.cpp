#include "paging.h"

#include <cassert>
#include <iostream>

int main() {
    // 8 virtual pages, 4 physical frames, 1024 bytes per page.
    PagingSystem paging(8, 4, 1024);

    // Invalid frame number.
    assert(!paging.mapPage(2, 5));

    // Valid mappings.
    assert(paging.mapPage(2, 1));
    assert(paging.mapPage(3, 2));

    // Frame 1 is already occupied.
    assert(!paging.mapPage(4, 1));

    // Translate logical address 2500.
    AddressTranslation result = paging.translateAddress(2500);

    assert(result.success);
    assert(result.pageNumber == 2);
    assert(result.offset == 452);
    assert(result.frameNumber == 1);
    assert(result.physicalAddress == 1476);

    std::cout << "Logical Address: " << result.logicalAddress << '\n';
    std::cout << "Page Number: " << result.pageNumber << '\n';
    std::cout << "Offset: " << result.offset << '\n';
    std::cout << "Frame Number: " << result.frameNumber << '\n';
    std::cout << "Physical Address: " << result.physicalAddress << '\n';

    // Unmapped page should produce a page fault.
    AddressTranslation fault = paging.translateAddress(4500);
    assert(!fault.success);

    std::cout << "\nUnmapped page: " << fault.message << '\n';

    // Invalid logical addresses.
    assert(!paging.translateAddress(-1).success);
    assert(!paging.translateAddress(8192).success);

    // Unmap page 2.
    assert(paging.unmapPage(2));
    assert(!paging.translateAddress(2500).success);

    std::cout << "\nAll paging tests passed!\n";

    return 0;
}
