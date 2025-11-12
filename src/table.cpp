#include "table.hpp"
#include "pager.hpp"
#include "cursor.hpp"
#include "node.hpp"
#include <cstring>
#include <stdexcept>
#include <iostream>

Table::Table(std::string filename) {
    pager = new Pager(filename);
    rootPageNum = 0;

    // Empty file ?
    if (pager->getNumPages() == 0) {
        uint8_t* node_data = pager->getPage(rootPageNum);
        Node node(node_data);
        node.initializeLeafNode();
    }
}

Table::~Table() {     
    pager->flushAllPages(num_rows, Row::getRowSize());
    delete pager;
}

// Returns ptr row to serialize; initializes page if needed

uint8_t* Table::getPageAddress(uint32_t pageNum) const{
    if (pageNum >= TABLE_MAX_PAGES) {
        throw std::out_of_range("Page number exceeds maximum pages");
    }
    return pager->getPage(pageNum);
}

void Table::insertRow(const Row& row) {
    uint8_t* nodeData = getPageAddress(rootPageNum);
    Node node(nodeData);
    Cursor cursor(*this, row.getId());
    
    // Check if we're inserting at a position with existing cells
    uint32_t numCells = *node.leafNodeNumCells();
    if (cursor.getCellNum() < numCells) {
        uint32_t keyAtPosition = *node.leafNodeKey(cursor.getCellNum());
        if (keyAtPosition == row.getId()) {
            throw std::invalid_argument("Duplicate key");
        }
    }
    
    node.leafNodeInsert(row.getId(), &row, cursor.getCellNum());
}

Row Table::getRow(uint32_t key) {    
    Cursor cursor(*this, key);
    
    // Check if the key actually exists
    uint8_t* nodeData = getPageAddress(rootPageNum);
    Node node(nodeData);
    uint32_t numCells = *node.leafNodeNumCells();
    
    // If cursor position is beyond valid cells, key doesn't exist
    if (cursor.getCellNum() >= numCells) {
        throw std::out_of_range("Key not found");
    }
    
    // Check if the key at cursor position matches the requested key
    uint32_t keyAtPosition = *node.leafNodeKey(cursor.getCellNum());
    if (keyAtPosition != key) {
        throw std::out_of_range("Key not found");
    }
    
    void* rowAddress = cursor.cursorSlot();        
    return Row::deserialize(rowAddress);
} 

ExecuteResult Table::execute_insert(const std::vector<std::string> tokens) {
    uint8_t* node_data = getPageAddress(rootPageNum);
    Node node(node_data);
    std::cout << "num cells: " << *node.leafNodeNumCells() << "\n";
    // delete below soon 
    // if (*node.leafNodeNumCells() == LEAF_NODE_MAX_CELLS) {
    //     return ExecuteResult::EXECUTE_TABLE_FULL;
    // }

    if (tokens.size() < 4) {
        return ExecuteResult::EXECUTE_FAILURE;
    }
    try {
        if (!tokens[1].empty() && tokens[1][0] == '-') {
            std::cout << "Error: Row ID cannot be negative\n";
            return ExecuteResult::EXECUTE_FAILURE;
        }
        uint32_t rowNum = static_cast<uint32_t>(std::stoul(tokens[1]));
        std::cout << "row num: " << rowNum << "\n";
        std::cout << "num rows: " << num_rows << "\n";
        std::string username = tokens[2];
        std::string email = tokens[3];

        Row newRow(rowNum, username, email);
        insertRow(newRow);
        return ExecuteResult::EXECUTE_SUCCESS;
    } catch (const std::invalid_argument& e) {
        std::cout << "Error: " << e.what() << "\n";
        return ExecuteResult::EXECUTE_DUPLICATE_KEY;
    } catch (const std::exception& e) {
        std::cout << "Error parsing insert values: " << e.what() << "\n";
        return ExecuteResult::EXECUTE_FAILURE;
    }
}

ExecuteResult Table::execute_select_all() {
    try {
        for (uint32_t i = 0; i < num_rows; ++i) {
            Row row = getRow(i);
            row.printRow();
        }

        if (num_rows == 0) {
            std::cout << "No rows in table.\n";
        }

        return ExecuteResult::EXECUTE_SUCCESS;
    } catch (const std::out_of_range& e) {
        throw std::out_of_range("Failed to retrivew a row.\n");
    } catch (const std::exception& e) {
        std::cout << "Error selecting rows: " << e.what() << "\n";
        return ExecuteResult::EXECUTE_FAILURE;
    }
}

ExecuteResult Table::execute_select(const std::vector<std::string>&) {
    // Not implemented yet
    return ExecuteResult::EXECUTE_SUCCESS;
}

uint32_t Table::getNumRows() const {
    uint8_t* node_data = getPageAddress(rootPageNum);
    Node node(node_data);
    return *node.leafNodeNumCells();
}
