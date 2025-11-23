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
    return static_cast<char*>(leafNodeCell(cellNum)) + LEAF_NODE_KEY_SIZE;
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

uint32_t* Node::internalNodeRightChild() {
    return reinterpret_cast<uint32_t*>(static_cast<char*>(data) + INTERNAL_NODE_RIGHT_CHILD_OFFSET);
}

uint32_t* Node::internalNodeCell(uint32_t cellNum) {
    return reinterpret_cast<uint32_t*>(static_cast<char*>(data) + INTERNAL_NODE_HEADER_SIZE + cellNum * INTERNAL_NODE_CELL_SIZE);
}

// returns pointer to child node cell
uint32_t* Node::internalNodeChild(uint32_t childNum) {
    uint32_t numKeys = *internalNodeNumKeys();
    std::cout << "numKeys: " << numKeys << "\n";
    std::cout << "childNum: " << childNum << "\n";
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

// stripped down version of Cursor::leafNodeFind
// searches parent for index of child 
uint32_t Node::internalNodeFindChild(uint32_t childKey) {
    uint32_t numKeys = *internalNodeNumKeys();
    uint32_t minIndex = 0;
    uint32_t maxIndex = numKeys;
    while(minIndex < maxIndex) {
        uint32_t currentIndex = (minIndex + maxIndex) / 2;
        if (*internalNodeKey(currentIndex) == childKey) {
            return currentIndex;
        } else if (*internalNodeKey(currentIndex) < childKey) {
            minIndex = currentIndex + 1;
        } else {
            maxIndex = currentIndex;
        }
    }

    return -1;
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

uint32_t* Node::leafNodeRightSibling() {
    return reinterpret_cast<uint32_t*>(static_cast<char*>(data) + LEAF_NODE_NEXT_LEAF_OFFSET);
}

uint32_t* Node::nodeParent() {
    return reinterpret_cast<uint32_t*>(static_cast<char*>(data) + PARENT_POINTER_OFFSET);
}

