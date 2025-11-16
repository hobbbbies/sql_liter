#include <gtest/gtest.h>
#include "node.hpp"
#include "row.hpp"
#include <cstdio>
#include <cstring>

class NodeTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Allocate a page-sized buffer for node data
        pageData = new uint8_t[PAGE_SIZE];
        std::memset(pageData, 0, PAGE_SIZE);
        node = std::make_unique<Node>(pageData);
    }
    
    void TearDown() override {
        node.reset();
        delete[] pageData;
    }
    
    uint8_t* pageData;
    std::unique_ptr<Node> node;
};

// Leaf Node Tests

TEST_F(NodeTest, InitializeLeafNode) {
    node->initializeLeafNode();
    EXPECT_EQ(node->getNodeType(), NodeType::NODE_LEAF);
    EXPECT_EQ(*node->leafNodeNumCells(), 0);
    EXPECT_EQ(node->isRootNode(), false);
}

TEST_F(NodeTest, SetAndGetNodeType) {
    node->setNodeType(NodeType::NODE_LEAF);
    EXPECT_EQ(node->getNodeType(), NodeType::NODE_LEAF);
    
    node->setNodeType(NodeType::NODE_INTERNAL);
    EXPECT_EQ(node->getNodeType(), NodeType::NODE_INTERNAL);
}

TEST_F(NodeTest, SetAndGetRootStatus) {
    node->initializeLeafNode();
    EXPECT_EQ(node->isRootNode(), false);
    
    node->setNodeRoot(true);
    EXPECT_EQ(node->isRootNode(), true);
    
    node->setNodeRoot(false);
    EXPECT_EQ(node->isRootNode(), false);
}

TEST_F(NodeTest, LeafNodeInsertSingleCell) {
    node->initializeLeafNode();
    
    Row row(1, "alice", "alice@example.com");
    node->leafNodeInsert(1, &row, 0);
    
    EXPECT_EQ(*node->leafNodeNumCells(), 1);
    EXPECT_EQ(*node->leafNodeKey(0), 1);
    
    // Deserialize and verify
    Row retrieved;
    retrieved.deserialize(node->leafNodeValue(0));
    EXPECT_EQ(retrieved.getId(), 1);
    std::cout << "retrieved username: " << retrieved.getUsername() << std::endl;
    EXPECT_EQ(retrieved.getUsername(), "alice");
    std::cout << "retrieved email: " << retrieved.getEmail() << std::endl;
    EXPECT_EQ(retrieved.getEmail(), "alice@example.com");
}

TEST_F(NodeTest, LeafNodeInsertMultipleCells) {
    node->initializeLeafNode();
    
    Row row1(1, "alice", "alice@example.com");
    Row row2(2, "bob", "bob@example.com");
    Row row3(3, "charlie", "charlie@example.com");
    
    node->leafNodeInsert(1, &row1, 0);
    node->leafNodeInsert(2, &row2, 1);
    node->leafNodeInsert(3, &row3, 2);
    
    EXPECT_EQ(*node->leafNodeNumCells(), 3);
    EXPECT_EQ(*node->leafNodeKey(0), 1);
    EXPECT_EQ(*node->leafNodeKey(1), 2);
    EXPECT_EQ(*node->leafNodeKey(2), 3);
}

TEST_F(NodeTest, LeafNodeInsertInMiddle) {
    node->initializeLeafNode();
    
    Row row1(1, "alice", "alice@example.com");
    Row row3(3, "charlie", "charlie@example.com");
    Row row2(2, "bob", "bob@example.com");
    
    node->leafNodeInsert(1, &row1, 0);
    node->leafNodeInsert(3, &row3, 1);
    node->leafNodeInsert(2, &row2, 1);  // Insert in middle
    
    EXPECT_EQ(*node->leafNodeNumCells(), 3);
    EXPECT_EQ(*node->leafNodeKey(0), 1);
    EXPECT_EQ(*node->leafNodeKey(1), 2);
    EXPECT_EQ(*node->leafNodeKey(2), 3);
}

TEST_F(NodeTest, LeafNodeInsertThrowsOnMaxCells) {
    node->initializeLeafNode();
    
    Row row(1, "test", "test@example.com");
    EXPECT_THROW(node->leafNodeInsert(1, &row, LEAF_NODE_MAX_CELLS), std::out_of_range);
}

TEST_F(NodeTest, LeafNodeGetMaxKey) {
    node->initializeLeafNode();
    
    Row row1(5, "alice", "alice@example.com");
    Row row2(10, "bob", "bob@example.com");
    Row row3(15, "charlie", "charlie@example.com");
    
    node->leafNodeInsert(5, &row1, 0);
    node->leafNodeInsert(10, &row2, 1);
    node->leafNodeInsert(15, &row3, 2);
    
    EXPECT_EQ(node->getNodeMaxKey(), 15);
}

// Internal Node Tests

TEST_F(NodeTest, InitializeInternalNode) {
    node->initializeInternalNode();
    EXPECT_EQ(node->getNodeType(), NodeType::NODE_INTERNAL);
    EXPECT_EQ(*node->internalNodeNumKeys(), 0);
    EXPECT_EQ(node->isRootNode(), false);
}

TEST_F(NodeTest, InternalNodeSetNumKeys) {
    node->initializeInternalNode();
    *node->internalNodeNumKeys() = 5;
    EXPECT_EQ(*node->internalNodeNumKeys(), 5);
}

TEST_F(NodeTest, InternalNodeSetRightChild) {
    node->initializeInternalNode();
    *node->internalNodeRightChild() = 42;
    EXPECT_EQ(*node->internalNodeRightChild(), 42);
}

TEST_F(NodeTest, InternalNodeChildAccessRightChild) {
    node->initializeInternalNode();
    *node->internalNodeNumKeys() = 2;
    *node->internalNodeRightChild() = 99;
    
    // Accessing child at numKeys should return right child
    EXPECT_EQ(*node->internalNodeChild(2), 99);
}

TEST_F(NodeTest, InternalNodeChildAccessRegularChild) {
    node->initializeInternalNode();
    *node->internalNodeNumKeys() = 2;
    
    // Set a child pointer
    *node->internalNodeCell(0) = 10;
    EXPECT_EQ(*node->internalNodeChild(0), 10);
}

TEST_F(NodeTest, InternalNodeChildThrowsOnInvalidAccess) {
    node->initializeInternalNode();
    *node->internalNodeNumKeys() = 2;
    
    // Accessing beyond numKeys should throw
    EXPECT_THROW(*node->internalNodeChild(3), std::out_of_range);
}

TEST_F(NodeTest, InternalNodeKeyAccess) {
    node->initializeInternalNode();
    *node->internalNodeNumKeys() = 1;
    
    // Set a key
    *node->internalNodeKey(0) = 100;
    EXPECT_EQ(*node->internalNodeKey(0), 100);
}

TEST_F(NodeTest, InternalNodeGetMaxKey) {
    node->initializeInternalNode();
    *node->internalNodeNumKeys() = 3;
    
    *node->internalNodeKey(0) = 10;
    *node->internalNodeKey(1) = 20;
    *node->internalNodeKey(2) = 30;
    
    EXPECT_EQ(node->getNodeMaxKey(), 30);
}
