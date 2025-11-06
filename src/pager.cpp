#include "pager.hpp"
#include <string>
#include <cstring>
#include "constants.hpp"
#include <iostream>
#include <fstream>
#include <iomanip>

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
    getFdStatus("at init");

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

    getFdStatus("before getPage");

    uint8_t* page = pages[pageNum];
    if (page == nullptr) { 
        // Not cached - this is where pages get allocated!
        std::cout << "DEBUG: Caching page " << pageNum << " for first time\n";
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
            std::cout << "DEBUG: Loaded page " << pageNum << " from file\n";
        } else {
            std::cout << "DEBUG: Created new empty page " << pageNum << "\n";
        }
    }
    getFdStatus("after getPage");
    return page;
}

uint32_t Pager::getFileLength() const {
    return fileLength; 
}

// size = bytes to write (PAGE_SIZE for full page, or calculated size for partial page)
void Pager::pagerFlush(uint32_t pageNum, uint32_t size) {
    getFdStatus("start of pager flush");
    if (pageNum >= TABLE_MAX_PAGES) {
        throw std::out_of_range("Page number exceeds maximum pages");
    }
    
    uint8_t* page = pages[pageNum];
    if (page == nullptr) {
        std::cout << "DEBUG: Tried to flush uncached page " << pageNum << " - skipping\n";
        return; // Nothing to flush
    }
    
    std::cout << "DEBUG: Flushing page " << pageNum << " (size: " << size << " bytes)\n";
    std::cout << "DEBUG: Page memory address: " << static_cast<void*>(page) << "\n";
    
    // Show first 64 bytes of page data in hex format
    std::cout << "DEBUG: First 64 bytes of page data (hex): ";
    for (uint32_t i = 0; i < std::min(size, 64u); ++i) {
        std::cout << std::hex << std::setw(2) << std::setfill('0') 
                  << static_cast<unsigned>(page[i]) << " ";
    }
    std::cout << std::dec << "\n";
    
    // Check if page contains any non-zero data
    bool hasData = false;
    for (uint32_t i = 0; i < size; ++i) {
        if (page[i] != 0) {
            hasData = true;
            break;
        }
    }
    std::cout << "DEBUG: Page contains " << (hasData ? "non-zero" : "all-zero") << " data\n";
    
    Pager::getFdStatus("DEBUG: File descriptor position before tellp: ");

    uint32_t targetPos = pageNum * PAGE_SIZE;
    // Calculate target position
    std::cout << "DEBUG: Target position for page " << pageNum << ": " << targetPos << "\n";
    
    Pager::getFdStatus("DEBUG: File descriptor position before seek: ");
    // Seek to write position
    fileDescriptor.seekp(targetPos, std::ios::beg);
    
    // Show file descriptor position after seeking
    std::streampos afterSeekPos = fileDescriptor.tellp();
    std::cout << "DEBUG: File descriptor position after seek: " << afterSeekPos << "\n";
    
    // Verify seek was successful
    if (afterSeekPos != targetPos) {
        std::cerr << "WARNING: Seek position mismatch! Expected: " << targetPos 
                  << ", Actual: " << afterSeekPos << "\n";
    }
    
    // Check file state before write
    std::cout << "DEBUG: File state before write - good(): " << fileDescriptor.good() 
              << ", eof(): " << fileDescriptor.eof() 
              << ", fail(): " << fileDescriptor.fail() 
              << ", bad(): " << fileDescriptor.bad() << "\n";
    
    // Perform the write
    fileDescriptor.write(reinterpret_cast<const char*>(page), size);
    
    // Show file descriptor position after write
    std::streampos afterWritePos = fileDescriptor.tellp();
    std::cout << "DEBUG: File descriptor position after write: " << afterWritePos << "\n";
    std::cout << "DEBUG: Expected position after write: " << (targetPos + static_cast<std::streampos>(size)) << "\n";

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
    
    // Show final position after flush
    std::streampos finalPos = fileDescriptor.tellp();
    std::cout << "DEBUG: Final file descriptor position after flush: " << finalPos << "\n";
}

void Pager::flushAllPages(uint32_t numRows, uint32_t rowSize) {
    uint32_t rowsPerPage = PAGE_SIZE / rowSize;
    uint32_t fullPages = numRows / rowsPerPage;

    try {        
        std::cout << "DEBUG: Starting flush - numRows: " << numRows 
                  << ", rowSize: " << rowSize 
                  << ", rowsPerPage: " << rowsPerPage 
                  << ", fullPages: " << fullPages << "\n";
        
        // Count how many pages are actually cached
        uint32_t cachedPages = 0;
        for (uint32_t i = 0; i < TABLE_MAX_PAGES; ++i) {
            if (pages[i] != nullptr) {
                cachedPages++;
            }
        }
        std::cout << "DEBUG: Found " << cachedPages << " cached pages out of " << TABLE_MAX_PAGES << " total slots\n";
        
        // Flush full pages
        for (uint32_t i = 0; i < fullPages; i++) {
            pagerFlush(i, PAGE_SIZE);
        }
        
        // Flush partial page if it exists
        uint32_t additionalRows = numRows % rowsPerPage;
        if (additionalRows > 0) {
            uint32_t partialPageSize = additionalRows * rowSize;
            std::cout << "DEBUG: Flushing partial page " << fullPages 
                      << " with " << additionalRows << " rows (" << partialPageSize << " bytes)\n";
            pagerFlush(fullPages, partialPageSize);
        }
        std::cout << "Done! Program safe for termination.\n";
    } catch(const std::exception& e) {
        std::cerr << "FATAL ERROR: Failed to flush data to disk - DATA MAY BE LOST!\n";
        std::cerr << "Error details: " << e.what() << "\n";
        std::cerr << "Database file may be corrupted. Check disk space and permissions.\n";

        std::abort();          // Since this is called from destructor, we can't throw   
    }   
}

