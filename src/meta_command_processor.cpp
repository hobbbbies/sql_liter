#include "meta_command_processor.hpp"
#include "table.hpp"
#include <unordered_map>
#include <functional>
#include <iostream>
#include <cstdlib>

MetaCommandProcessor::MetaCommandProcessor() {
    commands[".exit"] = [](Table* table) {
        std::exit(EXIT_SUCCESS);
        return MetaCommandResult::META_COMMAND_SUCCESS;
    };

    commands[".help"] = [](Table* table) {
        std::cout << "Available commands: .exit, .help, .tables\n";
        return MetaCommandResult::META_COMMAND_SUCCESS;
    };

    commands[".tables"] = [](Table* table) {
        std::cout << "No tables yet!\n";
        return MetaCommandResult::META_COMMAND_SUCCESS;
    };
}

MetaCommandResult MetaCommandProcessor::execute(const std::string& command, Table* table) {
    auto it = commands.find(command);
    if (it != commands.end()) {
        return it->second(table);
    }
    return MetaCommandResult::META_COMMAND_UNRECOGNIZED_COMMAND;
}