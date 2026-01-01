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
    
    // Helper: Fill table until it has an internal root node
    void fillToInternalRoot() {
        for(uint32_t i = 0; i < LEAF_NODE_MAX_CELLS + 1; i++) {
            table->insertRow(Row(i, "test", "test@example.com"));
        }
    }
    
    // Helper: Fill internal node to capacity
    uint32_t fillInternalNodeToCapacity(uint32_t startKey = LEAF_NODE_MAX_CELLS + 1, uint32_t pageNum = 0) {
        uint32_t key = startKey;
        uint32_t targetPage = (pageNum == 0) ? table->getRootPageNum() : pageNum;
        uint8_t* nodeData = table->getPageAddress(targetPage);
        Node node(nodeData);
        
        while (*node.internalNodeNumKeys() < INTERNAL_NODE_MAX_KEYS) {
            for(uint32_t i = 0; i < LEAF_NODE_RIGHT_SPLIT_COUNT + 2; i++) {
                table->insertRow(Row(key++, "test", "test@example.com"));
            }
        }
        return key;  // Return next available key
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
TEST_F(TableTest, InternalNodeDoesntUpdateMaxKeyOnRightInsert) {
    for(uint32_t i = 0; i < LEAF_NODE_MAX_CELLS; i++) {
        table->insertRow(Row(i, "test", "test@example.com"));
    }
    // trigger split
    table->insertRow(Row(LEAF_NODE_MAX_CELLS, "test", "test@example.com"));
    // key should point to largest value of left child

    uint8_t* rootNodeData = table->getPageAddress(table->getRootPageNum());
    Node rootNode(rootNodeData);
    uint32_t maxKey = rootNode.getNodeMaxKey();
    EXPECT_EQ(maxKey, LEAF_NODE_MAX_CELLS / 2); 
    table->insertRow(Row(LEAF_NODE_MAX_CELLS + 1, "test", "test@example.com"));
    EXPECT_EQ(maxKey, LEAF_NODE_MAX_CELLS / 2); 
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

// ensures node split works 
TEST_F(TableTest, InsertionPastMaxCellsDoesNotCrash) {
    // 15 insertions
    for(uint32_t i = 0; i < LEAF_NODE_MAX_CELLS; i++) {
        table->insertRow(Row(i, "test", "test@example.com"));
    }
    table->insertRow(Row(LEAF_NODE_MAX_CELLS, "test", "test@example.com"));
    table->insertRow(Row(LEAF_NODE_MAX_CELLS + 1, "test", "test@example.com"));    
    table->insertRow(Row(LEAF_NODE_MAX_CELLS + 2, "test", "test@example.com"));    
    EXPECT_EQ(table->getUnusedPageNum(), 4);
}


// this does not fucking work
TEST_F(TableTest, InternalNodeUpdatesOnLeftInsert) {
    // Step 1: Fill table and trigger initial split (creates internal root)
    // Insert keys 0-2 (fills to LEAF_NODE_MAX_CELLS = 3)
    for(uint32_t i = 0; i < LEAF_NODE_MAX_CELLS; i++) {
        table->insertRow(Row(i, "test", "test@example.com"));
    }
    // Insert key 3 triggers split: left child [0,1], right child [2,3]
    table->insertRow(Row(LEAF_NODE_MAX_CELLS, "test", "test@example.com"));
    
    uint8_t* rootNodeData = table->getPageAddress(table->getRootPageNum());
    Node rootNode(rootNodeData);
    
    // Verify initial state: parent key should be max of left child (1)
    uint32_t initialParentKey = *rootNode.internalNodeKey(0);
    EXPECT_EQ(initialParentKey, LEAF_NODE_LEFT_SPLIT_COUNT - 1);  // max of left child [0-6]
    
    // Step 2: Fill left child to capacity again by inserting keys 14-19
    // Left child now has [0-6, 14-19] = 13 keys
    for(uint32_t i = 14; i < 14 + (LEAF_NODE_MAX_CELLS - LEAF_NODE_LEFT_SPLIT_COUNT); i++) {
        table->insertRow(Row(i, "test", "test@example.com"));
    }
    
    // Step 3: Trigger split on left child by inserting key 20
    // This should split left child and update parent to have 2 keys
    table->insertRow(Row(20, "test", "test@example.com"));
    
    // Parent should now have 2 keys (pointing to 3 children)
    EXPECT_EQ(*rootNode.internalNodeNumKeys(), 2);
    
    // First parent key should be updated to new max of leftmost child
    uint32_t updatedParentKey = *rootNode.internalNodeKey(0);
    EXPECT_EQ(updatedParentKey, LEAF_NODE_LEFT_SPLIT_COUNT - 1);  // max of leftmost child after split
}

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

TEST_F(TableTest, InternalNodeInsertOnLeftChild) {
    for(uint32_t i = 0; i < LEAF_NODE_MAX_CELLS; i++) {
        table->insertRow(Row(i, "test", "test@example.com"));
    }
    // this will create an internal node @ the root
    table->insertRow(Row(LEAF_NODE_MAX_CELLS, "test", "test@example.com"));
    
    uint8_t* rootNodeData = table->getPageAddress(table->getRootPageNum());
    Node rootNode(rootNodeData);
    EXPECT_EQ(*rootNode.internalNodeKey(0), LEAF_NODE_MAX_CELLS / 2); 
}

TEST_F(TableTest, InternalNodeSplitWorks) {
    // Simple test: fill the root internal node to capacity, then insert one more
    // This should trigger internalNodeSplitAndInsert
    
    std:: cout << "Filling to internal root\n";
    fillToInternalRoot();
    std::cout << "Crashing after here\n";
    
    uint8_t* rootData = table->getPageAddress(table->getRootPageNum());
    Node rootNode(rootData);
    EXPECT_EQ(rootNode.getNodeType(), NodeType::NODE_INTERNAL);
    EXPECT_EQ(*rootNode.internalNodeNumKeys(), 1);

    std::cout << "Filling internal node to capacity\n";
    uint32_t nextKey = fillInternalNodeToCapacity();
    std::cout << "Filled internal node to capacity!\n";

    // Root should now be full
    EXPECT_EQ(*rootNode.internalNodeNumKeys(), INTERNAL_NODE_MAX_KEYS);
    
    // One more insert should trigger internal node split
    table->insertRow(Row(nextKey, "test", "test@example.com"));
    
    // After split, root should have fewer keys (or still be internal with new structure)
    EXPECT_EQ(rootNode.getNodeType(), NodeType::NODE_INTERNAL);
}

// TEST_F(TableTest, InternalNodeSplitWorksAtTwoLevels) {
//     fillToInternalRoot();
    
//     uint8_t* rootData = table->getPageAddress(table->getRootPageNum());
//     Node rootNode(rootData);
//     EXPECT_EQ(rootNode.getNodeType(), NodeType::NODE_INTERNAL);
//     EXPECT_EQ(*rootNode.internalNodeNumKeys(), 1);
    
//     uint32_t nextKey = fillInternalNodeToCapacity();
    
//     // Root should now be full
//     EXPECT_EQ(*rootNode.internalNodeNumKeys(), INTERNAL_NODE_MAX_KEYS);
    
//     // One more insert should trigger internal node split
//     table->insertRow(Row(nextKey, "test", "test@example.com"));
    
//     // After split, root should have fewer keys (or still be internal with new structure)
//     EXPECT_EQ(rootNode.getNodeType(), NodeType::NODE_INTERNAL);
    
//     // everything above here was a repeat of the last test
//     EXPECT_EQ(*rootNode.internalNodeNumKeys(), 2); // should have split into 2 keys now (3 children)
    
//     // Get the rightmost child (where new sequential keys will route to)
//     uint32_t rightChildPageNum = *rootNode.internalNodeRightChild();
//     uint8_t* rightChildData = table->getPageAddress(rightChildPageNum);
//     Node rightChildNode(rightChildData);
    
//     // Verify it's an internal node
//     EXPECT_EQ(rightChildNode.getNodeType(), NodeType::NODE_INTERNAL);
//     uint32_t initialChildKeys = *rightChildNode.internalNodeNumKeys();
    
//     // Fill this child internal node to capacity
//     nextKey = fillInternalNodeToCapacity(nextKey, rightChildPageNum);
    
//     // Verify it's now full
//     EXPECT_EQ(*rightChildNode.internalNodeNumKeys(), INTERNAL_NODE_MAX_KEYS);
    
//     // Insert one more key to trigger split at the second level
//     table->insertRow(Row(nextKey, "test", "test@example.com"));
    
//     // After split, the child internal node should have fewer keys
//     EXPECT_LT(*rightChildNode.internalNodeNumKeys(), INTERNAL_NODE_MAX_KEYS);
    
//     // The root should now have one more key (pointing to the new child created by the split)
//     EXPECT_EQ(*rootNode.internalNodeNumKeys(), 3);
// }
