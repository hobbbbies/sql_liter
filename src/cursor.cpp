#include "cursor.hpp"

Cursor::Cursor(Table& table, uint32_t rowNum) : table(table), rowNum(rowNum) {}

Cursor::~Cursor() {}

// Gives pointer in memory to row 
void* Cursor::cursorSlot() {
    uint32_t pageNum = rowNum / ROWS_PER_PAGE;
    uint8_t* page = table.getPageAddress(pageNum);  // Let exceptions propagate naturally
    uint32_t row_offset = rowNum % ROWS_PER_PAGE;
    uint32_t byte_offset = row_offset * Row::getRowSize();
    return page + byte_offset;
}

void Cursor::cursorAdvance() {
    rowNum += 1;
}

