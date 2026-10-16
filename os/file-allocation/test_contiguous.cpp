#include "contiguous.h"
#include <iostream>

int main() {
    ContiguousAllocator disk(20);

    auto a = disk.allocate("FileA", 5);
    auto b = disk.allocate("FileB", 4);
    auto c = disk.allocate("FileC", 3);

    std::cout << a.message << '\n';
    std::cout << b.message << '\n';
    std::cout << c.message << '\n';

    disk.displayDisk();
    disk.displayFiles();

    std::cout << "\nFree blocks: " << disk.freeBlocks() << '\n';

    std::cout << "\nDeleting FileB...\n";
    disk.deallocate("FileB");
    disk.displayDisk();

    return 0;
}
