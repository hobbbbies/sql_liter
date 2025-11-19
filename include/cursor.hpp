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
    Cursor(Table& table, uint32_t key);
    ~Cursor();
    void* cursorSlot();
    void cursorAdvance();
    uint32_t getCellNum() const { return cellNum; }
    uint32_t getPageNum() const { return pageNum; }
private:
    void leafNodeFind(uint32_t key, uint32_t pageNum);
    void internalNodeFind(uint32_t key, uint32_t pageNum);
};