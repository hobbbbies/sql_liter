#pragma once

#include <unordered_map>
#include <functional>
#include <string>
#include "enums.hpp"
#include "table.hpp"

class MetaCommandProcessor {
private:
    std::unordered_map<std::string, std::function<MetaCommandResult(Table* table)>> commands;

public:
    MetaCommandProcessor();
    MetaCommandResult execute(const std::string& command, Table* table);
};