#include <iostream>
#include <string>
#include <cstdlib>
#include <unordered_map>
#include <functional>

enum class MetaCommandResult {
    META_COMMAND_SUCCESS,
    META_COMMAND_UNRECOGNIZED_COMMAND
};

enum class PrepareResult { 
    PREPARE_SUCCESS, 
    PREPARE_UNRECOGNIZED_STATEMENT 
};

class InputBuffer {
private: 
    std::string buffer;
public: 
    InputBuffer() = default;    

    const std::string& getBuffer() const { return buffer; }

    void readInput() {
        std::getline(std::cin, buffer);
        if (std::cin.fail()) {
            std::cout << "Error reading input\n";
            std::exit(EXIT_FAILURE);
        }
    }
};

class StatementProcessor {
private:
    std::unordered_map<std::string, std::function<PrepareResult()>> statements;
    
public:
    StatementProcessor() {
        statements["insert"] = []() {
            std::cout << "This is where we would do an insert.\n";
            return PrepareResult::PREPARE_SUCCESS;
        };
        
        statements["select"] = []() {
            std::cout << "This is where we would do a select.\n";
            return PrepareResult::PREPARE_SUCCESS;
        };
    }

    PrepareResult execute(const std::string& statement) {
        auto it = statements.find(statement);
        if (it != statements.end()) {
            return it->second();
        }
        return PrepareResult::PREPARE_UNRECOGNIZED_STATEMENT;
    }
};

class MetaCommandProcessor {
private:
    std::unordered_map<std::string, std::function<MetaCommandResult()>> commands;
    
public:
    MetaCommandProcessor() {
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
    
    MetaCommandResult execute(const std::string& command) {
        auto it = commands.find(command);
        if (it != commands.end()) {
            return it->second();
        }
        return MetaCommandResult::META_COMMAND_UNRECOGNIZED_COMMAND;
    }
};



void printPrompt() { 
    std::cout << "db > "; 
}

int main() {
    InputBuffer inputBuffer;
    MetaCommandProcessor metaProcessor;
    StatementProcessor statementProcessor;
    
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
                    continue;
            }
        }
        

        PrepareResult result = statementProcessor.execute(inputBuffer.getBuffer());
        switch (result) {      
            case PrepareResult::PREPARE_SUCCESS:
                std::cout << "Executed.\n";
                break;      
            case PrepareResult::PREPARE_UNRECOGNIZED_STATEMENT:
                std::cout << "Unrecognized keyword at start of '" << inputBuffer.getBuffer() << "'.\n";
                continue;
        }
    }
    
    return 0;
}