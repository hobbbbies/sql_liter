#include "cursor.hpp"

Cursor::Cursor(Table& table, uint32_t rowNum) : table(table), rowNum(rowNum) {}

Cursor::~Cursor() {}

// Gives pointer in memory to row 
void* Cursor::cursorSlot() {
    uint32_t pageNum = rowNum / ROWS_PER_PAGE;
    try {
        uint8_t* page = table.getPageAddress(pageNum);
        uint32_t row_offset = rowNum % ROWS_PER_PAGE;
        uint32_t byte_offset = row_offset * Row::getRowSize();
        return page + byte_offset;
    } catch(std::exception e) {
        std::cerr << "Error slotting cursor: " << e.what() << "\n";
        return nullptr;
    }
}

void Cursor::cursorAdvance() {
    rowNum += 1;
     if (rowNum >= table.getNumRows()) {
        endOfTable = true;
    }
}

