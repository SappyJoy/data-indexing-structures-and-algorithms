#include "common/Utils.hpp"
#include "gtest/gtest.h"
#include <fcntl.h>

namespace ehash {

TEST(PrintTest, temporary_print_test) {
    common::print("Hello, World!");
    EXPECT_EQ(2, 2);
    EXPECT_NE(2, 3);
}

} // namespace ehash
