#include <gtest/gtest.h>
#include "table.hpp"
#include "row.hpp"
#include <cstdio>

class TableTest : public ::testing::Test {
protected:
    void SetUp() override {
        table = std::make_unique<Table>("test.txt");
    }

    void TearDown() override {
        table.reset();
        std::remove("test.txt");
        std::remove("test2.txt");
    }
    
    std::unique_ptr<Table> table;
};

TEST_F(TableTest, EmptyTableHasZeroRows) {
    std::unique_ptr<Table> empty_table = std::make_unique<Table>("test2.txt");
    EXPECT_EQ(empty_table->getNumRows(), 0);
}

TEST_F(TableTest, InsertSingleRow) {
    Row row(1, "john", "john@example.com");
    table->insertRow(row);

    
    EXPECT_EQ(table->getNumRows(), 1);
}

TEST_F(TableTest, InsertAndRetrieveRow) {
    Row original(123, "bob", "bob@test.com");
    table->insertRow(original);
    
    Row retrieved = table->getRow(123);
    
    EXPECT_EQ(retrieved.getId(), 123);
    EXPECT_STREQ(retrieved.getUsername(), "bob");
    EXPECT_STREQ(retrieved.getEmail(), "bob@test.com");
}

TEST_F(TableTest, InsertMultipleRows) {
    table->insertRow(Row(1, "alice", "alice@test.com"));
    table->insertRow(Row(2, "bob", "bob@test.com"));
    table->insertRow(Row(3, "charlie", "charlie@test.com"));
    
    EXPECT_EQ(table->getNumRows(), 3);
    
    Row second = table->getRow(2);
    EXPECT_EQ(second.getId(), 2);
    EXPECT_STREQ(second.getUsername(), "bob");
}

TEST_F(TableTest, GetRowOutOfBounds) {
    
    std::unique_ptr<Table> empty_table = std::make_unique<Table>("test2.txt");
    empty_table->insertRow(Row(1, "test", "test@example.com"));

    EXPECT_THROW(empty_table->getRow(2), std::out_of_range);
    EXPECT_THROW(empty_table->getRow(100), std::out_of_range);
}

// leafNodeSplit
TEST_F(CursorTest, LeafNodeSplitAndInsertIsCalledOnRightSize) {
    
}

TEST_F(CursorTest, NumPagesIs2AfterSplit) {
}