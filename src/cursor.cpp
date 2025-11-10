#include "cursor.hpp"
#include "node.hpp"

Cursor::Cursor(Table& table, uint32_t key, uint32_t pageNum, uint32_t cellNum) : table(table) {
    // create node object
    uint8_t* nodeData = table.getPageAddress(pageNum);
    Node node(nodeData);     

    // find key in node - Either leaf or internal node
    NodeType nodeType = node.getNodeType();
    if (nodeType == NodeType::NODE_LEAF) {
        // stub
    } else {
        // stub
    }
    
    // Use provided pageNum and cellNum, or defaults (start of table)
    if (pageNum == UINT32_MAX) {
        // Default to start of table (root page, cell 0)
        this->pageNum = table.getRootPageNum();
        this->cellNum = 0;
    } else {
        // Use provided values
        this->pageNum = pageNum;
        this->cellNum = (cellNum == UINT32_MAX) ? 0 : cellNum;
    }
    
    if (this->pageNum >= TABLE_MAX_PAGES) {
        throw std::out_of_range("Page number " + std::to_string(this->pageNum) + " exceeds maximum pages (" + std::to_string(TABLE_MAX_PAGES) + ")");
    }

    uint8_t* nodeData = table.getPageAddress(this->pageNum);
    Node node(nodeData);
    uint32_t numCells = *node.leafNodeNumCells();
    
    if (this->cellNum > numCells) {
        throw std::out_of_range("Cell number " + std::to_string(this->cellNum) + " exceeds number of cells (" + std::to_string(numCells) + ")");
    }
    
    this->endOfTable = this->cellNum >= numCells;
}


Cursor::~Cursor() {}

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

