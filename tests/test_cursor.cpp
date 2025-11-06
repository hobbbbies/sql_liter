#include <gtest/gtest.h>
#include "cursor.hpp"
#include "table.hpp"
#include "row.hpp"
#include <cstdio>

class CursorTest : public ::testing::Test {
protected:
    void SetUp() override {
        test_filename = "test_cursor.db";
        std::remove(test_filename.c_str());  // Clean up any existing file
        table = std::make_unique<Table>(test_filename);
    }
    
    void TearDown() override {
        table.reset();
        std::remove(test_filename.c_str());
    }
    
    std::string test_filename;
    std::unique_ptr<Table> table;
};

TEST_F(CursorTest, InitializesCursorAtFirstRow) {
    // Create cursor at row 0 (first row position)
    Cursor cursor(*table, 0);
    
    // The cursor should be pointing to the beginning of the table
    // We can verify this by checking that the slot points to page 0, offset 0
    void* slot = cursor.cursorSlot();
    
    // The slot should not be null
    ASSERT_NE(slot, nullptr);
    
    // For row 0, we should be at the start of page 0
    uint8_t* page0 = table->getPageAddress(0);
    EXPECT_EQ(slot, page0);  // First row should be at start of first page
}

TEST_F(CursorTest, InitializesCursorAtEndOfTable) {
    // Create cursor at the last possible row position
    uint32_t lastRowPosition = TABLE_MAX_ROWS - 1;
    Cursor cursor(*table, lastRowPosition);
    
    // The cursor should be pointing to the last possible position
    void* slot = cursor.cursorSlot();
    
    // The slot should not be null
    ASSERT_NE(slot, nullptr);
    
    // Calculate expected page and offset for the last row
    uint32_t expectedPage = lastRowPosition / ROWS_PER_PAGE;
    uint32_t expectedRowOffset = lastRowPosition % ROWS_PER_PAGE;
    uint32_t expectedByteOffset = expectedRowOffset * Row::getRowSize();
    
    uint8_t* expectedPagePtr = table->getPageAddress(expectedPage);
    void* expectedSlot = expectedPagePtr + expectedByteOffset;
    
    EXPECT_EQ(slot, expectedSlot);
}

TEST_F(CursorTest, CursorSlotCalculatesCorrectOffset) {
    // Test cursor at row 5 (arbitrary middle position)
    uint32_t testRowNum = 5;
    Cursor cursor(*table, testRowNum);
    
    void* slot = cursor.cursorSlot();
    ASSERT_NE(slot, nullptr);
    
    // Calculate expected position manually
    uint32_t expectedPage = testRowNum / ROWS_PER_PAGE;
    uint32_t expectedRowOffset = testRowNum % ROWS_PER_PAGE;
    uint32_t expectedByteOffset = expectedRowOffset * Row::getRowSize();
    
    uint8_t* expectedPagePtr = table->getPageAddress(expectedPage);
    void* expectedSlot = expectedPagePtr + expectedByteOffset;
    
    EXPECT_EQ(slot, expectedSlot);
}

TEST_F(CursorTest, CursorAcrossDifferentPages) {
    // Test cursor at position that should be on second page
    uint32_t secondPageRowNum = ROWS_PER_PAGE + 1;  // First row of second page + 1
    Cursor cursor(*table, secondPageRowNum);
    
    void* slot = cursor.cursorSlot();
    ASSERT_NE(slot, nullptr);
    
    // This should be on page 1
    uint32_t expectedPage = secondPageRowNum / ROWS_PER_PAGE;
    EXPECT_EQ(expectedPage, 1);  // Should be on second page
    
    uint32_t expectedRowOffset = secondPageRowNum % ROWS_PER_PAGE;
    uint32_t expectedByteOffset = expectedRowOffset * Row::getRowSize();
    
    uint8_t* expectedPagePtr = table->getPageAddress(expectedPage);
    void* expectedSlot = expectedPagePtr + expectedByteOffset;
    
    EXPECT_EQ(slot, expectedSlot);
}
