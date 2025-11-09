#include "meta_command_processor.hpp"
#include "table.hpp"
#include "node.hpp"
#include <unordered_map>
#include <functional>
#include <iostream>
#include <cstdlib>

MetaCommandProcessor::MetaCommandProcessor() {
    commands[".exit"] = [](Table* table) {
        return MetaCommandResult::META_COMMAND_EXIT;
    };

    commands[".help"] = [](Table* table) {
        std::cout << "Available commands: .exit, .help, .tables\n";
        return MetaCommandResult::META_COMMAND_SUCCESS;
    };

    commands[".tables"] = [](Table* table) {
        std::cout << "No tables yet!\n";
        return MetaCommandResult::META_COMMAND_SUCCESS;
    };

    commands[".constants"] = [](Table* table) {
        std::cout << "Constants:\n";
        std::cout << "ROW_SIZE_BYTES: " << ROW_SIZE_BYTES << "\n";
        std::cout << "COMMON_NODE_HEADER_SIZE: " << COMMON_NODE_HEADER_SIZE << "\n";
        std::cout << "LEAF_NODE_HEADER_SIZE: " << LEAF_NODE_HEADER_SIZE << "\n";
        std::cout << "LEAF_NODE_CELL_SIZE: " << LEAF_NODE_CELL_SIZE << "\n";
        std::cout << "LEAF_NODE_SPACE_FOR_CELLS: " << LEAF_NODE_SPACE_FOR_CELLS << "\n";
        std::cout << "LEAF_NODE_MAX_CELLS: " << LEAF_NODE_MAX_CELLS << "\n";
        return MetaCommandResult::META_COMMAND_SUCCESS;
    };

    commands[".btree"] = [](Table* table) {
        std::cout << "Tree:\n";
        uint8_t* nodeData = table->getPageAddress(table->getRootPageNum());
        Node node(nodeData);
        node.printLeafNode();
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