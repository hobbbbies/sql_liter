#pragma once

#include "table.hpp"
#include "enums.hpp"
#include "node.hpp"


class Cursor {
private:    
    Table& table;
    uint32_t pageNum;
    uint32_t cellNum;
    bool endOfTable;
public:
    Cursor(Table& table, uint32_t pageNum, uint32_t cellNum);
    ~Cursor();
    void* cursorSlot();
    void cursorAdvance();
};