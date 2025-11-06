#include "cursor.hpp"

Cursor::Cursor(Table& table, uint32_t rowNum, bool endOfTable) : table(table), rowNum(rowNum), endOfTable(endOfTable) {}

Cursor::~Cursor() {}

void* Cursor::cursorSlot() {
    uint32_t page_num = rowNum / ROWS_PER_PAGE;
    try {

    } catch(std::exception e) {
        std::cerr << "Error slotting cursor: " << e.what() << "\n";
    }
    uint32_t row_offset = rowNum % ROWS_PER_PAGE;
    uint32_t byte_offset = row_offset * Row::getRowSize();
    return page + byte_offset;
}