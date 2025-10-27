#include <iostream>
#include "enums.hpp"
#include "input_buffer.hpp"
#include "meta_command_processor.hpp"
#include "statement_processor.hpp"
#include "table.hpp"
#include "utils.hpp"

int main() {
    InputBuffer inputBuffer;
    MetaCommandProcessor metaProcessor;
    Table db_table;
    StatementProcessor statementProcessor = StatementProcessor(db_table);
    
    while (true) {
        printPrompt();
        inputBuffer.readInput();

        if (inputBuffer.getBuffer()[0] == '.') {
            MetaCommandResult result = metaProcessor.execute(inputBuffer.getBuffer());
            switch (result) {
                case MetaCommandResult::META_COMMAND_SUCCESS:
                    std::cout << "Executed.\n";
                    break;      
                case MetaCommandResult::META_COMMAND_UNRECOGNIZED_COMMAND:
                    std::cout << "Unrecognized command at start of '" << inputBuffer.getBuffer() << "'.\n";
                    break;
            }
            continue;
        }
        
        PrepareResult result = statementProcessor.execute(inputBuffer.getBuffer());
        switch (result) {      
            case PrepareResult::PREPARE_SUCCESS:
                std::cout << "Executed.\n";
                break;      
            case PrepareResult::PREPARE_UNRECOGNIZED_STATEMENT:
                std::cout << "Unrecognized keyword at start of '" << inputBuffer.getBuffer() << "'.\n";
                continue;
            case PrepareResult::PREPARE_SYNTAX_ERROR:
                std::cout << "Invalid syntax\n";
        }
    }
    
    return 0;
}
