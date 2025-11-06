#pragma once

#include "table.hpp"
#include "enums.hpp"


class Cursor {
private:    
    Table& table;
    uint32_t rowNum = 0;
public:
    Cursor(Table& table, uint32_t rowNum);
    ~Cursor();
    void* cursorSlot();
    void cursorAdvance();
};