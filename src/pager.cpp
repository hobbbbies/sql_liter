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

    numPages = fileLength / PAGE_SIZE;         
    if (fileLength % PAGE_SIZE) {
        std::cerr <<"Error: File size is not a multiple of page size. Corrupt File\n";
        exit(EXIT_FAILURE);
    }
    
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

void Pager::getFdStatus(const std::string& context) {
    std::streampos currentPos = fileDescriptor.tellp();
    std::cout << "DEBUG " << context << " - File position: " << currentPos 
              << ", EOF?: " << fileDescriptor.eof() << "\n";
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
        // Not cached - this is where pages get allocated!
        page = pages[pageNum] = new uint8_t[PAGE_SIZE];
        std::memset(page, 0, PAGE_SIZE);
        
        // Check if page_num is in range of numPages. If it is, we need to read from file
        // else, just return page pointer. Read from it later. 
        if (pageNum < numPages) {
            // Reads file and stores into page ptr 
            fileDescriptor.seekg(pageNum * PAGE_SIZE, std::ios::beg);
            fileDescriptor.read(reinterpret_cast<char*>(page), PAGE_SIZE);

            if (fileDescriptor.fail()) {
                if (fileDescriptor.eof()) {
                    fileDescriptor.clear();
                } else {
                    std::cerr << "Error reading page " << pageNum << std::endl;
                    delete[] page;
                    pages[pageNum] = nullptr;
                    throw std::runtime_error("Failed to read page from file");
                }                
            }
        } 
    }
    // do after file reading incase of fail 
    if (pageNum >= numPages) {
        numPages = pageNum + 1;
    }
    
    return page;
}

uint32_t Pager::getFileLength() const {
    return fileLength; 
}

// size = bytes to write (PAGE_SIZE for full page, or calculated size for partial page)
void Pager::pagerFlush(uint32_t pageNum) {
    if (pageNum >= TABLE_MAX_PAGES) {
        throw std::out_of_range("Page number exceeds maximum pages");
    }
    
    uint8_t* page = pages[pageNum];
    if (page == nullptr) {
        return; // Nothing to flush
    }
    // Optional: could validate page contents here if needed
    uint32_t targetPos = pageNum * PAGE_SIZE;
    // Seek to write position
    fileDescriptor.seekp(targetPos, std::ios::beg);
    
    // Show file descriptor position after seeking
    std::streampos afterSeekPos = fileDescriptor.tellp();
    
    // Verify seek was successful
    if (afterSeekPos != targetPos) {
        std::cerr << "WARNING: Seek position mismatch! Expected: " << targetPos 
                  << ", Actual: " << afterSeekPos << "\n";
    }
    
    // Perform the write
    fileDescriptor.write(reinterpret_cast<const char*>(page), PAGE_SIZE);
    
    if (fileDescriptor.fail()) {
        if (fileDescriptor.bad()) {
            std::cerr << "A severe, non-recoverable error occurred." << std::endl;
        } else if (fileDescriptor.eof()) {
            std::cerr << "End of file reached during an operation." << std::endl;
        } else {
            std::cerr << "A logical error occurred during file operation." << std::endl;
        }
        std::cerr << "Error flushing page " << pageNum << std::endl;
        throw std::runtime_error("Failed to write page to file");
    }
    
    fileDescriptor.flush();
}

void Pager::flushAllPages(uint32_t numRows, uint32_t rowSize) {

    try {        
        for (uint32_t i = 0; i < numPages; i++) {
            pagerFlush(i);
        }

        std::cout << "Done! Program safe for termination.\n";
    } catch(const std::exception& e) {
        std::cerr << "FATAL ERROR: Failed to flush data to disk - DATA MAY BE LOST!\n";
        std::cerr << "Error details: " << e.what() << "\n";
        std::cerr << "Database file may be corrupted. Check disk space and permissions.\n";

        std::abort();          // Since this is called from destructor, we can't throw   
    }   
}

uint32_t Pager::getNumPages() {
    return numPages;
}