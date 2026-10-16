#include "contiguous.h"
#include <iostream>

int main() {
    ContiguousAllocator disk(10);

    disk.allocate("A", 3);
    disk.allocate("B", 3);
    disk.allocate("C", 3);

    disk.deallocate("A");
    disk.deallocate("C");

    std::cout << "Free blocks: " << disk.freeBlocks() << '\n';

    auto result = disk.allocate("D", 5);
    std::cout << result.message << '\n';

    return 0;
}
