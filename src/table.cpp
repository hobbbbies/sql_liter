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
    uint32_t insertPos = *node.leafNodeNumCells();  // Always insert at end
    node.leafNodeInsert(row.getId(), &row, insertPos);
}

Row Table::getRow(uint32_t row_num) {
    if (row_num >= num_rows) {
        throw std::out_of_range("row_num exceeds num_rows");
    }
    
    Cursor cursor(*this, row_num);
    void* rowAddress = cursor.cursorSlot();        
    return Row::deserialize(rowAddress);
}

ExecuteResult Table::execute_insert(const std::vector<std::string> tokens) {
    uint8_t* node_data = getPageAddress(rootPageNum);
    Node node(node_data);
    if (*node.leafNodeNumCells() == LEAF_NODE_MAX_CELLS) {
        return ExecuteResult::EXECUTE_TABLE_FULL;
    }

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
