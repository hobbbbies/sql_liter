#pragma once 

#include "constants.hpp"
#include <cstdint>
#include "row.hpp"

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
    void leafNodeInsert(uint32_t key, Row* value, uint32_t cellNum);
};