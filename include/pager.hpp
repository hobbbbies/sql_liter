#pragma once
#include "constants.hpp"
#include <iostream>
#include <fstream>

class Pager {
private:
    std::fstream fileDescriptor;
    uint32_t fileLength;
    uint8_t* pages[TABLE_MAX_PAGES];
    void getFdStatus(const std::string& context);  // Debug helper method
    uint32_t numPages;

public:
    Pager(const std::string& filename);
    ~Pager();

    uint8_t* getPage(uint32_t page_num);
    uint32_t getFileLength() const;
    void pagerFlush(uint32_t pageNum);
    void flushAllPages(uint32_t numRows, uint32_t rowSize);  // New method
    uint32_t getNumPages() const { return numPages; }
};
