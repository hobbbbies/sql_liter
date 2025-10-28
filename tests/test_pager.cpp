#include <gtest/gtest.h>
#include "table.hpp"
#include "pager.hpp"

class PagerTest : public ::testing::Test {
protected:
    void SetUp() override {
        pager = std::make_unique<Pager>("test.txt");
    }

    void TearDown() override {
        std::remove("test.txt");
    }
    
    std::unique_ptr<Pager> pager;
};

TEST_F(PagerTest, ConstructorWorks) {
    ASSERT_NE(pager, nullptr);
}

TEST_F(PagerTest, FileDescriptorIsProperlyInitialized) {
    // Verify that the pager was created successfully
    ASSERT_NE(pager, nullptr);
    
    // If your Pager class has a method to get the file descriptor, use it:
    // int fd = pager->getFileDescriptor();
    // EXPECT_GE(fd, 0) << "File descriptor should be non-negative";
    
    // If your Pager class has a method to check if it's valid/open:
    // EXPECT_TRUE(pager->isOpen()) << "File should be opened successfully";
    
    // Alternative: Check if the file was created on disk
    std::ifstream file("test.txt");
    EXPECT_TRUE(file.good()) << "Test file should exist and be accessible";
    file.close();
}

TEST_F(PagerTest, FileLengthZeroOnOpen) {
    ASSERT_EQ(pager->getFileLength(), 0);
}

TEST_F(PagerTest, GetPageReturnsValidPtr) {
    ASSERT_NE(pager->getPage(1), nullptr);
}

TEST_F(PagerTest, GetPageThrowsErrorOutOfBounds) {
    // Test that accessing a page beyond TABLE_MAX_PAGES throws std::out_of_range
    EXPECT_THROW(pager->getPage(TABLE_MAX_PAGES), std::out_of_range);
    
    // You can also test with a value way beyond the limit
    EXPECT_THROW(pager->getPage(TABLE_MAX_PAGES + 100), std::out_of_range);
}

TEST_F(PagerTest, GetPageDoesNotThrowForValidPage) {
    // Test that accessing valid pages doesn't throw
    EXPECT_NO_THROW(pager->getPage(0));
    EXPECT_NO_THROW(pager->getPage(TABLE_MAX_PAGES - 1));
}




