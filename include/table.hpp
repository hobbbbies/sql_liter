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
    Pager* pager;
    uint32_t rootPageNum;
    uint32_t num_rows = 0; // TO BE REMOVED

public:
    Table(std::string filename);
    ~Table();
    
    uint8_t* getPageAddress(uint32_t pageNum) const;
    uint32_t getNumRows() const;  
    void incrementRows() { ++num_rows; }
    uint32_t getRootPageNum() const { return rootPageNum; }
    void insertRow(const Row& row);
    Row getRow(uint32_t key);

    ExecuteResult execute_insert(const std::vector<std::string> tokens);
    ExecuteResult execute_select_all();
    ExecuteResult execute_select(const std::vector<std::string>& tokens);
};
