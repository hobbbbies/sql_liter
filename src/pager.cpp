#include "pager.hpp"
#include <string>
#include <cstring>
#include "constants.hpp"
#include <iostream>
#include <fstream>

Pager::Pager(const std::string& filename) {
    fileDescriptor.open(filename, std::ios::in | std::ios::out | std::ios::binary);

    // open without read mode, then reopen
    if (!fileDescriptor.is_open()) {
        fileDescriptor.clear(); // Clear any error flags
        fileDescriptor.open(filename, std::ios::out | std::ios::binary);
        fileDescriptor.close();
        fileDescriptor.open(filename, std::ios::in | std::ios::out | std::ios::binary);
    }

    if (!fileDescriptor.is_open()) {
        std::cerr << "Error: could not open file." << std::endl;
        exit(EXIT_FAILURE);
    }

    fileDescriptor.seekg(0, std::ios::end);
    fileLength = static_cast<uint32_t>(fileDescriptor.tellg());
    fileDescriptor.seekg(0, std::ios::beg);
    
    for (int i = 0; i < TABLE_MAX_PAGES; i++) {
        pages[i] = nullptr;    
    } 
}

Pager::~Pager() {
    if (fileDescriptor.is_open()) {
        fileDescriptor.close();
    }

    for (int i = 0; i < TABLE_MAX_PAGES; i++) {
       delete[] pages[i];
    }
}

/* 
Tries to locate page
if page already cached, we can return it 
else, need to allocate memory for it, and retrieve it from the file
*/
uint8_t* Pager::getPage(uint32_t pageNum) {
    if (pageNum >= TABLE_MAX_PAGES) {
        throw std::out_of_range("Page number exceeds maximum pages");
    }

    uint8_t* page = pages[pageNum];
    if (page == nullptr) { 
        // Not cached
        page = pages[pageNum] = new uint8_t[PAGE_SIZE];
        std::memset(page, 0, PAGE_SIZE);
        
        uint32_t numPages = fileLength / PAGE_SIZE;         
        // If there's an extra page that doesn't take up a full 4096 kb
        if (fileLength % PAGE_SIZE) {
            numPages++;
        }

        // Check if page_num is in range of numPages. If it is, we need to read from file
        // else, just return page pointer. Read from it later. 
        if (pageNum < numPages) {
            // Reads file and stores into page ptr 
            fileDescriptor.seekg(pageNum * PAGE_SIZE, std::ios::beg);
            fileDescriptor.read(reinterpret_cast<char*>(page), PAGE_SIZE);

            if (fileDescriptor.fail() && !fileDescriptor.eof()) {
                std::cerr << "Error reading page " << pageNum << std::endl;
                delete[] page;
                pages[pageNum] = nullptr;
                throw std::runtime_error("Failed to read page from file");
            }
        } 
    }

    return page;
}

uint32_t Pager::getFileLength() const { 
    return fileLength; 
}

// size = bytes to write (PAGE_SIZE for full page, or calculated size for partial page)
void Pager::pagerFlush(uint32_t pageNum, uint32_t size) {
    if (pageNum >= TABLE_MAX_PAGES) {
        throw std::out_of_range("Page number exceeds maximum pages");
    }
    
    uint8_t* page = pages[pageNum];
    if (page == nullptr) {
        return; // Nothing to flush
    }

    fileDescriptor.seekp(pageNum * PAGE_SIZE, std::ios::beg);
    fileDescriptor.write(reinterpret_cast<const char*>(page), size);

    if (fileDescriptor.fail()) {
        std::cerr << "Error flushing page " << pageNum << std::endl;
        throw std::runtime_error("Failed to write page to file");
    }
    
    fileDescriptor.flush();
}

void Pager::flushAllPages(uint32_t numRows, uint32_t rowSize) {
    uint32_t rowsPerPage = PAGE_SIZE / rowSize;
    uint32_t fullPages = numRows / rowsPerPage;
    
    // Flush full pages
    for (uint32_t i = 0; i < fullPages; i++) {
        pagerFlush(i, PAGE_SIZE);
    }
    
    // Flush partial page if it exists
    uint32_t additionalRows = numRows % rowsPerPage;
    if (additionalRows > 0) {
        uint32_t partialPageSize = additionalRows * rowSize;
        pagerFlush(fullPages, partialPageSize);
    }
}
