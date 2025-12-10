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
    uint32_t rootPageNum; // root node key

public:
    Table(std::string filename);
    ~Table();
    
    uint8_t* getPageAddress(uint32_t pageNum) const;
    uint32_t getRootPageNum() const { return rootPageNum; }
    void insertRow(const Row& row);
    Row getRow(uint32_t key);
    void leafNodeSplitAndInsert(uint32_t key, const Row* value, uint32_t cellNumToInsertAt, uint32_t oldNodePageNum);
    uint32_t getUnusedPageNum() const { return pager->getNumPages(); }
    uint32_t getNumRows() const;
    void createNewRoot(uint32_t rightChildPageNum);
    void internalNodeInsert(uint32_t key, uint32_t childPageNum);
    void internalNodeSplitAndInsert(uint32_t parentPageNum, uint32_t oldNodePageNum);

    ExecuteResult execute_insert(const std::vector<std::string> tokens);
    ExecuteResult execute_insert_multiple(const std::vector<std::string> tokens);
    ExecuteResult execute_select_all();
    ExecuteResult execute_select(const std::vector<std::string>& tokens);
};
