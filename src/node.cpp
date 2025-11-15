#include "node.hpp"
#include "row.hpp"
#include <iostream>

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
    setNodeType(NodeType::NODE_LEAF);
    *leafNodeNumCells() = 0;
    setNodeRoot(false);
}

void Node::leafNodeInsert(uint32_t key, const Row* value, uint32_t cellNum) {    
    // split here 
    if (cellNum >= LEAF_NODE_MAX_CELLS) {
        throw std::out_of_range("Cell number exceeds maximum cells - Make call to split first");
    }

    uint32_t numCells = *leafNodeNumCells();

    if (cellNum < numCells) {
        for(uint32_t i = numCells; i > cellNum; i--) {
            std::memcpy(leafNodeCell(i), leafNodeCell(i-1), LEAF_NODE_CELL_SIZE);
        }
    }
    
    *leafNodeKey(cellNum) = key;
    value->serialize(leafNodeValue(cellNum));
    *leafNodeNumCells() = numCells + 1;
}

void Node::printLeafNode() {
    uint32_t numCells = *leafNodeNumCells();
    std::cout << "leaf (size " << numCells << ")\n";
    for (uint32_t i = 0; i < numCells; i++) {
        uint32_t key = *leafNodeKey(i);
        std::cout << "  - " << i << " : " << key << "\n";
    }
}

NodeType Node::getNodeType() const {
    return static_cast<NodeType>(*static_cast<uint8_t*>(data));
}

void Node::setNodeType(NodeType type) {
    *static_cast<uint8_t*>(data) = static_cast<uint8_t>(type);
}

bool Node::isRootNode() {
    uint8_t* val = static_cast<uint8_t*>(data) + IS_ROOT_OFFSET;
    return static_cast<bool>(*val);
}

void Node::setNodeRoot(bool isRoot) {
    uint8_t value = isRoot;
    *(static_cast<uint8_t*>(data) + IS_ROOT_OFFSET) = value;
}

uint32_t* Node::internalNodeNumKeys() {
    return reinterpret_cast<uint32_t*>(static_cast<char*>(data) + INTERNAL_NODE_NUM_KEYS_OFFSET);
}

uint32_t* Node::internalNodeRightChild() {
    return reinterpret_cast<uint32_t*>(static_cast<char*>(data) + INTERNAL_NODE_RIGHT_CHILD_OFFSET);
}

uint32_t* Node::internalNodeCell(uint32_t cellNum) {
    return reinterpret_cast<uint32_t*>(static_cast<char*>(data) + INTERNAL_NODE_HEADER_SIZE + cellNum * INTERNAL_NODE_CELL_SIZE);
}

uint32_t* Node::internalNodeChild(uint32_t childNum) {
    uint32_t numKeys = *internalNodeNumKeys();
    if (childNum > numKeys) {
        throw std::out_of_range("Tried to access child_num " + std::to_string(childNum) + " > num_keys " + std::to_string(numKeys));
    } else if (childNum == numKeys) {
        return internalNodeRightChild();
    } else {
        return internalNodeCell(childNum);
    }
}

uint32_t* Node::internalNodeKey(uint32_t keyNum) {
    return internalNodeCell(keyNum) + INTERNAL_NODE_CHILD_SIZE / sizeof(uint32_t);
}

void Node::initializeInternalNode() {
    setNodeType(NodeType::NODE_INTERNAL);
    *internalNodeNumKeys() = 0;
    setNodeRoot(false);
}

uint32_t Node::getNodeMaxKey() {
    switch (getNodeType()) {
        case NodeType::NODE_INTERNAL:
            return *internalNodeKey(*internalNodeNumKeys() - 1);
        case NodeType::NODE_LEAF:
            return *leafNodeKey(*leafNodeNumCells() - 1);
        default:
            throw std::logic_error("Unknown node type");
    }
}