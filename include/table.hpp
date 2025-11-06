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

public:
    Table(std::string filename);
    ~Table();
    
    uint8_t* getPageAddress(uint32_t pageNum) const;
    uint8_t* row_slot(uint32_t row_num) const;
    uint32_t getNumRows() const { return num_rows; }
    void incrementRows() { ++num_rows; }
    void insertRow(const Row& row);
    Row getRow(uint32_t row_num);

    ExecuteResult execute_insert(const std::vector<std::string> tokens);
    ExecuteResult execute_select_all();
    ExecuteResult execute_select(const std::vector<std::string>& tokens);
};
