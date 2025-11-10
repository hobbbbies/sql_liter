#include "cursor.hpp"
#include "node.hpp"

Cursor::Cursor(Table& table, uint32_t key) : table(table), endOfTable(false) {
    // Start at root page
    uint32_t rootPageNum = table.getRootPageNum();
    uint8_t* nodeData = table.getPageAddress(rootPageNum);
    Node node(nodeData);     

    // find key in node - Either leaf or internal node
    NodeType nodeType = node.getNodeType();
    if (nodeType == NodeType::NODE_LEAF) {
        leafNodeFind(key, rootPageNum);
    } else {
        // stub for internal nodes
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
    while (minIndex < onePastMaxIndex) { 
        uint32_t currentIndex = (minIndex + onePastMaxIndex) / 2;
        uint32_t currentKey = *node.leafNodeKey(currentIndex);
        if (currentKey == key) {
            this->cellNum = currentIndex;
            return;
        } else if (currentKey < key) {
            minIndex = currentIndex + 1;
        } else {
            onePastMaxIndex = currentIndex;
        }
    }

    this->cellNum = minIndex;
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
    endOfTable = cellNum >= numCells;
}

