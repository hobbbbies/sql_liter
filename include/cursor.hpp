#pragma once

#include "table.hpp"
#include "enums.hpp"


class Cursor {
private:    
    Table& table;
    uint32_t rowNum = 0;
    bool endOfTable = false;
public:
    Cursor(Table& table, uint32_t rowNum, bool endOfTable);
    ~Cursor();
    void* cursorSlot();
};