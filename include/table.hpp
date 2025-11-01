#pragma once

#include <cstdint>
#include <vector>
#include <string>

#include "constants.hpp"
#include "enums.hpp"
#include "row.hpp"

#include "pager.hpp"

class Table {
private:
    uint32_t num_rows;
    Pager* pager;

    static constexpr uint32_t ROWS_PER_PAGE = PAGE_SIZE / Row::getRowSize();
    static constexpr uint32_t TABLE_MAX_ROWS = ROWS_PER_PAGE * TABLE_MAX_PAGES;
public:
    Table(std:: string filename);
    ~Table();
    
    uint8_t* row_slot(uint32_t row_num) const;
    uint32_t getNumRows() const { return num_rows; }
    void incrementRows() { ++num_rows; }
    void insertRow(const Row& row);
    Row getRow(uint32_t row_num) const;

    ExecuteResult execute_insert(const std::vector<std::string> tokens);
    ExecuteResult execute_select_all();
    ExecuteResult execute_select(const std::vector<std::string>& tokens);
};
