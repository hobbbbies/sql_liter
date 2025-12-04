#include "node.hpp"
#include "row.hpp"
#include "table.hpp"
#include "cursor.hpp"
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
    void* cellPtr = leafNodeCell(cellNum);
    void* valuePtr = static_cast<char*>(cellPtr) + LEAF_NODE_KEY_SIZE;
    return valuePtr;
}

void Node::initializeLeafNode() {
    setNodeType(NodeType::NODE_LEAF);
    *leafNodeNumCells() = 0;
    setNodeRoot(false);
    *leafNodeRightSibling() = 0;
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

// delete later 
void Node::printLeafNode() {
    uint32_t numCells = *leafNodeNumCells();
    std::cout << "leaf (size " << numCells << ")\n";
    for (uint32_t i = 0; i < numCells; i++) {
        uint32_t key = *leafNodeKey(i);
        std::cout << "  - " << i << " : " << key << "\n";
    }
}

void Node::indent(uint32_t level) {
    for (uint32_t i = 0; i < level; i++) {
        std::cout << "  ";
    }
}

void Node::printTree(Table& table, uint32_t rootPageNum, uint32_t indentationLevel) {
    // get node from pager 
    uint8_t* nodeData = table.getPageAddress(rootPageNum);
    Node node(nodeData);
    // switch case on node type
    switch(node.getNodeType()) {
        case NodeType::NODE_LEAF: {
            uint32_t numKeys = *node.leafNodeNumCells();
            indent(indentationLevel);
            std::cout << "- leaf (size " << numKeys << ")\n";
            for (uint32_t i = 0; i < numKeys; i++) {
                uint32_t key = *node.leafNodeKey(i);
                indent(indentationLevel + 1);
                std::cout << "- " << key << "\n";
            }
            break;
        }
        case NodeType::NODE_INTERNAL: {
            uint32_t numKeys = *node.internalNodeNumKeys();
            indent(indentationLevel);
            std::cout << "- internal (size " << numKeys << ")\n";
            for (uint32_t i = 0; i < numKeys; i++) {
                uint32_t childPageNum = *node.internalNodeChild(i);
                std::cout << "child page num: " << childPageNum << "\n";
                std::cout << "index: " << i << "\n";
                indent(indentationLevel + 1);
                std::cout << "- child " << i << " (page " << childPageNum << "):\n";
                node.printTree(table, childPageNum, indentationLevel + 2);
                
                uint32_t key = *node.internalNodeKey(i);
                indent(indentationLevel + 1);
                std::cout << "- key " << i << ": " << key << "\n";
            }
            // print rightmost child
            uint32_t rightChildPageNum = *node.internalNodeRightChild();
            if (rightChildPageNum == INVALID_PAGE_NUM) {
                break;
            }
            std::cout << "right child page num: " << rightChildPageNum << "\n";
            indent(indentationLevel + 1);
            std::cout << "- child " << numKeys << " (page " << rightChildPageNum << "):\n";
            node.printTree(table, rightChildPageNum, indentationLevel + 2);
            break;
        }
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

// returns page of right child 
uint32_t* Node::internalNodeRightChild() {
    return reinterpret_cast<uint32_t*>(static_cast<char*>(data) + INTERNAL_NODE_RIGHT_CHILD_OFFSET);
}

uint32_t* Node::internalNodeCell(uint32_t cellNum) {
    return reinterpret_cast<uint32_t*>(static_cast<char*>(data) + INTERNAL_NODE_HEADER_SIZE + cellNum * INTERNAL_NODE_CELL_SIZE);
}

// returns pointer to child node cell
uint32_t* Node::internalNodeChild(uint32_t childNum) {
    uint32_t numKeys = *internalNodeNumKeys();
    if (childNum > numKeys) {
        throw std::out_of_range("Tried to access child_num " + std::to_string(childNum) + " > num_keys " + std::to_string(numKeys));
    } else if (childNum == numKeys) {
        uint32_t* rightChild = internalNodeRightChild();
        if (*rightChild == INVALID_PAGE_NUM) {
            throw std::out_of_range("Tried to access right child of internal node with no right child");
        }
        return rightChild;
    } else {
        uint32_t* child = internalNodeCell(childNum); 
        if (*child == INVALID_PAGE_NUM) {
            throw std::out_of_range("Tried to access child_num " + std::to_string(childNum) + " but was invalid page num");
        }
        return child;
    }
}

uint32_t* Node::internalNodeKey(uint32_t keyNum) {
    return reinterpret_cast<uint32_t*>(reinterpret_cast<char*>(internalNodeCell(keyNum)) + INTERNAL_NODE_CHILD_SIZE);
}

// Searches parent for index of child by page number
// Returns the index where the child page number is stored, or UINT32_MAX if not found
uint32_t Node::internalNodeFindChild(uint32_t childPageNum) {
    uint32_t numKeys = *internalNodeNumKeys();
    
    // Check all regular children (0 to numKeys-1)
    for (uint32_t i = 0; i < numKeys; i++) {
        if (*internalNodeChild(i) == childPageNum) {
            return i;
        }
    }
    
    // Check right child
    if (*internalNodeRightChild() == childPageNum) {
        return numKeys;  // Right child is conceptually at index numKeys
    }
    
    return UINT32_MAX;  // Not found
}

void Node::initializeInternalNode() {
    setNodeType(NodeType::NODE_INTERNAL);
    *internalNodeNumKeys() = 0;
    *internalNodeRightChild() = INVALID_PAGE_NUM;
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

uint32_t* Node::leafNodeRightSibling() {
    return reinterpret_cast<uint32_t*>(static_cast<char*>(data) + LEAF_NODE_NEXT_LEAF_OFFSET);
}

uint32_t* Node::nodeParent() {
    return reinterpret_cast<uint32_t*>(static_cast<char*>(data) + PARENT_POINTER_OFFSET);
}

void Node::internalNodeUpdateMaxKey(uint32_t childPageNum, uint32_t newNodeMax) {
    uint32_t oldChildIndex = internalNodeFindChild(childPageNum);
    if (oldChildIndex == UINT32_MAX) {
        throw std::runtime_error("Child not found in parent node");
    }
    *internalNodeKey(oldChildIndex) = newNodeMax;
}
