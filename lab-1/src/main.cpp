#include "ehash/ExtensibleHashing.hpp"
#include <cassert>
#include <iostream>

using namespace ehash;

int main(int argc, char *argv[]) {
    ExtensibleHashing hash_table(2);

    // Insert some values
    assert(hash_table.insert(10, 100));
    assert(hash_table.insert(22, 200));
    assert(hash_table.insert(15, 300));

    // Search for values
    assert(hash_table.search(10) == 100);
    assert(hash_table.search(22) == 200);

    // Remove a value
    assert(hash_table.remove(10));

    assert(hash_table.search(10) == std::nullopt);

    // Insert more values to trigger bucket split
    assert(hash_table.insert(31, 400));

    // Print the state of the hash table
    hash_table.print();

    std::cout << "All tests passed!\n";
}
