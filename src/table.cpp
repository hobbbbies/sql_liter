#include "table.hpp"

#include <cstring>
#include <stdexcept>
#include <iostream>

Table::Table() : num_rows(0) {
    for (int i = 0; i < TABLE_MAX_PAGES; ++i) {
        pages[i] = nullptr;
    }
}

Table::~Table() {
    for (int i = 0; i < TABLE_MAX_PAGES; ++i) {
        delete[] pages[i];
    }
}

void* Table::row_slot(uint32_t row_num) {
    uint32_t page_num = row_num / ROWS_PER_PAGE;

    if (page_num >= TABLE_MAX_PAGES) {
        throw std::out_of_range("Page number exceeds maximum pages");
    }

    uint8_t* page = pages[page_num];
    if (page == nullptr) {
        page = pages[page_num] = new uint8_t[PAGE_SIZE];
        std::memset(page, 0, PAGE_SIZE);
    }

    uint32_t row_offset = row_num % ROWS_PER_PAGE;
    uint32_t byte_offset = row_offset * Row::getRowSize();
    return page + byte_offset;
}

void Table::insertRow(const Row& row) {
    if (num_rows == TABLE_MAX_ROWS) { 
        
        throw std::out_of_range("Table is full");
    }
    void* slot = row_slot(num_rows);
    row.serialize(slot);
    ++num_rows;
}

Row Table::getRow(uint32_t row_num) const {
    if (row_num >= num_rows) {
        throw std::out_of_range("Row number out of bounds");
    }
    uint32_t page_num = row_num / ROWS_PER_PAGE;
    uint32_t row_offset = row_num % ROWS_PER_PAGE;
    uint32_t byte_offset = row_offset * Row::getRowSize();

    return Row::deserialize(pages[page_num] + byte_offset);
}

ExecuteResult Table::execute_insert(const std::vector<std::string> tokens) {
    if (num_rows == TABLE_MAX_ROWS) {
        return ExecuteResult::EXECUTE_TABLE_FULL;
    }

    if (tokens.size() < 4) {
        return ExecuteResult::EXECUTE_FAILURE;
    }
    try {
        uint32_t rowNum = static_cast<uint32_t>(std::stoul(tokens[1]));
        std::cout << "row num: " << rowNum << "\n";
        std::cout << "num rows: " << num_rows << "\n";
        std::string username = tokens[2];
        std::string email = tokens[3];

        Row newRow(rowNum, username, email);
        insertRow(newRow);
        return ExecuteResult::EXECUTE_SUCCESS;
    } catch (const std::exception& e) {
        std::cout << "Error parsing insert values" << e.what() << "\n";
        return ExecuteResult::EXECUTE_FAILURE;
    }
}

ExecuteResult Table::execute_select_all() {
    for (uint32_t i = 0; i < num_rows; ++i) {
        Row row = getRow(i);
        row.printRow();
    }

    if (num_rows == 0) {
        std::cout << "No rows in table.\n";
    }

    return ExecuteResult::EXECUTE_SUCCESS;
}

ExecuteResult Table::execute_select(const std::vector<std::string>& /*tokens*/) {
    // Not implemented yet
    return ExecuteResult::EXECUTE_SUCCESS;
}
