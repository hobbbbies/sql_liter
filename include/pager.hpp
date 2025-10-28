#pragma once
#include "constants.hpp"
#include <iostream>
#include <fstream>

class Pager {
private:
    std::fstream fileDescriptor;
    uint32_t fileLength;
    uint8_t* pages[TABLE_MAX_PAGES];
public:
    Pager(const std::string& filename);
    ~Pager();

    Pager* pagerOpen(std::string& filename);
    uint8_t* getPage(uint32_t page_num);
    void pagerClose();
    uint32_t getFileLength();
    void pagerFlush(uint32_t pageNum, uint32_t size=4096);
    // void writePage(uint32_t page_num, uint8_t* page);
    // void readPage(uint32_t page_num, uint8_t* page);
};