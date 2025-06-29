#include <gtest/gtest.h>
#include "bloch/version/version.hpp"

TEST(VersionTest, HasCorrectVersion) {
    EXPECT_STREQ(bloch::version, "0.1.0");
}
