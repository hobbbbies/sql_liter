#include <gtest/gtest.h>
#include "table.hpp"
#include "row.hpp"
#include <cstdio>
#include "node.hpp"

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
TEST_F(TableTest, LeafNodeSplitAndInsertIsCalledOnRightSize) {
    for(uint32_t i = 0; i < LEAF_NODE_MAX_CELLS; i++) {
        table->insertRow(Row(i, "test", "test@example.com"));
    }
    EXPECT_EQ(table->getNumRows(), LEAF_NODE_MAX_CELLS);
    uint8_t* rootNodeData = table->getPageAddress(table->getRootPageNum());
    Node rootNode(rootNodeData);
    EXPECT_EQ(rootNode.getNodeType(), NodeType::NODE_LEAF);
    table->insertRow(Row(LEAF_NODE_MAX_CELLS, "test", "test@example.com"));
    EXPECT_EQ(rootNode.getNodeType(), NodeType::NODE_INTERNAL);    
}

TEST_F(TableTest, NumPagesIs3AfterSplit) {
    for(uint32_t i = 0; i < LEAF_NODE_MAX_CELLS; i++) {
        table->insertRow(Row(i, "test", "test@example.com"));
    }
    EXPECT_EQ(table->getUnusedPageNum(), 1);
    table->insertRow(Row(LEAF_NODE_MAX_CELLS, "test", "test@example.com"));
    EXPECT_EQ(table->getUnusedPageNum(), 3);
}

TEST_F(TableTest, NewRootPointsToCorrectChildren) {
    for(uint32_t i = 0; i < LEAF_NODE_MAX_CELLS; i++) {
        table->insertRow(Row(i, "test", "test@example.com"));
    }
    table->insertRow(Row(LEAF_NODE_MAX_CELLS, "test", "test@example.com"));
    uint8_t* rootNodeData = table->getPageAddress(table->getRootPageNum());
    Node rootNode(rootNodeData);
    // because right child is created before left childs new position
    EXPECT_EQ(*rootNode.internalNodeChild(0), 2);
    EXPECT_EQ(*rootNode.internalNodeChild(1), 1);
}

TEST_F(TableTest, NewRootHasCorrectNumKeys) {
    for(uint32_t i = 0; i < LEAF_NODE_MAX_CELLS; i++) {
        table->insertRow(Row(i, "test", "test@example.com"));
    }
    table->insertRow(Row(LEAF_NODE_MAX_CELLS, "test", "test@example.com"));
    uint8_t* rootNodeData = table->getPageAddress(table->getRootPageNum());
    Node rootNode(rootNodeData);
    EXPECT_EQ(*rootNode.internalNodeNumKeys(), 1); 
}

TEST_F(TableTest, InsertionPastMaxCellsDoesNotCrash) {
    // 15 insertions
    for(uint32_t i = 0; i < LEAF_NODE_MAX_CELLS; i++) {
        table->insertRow(Row(i, "test", "test@example.com"));
    }
    table->insertRow(Row(LEAF_NODE_MAX_CELLS, "test", "test@example.com"));
    table->insertRow(Row(LEAF_NODE_MAX_CELLS + 1, "test", "test@example.com"));    
    table->insertRow(Row(LEAF_NODE_MAX_CELLS + 2, "test", "test@example.com"));    
    EXPECT_EQ(table->getUnusedPageNum(), 3);
}