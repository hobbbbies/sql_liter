#include "node.hpp"

uint32_t* Node::leafNodeNumCells() {
    return reinterpret_cast<uint32_t*>(static_cast<char*>(data) + LEAF_NODE_NUM_CELLS_OFFSET);
}

void* Node::leafNodeCell(uint32_t cellNum) {
    return static_cast<char*>(data) + LEAF_NODE_HEADER_SIZE + cellNum * LEAF_NODE_CELL_SIZE;
}

uint32_t* Node::leafNodeKey(uint32_t cellNum) {
    return reinterpret_cast<uint32_t*>(leafNodeCell(cellNum));
}

void* Node::leafNodeValue(uint32_t cellNum) {
    return static_cast<char*>(leafNodeCell(cellNum)) + LEAF_NODE_KEY_SIZE;
}

void Node::initializeLeafNode() {
    *leafNodeNumCells() = 0;
}

void Node::leafNodeInsert(uint32_t key, Row* value, uint32_t cellNum) {
    
    // split here 
    if (cellNum >= LEAF_NODE_MAX_CELLS) {
        throw std::out_of_range("Cell number exceeds maximum cells");
    }

    uint32_t numCells = *leafNodeNumCells();

    if (cellNum < numCells) {
        for(uint32_t i = numCells; i > cellNum; i--) {
            std::memcpy(leafNodeCell(i), leafNodeCell(i-1), LEAF_NODE_CELL_SIZE);
        }
    }
    
    uint8_t* cell = leafNodeCell(cellNum);
    *leafNodeKey(cellNum) = key;
    std::memcpy(leafNodeValue(cellNum), value, ROW_SIZE);
    *leafNodeNumCells() = numCells + 1;
}
