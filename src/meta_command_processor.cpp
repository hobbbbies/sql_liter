#include "meta_command_processor.hpp"

#include <unordered_map>
#include <functional>
#include <iostream>
#include <cstdlib>

MetaCommandProcessor::MetaCommandProcessor() {
    commands[".exit"] = []() {
        std::exit(EXIT_SUCCESS);
        return MetaCommandResult::META_COMMAND_SUCCESS;
    };

    commands[".help"] = []() {
        std::cout << "Available commands: .exit, .help, .tables\n";
        return MetaCommandResult::META_COMMAND_SUCCESS;
    };

    commands[".tables"] = []() {
        std::cout << "No tables yet!\n";
        return MetaCommandResult::META_COMMAND_SUCCESS;
    };
}

MetaCommandResult MetaCommandProcessor::execute(const std::string& command) {
    auto it = commands.find(command);
    if (it != commands.end()) {
        return it->second();
    }
    return MetaCommandResult::META_COMMAND_UNRECOGNIZED_COMMAND;
}