#include "cursor.hpp"
#include "node.hpp"

Cursor::Cursor(Table& table, uint32_t key) : table(table), endOfTable(false) {
    // Start at root page
    uint32_t rootPageNum = table.getRootPageNum();
    uint8_t* nodeData = table.getPageAddress(rootPageNum);
    Node node(nodeData);     

    // find in node - Either leaf or internal node
    NodeType nodeType = node.getNodeType();
    if (nodeType == NodeType::NODE_LEAF) {
        leafNodeFind(key, rootPageNum);
    } else {
        internalNodeFind(key, rootPageNum);
    }
}


Cursor::~Cursor() {}

// sets cursor to correct cell within given row (pageNum)
void Cursor::leafNodeFind(uint32_t key, uint32_t pageNum) {
    this->pageNum = pageNum;

    uint8_t* nodeData = table.getPageAddress(pageNum);
    Node node(nodeData);
    
    uint32_t minIndex = 0;
    uint32_t onePastMaxIndex = *node.leafNodeNumCells();
    // binary search inside of leaf node to find key OR insertion position
    while (minIndex < onePastMaxIndex) { 
        uint32_t currentIndex = (minIndex + onePastMaxIndex) / 2;
        uint32_t currentKey = *node.leafNodeKey(currentIndex);
        std::cout << "current key: " << currentKey << "\n";
        if (currentKey == key) {
            this->cellNum = currentIndex;
            return;
        } else if (currentKey < key) {
            minIndex = currentIndex + 1;
        } else {
            onePastMaxIndex = currentIndex;
        }
    }
    std::cout << "key not found, pointing to " << minIndex << "\n";
    // if key is not found, cellNum will point to its insertion position; follows BST property
    this->cellNum = minIndex;
}

void Cursor::internalNodeFind(uint32_t key, uint32_t pageNum) {
    std::cout << "Executing internalNodefind for key: " << key << "\n";
    // create node from pagNum
    uint8_t* nodeData = table.getPageAddress(pageNum);
    Node node(nodeData);
    
    // binary search to find index of child node 
    uint32_t minIndex = 0;
    uint32_t maxIndex = *node.internalNodeNumKeys();
    while (minIndex < maxIndex) { 
        uint32_t currentIndex = (minIndex + maxIndex) / 2;
        uint32_t keyToRight = *node.internalNodeKey(currentIndex);
        if (keyToRight >= key) {
            maxIndex = currentIndex;
        } else {
            minIndex = currentIndex + 1;
        }
    }
    
    // call search function on found node
    uint32_t childPageNum = *node.internalNodeChild(minIndex);
    uint8_t* childData = table.getPageAddress(childPageNum);
    Node childNode(childData);
    if (childNode.getNodeType() == NodeType::NODE_LEAF) {
        leafNodeFind(key, childPageNum);
    } else {
        internalNodeFind(key, childPageNum);
    }
}

// Gives pointer in memory to row 
void* Cursor::cursorSlot() {
    uint8_t* nodeData = table.getPageAddress(pageNum); 
    Node node(nodeData);
    return node.leafNodeValue(cellNum);
}

void Cursor::cursorAdvance() {
    cellNum += 1;
    uint8_t* nodeData = table.getPageAddress(pageNum);
    Node node(nodeData);
    uint32_t numCells = *node.leafNodeNumCells();

    if (cellNum >= numCells) {
        // advance to next leaf node 
        uint32_t rightSibling = *node.leafNodeRightSibling();
        if (rightSibling == 0) {
            endOfTable = true;
        } else {
            pageNum = rightSibling;
            cellNum = 0;
            endOfTable = false;
        }
    }
}

