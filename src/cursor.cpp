#include "cursor.hpp"
#include "node.hpp"

Cursor::Cursor(Table& table, uint32_t pageNum, uint32_t cellNum) : table(table), pageNum(pageNum), cellNum(cellNum) {
    if (pageNum >= TABLE_MAX_PAGES) {
        throw std::out_of_range("Page number " + std::to_string(pageNum) + " exceeds maximum pages (" + std::to_string(TABLE_MAX_PAGES) + ")");
    }

    uint8_t* nodeData = table.getPageAddress(pageNum);
    Node node(nodeData);
    uint32_t numCells = *node.leafNodeNumCells();
    if (cellNum >= numCells) {
        throw std::out_of_range("Cell number " + std::to_string(cellNum) + " exceeds number of cells (" + std::to_string(numCells) + ")");
    }
    endOfTable = cellNum >= numCells;
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

