#pragma once

#include <string>

#include "enums.hpp"
#include "table.hpp"

class StatementProcessor {
private:
    // kept private in impl to reduce header deps
    class Impl;
    Impl* pimpl;

public:
    explicit StatementProcessor(Table& db_table);
    ~StatementProcessor();

    PrepareResult execute(const std::string& statement);
};
