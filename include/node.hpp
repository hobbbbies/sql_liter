#pragma once 

#include "constants.hpp"
#include <cstdint>
#include "row.hpp"
#include "enums.hpp"

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
    
    // Internal Node methods 
    uint32_t* internalNodeNumKeys();
    uint32_t* internalNodeRightChild();
    uint32_t* internalNodeCell(uint32_t cellNum);
    uint32_t* internalNodeChild(uint32_t childNum);
    uint32_t* internalNodeKey(uint32_t keyNum);
    void initializeInternalNode();
    
    // Node utility methods
    uint32_t getNodeMaxKey();

    // Node type methods
    NodeType getNodeType() const;
    void setNodeType(NodeType type);
    bool isRootNode();
    void setNodeRoot(bool isRoot);
};