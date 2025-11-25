#pragma once 

#include "constants.hpp"
#include <cstdint>
#include "row.hpp"
#include "enums.hpp"
#include "table.hpp"

class Node {
private:
    void* data;    
public:
    Node(void* node_data) : data(node_data) {}
    
    // Leaf node methods
    uint32_t* leafNodeNumCells();
    void* leafNodeCell(uint32_t cellNum);
    uint32_t* leafNodeKey(uint32_t cellNum);
    void* leafNodeValue(uint32_t cellNum);
    void initializeLeafNode();
    void leafNodeInsert(uint32_t key, const Row* value, uint32_t cellNum);
    void printLeafNode();
    uint32_t* leafNodeRightSibling();
    
    // Internal Node methods 
    // all uint32_t* returns represent page numbers
    uint32_t* internalNodeNumKeys();
    uint32_t* internalNodeRightChild();
    uint32_t* internalNodeCell(uint32_t cellNum);
    uint32_t* internalNodeChild(uint32_t childNum);
    uint32_t* internalNodeKey(uint32_t keyNum);
    uint32_t internalNodeFindChild(uint32_t childPageNum);
    void internalNodeUpdateMaxKey(uint32_t childPageNum, uint32_t newNodeMax);
    void initializeInternalNode();
    
    // Node utility methods
    uint32_t getNodeMaxKey();
    uint32_t* nodeParent();

    // Node type methods
    NodeType getNodeType() const;
    void setNodeType(NodeType type);
    bool isRootNode();
    void setNodeRoot(bool isRoot);

    // print methods
    void indent(uint32_t level);
    void printTree(Table& table, uint32_t rootPageNum, uint32_t indentationLevel = 0);
};