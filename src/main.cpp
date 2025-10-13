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

enum class StatementType { 
    STATEMENT_INSERT, 
    STATEMENT_SELECT 
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

class Statement {
public:
    StatementType type;
    Statement(StatementType t) : type(t) {}

    void execute() const {
        switch (type) {
            case StatementType::STATEMENT_INSERT:
                std::cout << "This is where we would do an insert.\n";
                break;
            case StatementType::STATEMENT_SELECT:
                std::cout << "This is where we would do a select.\n";
                break;
        }
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

class StatementProcessor {
public:
    PrepareResult prepareStatement(const std::string& input, Statement& statement) {
        // Simple parsing - check first word
        if (input.substr(0, 6) == "insert") {
            statement = Statement(StatementType::STATEMENT_INSERT);
            return PrepareResult::PREPARE_SUCCESS;
        }
        if (input == "select") {
            statement = Statement(StatementType::STATEMENT_SELECT);
            return PrepareResult::PREPARE_SUCCESS;
        }
        return PrepareResult::PREPARE_UNRECOGNIZED_STATEMENT;
    }
};

void printPrompt() { 
    std::cout << "db > "; 
}

int main() {
    InputBuffer inputBuffer;
    MetaCommandProcessor metaProcessor;  // Create once, reuse
    StatementProcessor statementProcessor;  // Create once, reuse
    
    while (true) {
        printPrompt();
        inputBuffer.readInput();
        
        if (inputBuffer.getBuffer()[0] == '.') {
            // Handle meta commands
            MetaCommandResult result = metaProcessor.execute(inputBuffer.getBuffer());
            if (result == MetaCommandResult::META_COMMAND_UNRECOGNIZED_COMMAND) {
                std::cout << "Unrecognized command '" << inputBuffer.getBuffer() << "'\n";
            }
            continue;
        }
        
        // Handle SQL statements
        Statement statement(StatementType::STATEMENT_SELECT); // Default
        PrepareResult result = statementProcessor.prepareStatement(inputBuffer.getBuffer(), statement);
        
        switch (result) {
            case PrepareResult::PREPARE_SUCCESS:
                statement.execute();
                std::cout << "Executed.\n";
                break;
            case PrepareResult::PREPARE_UNRECOGNIZED_STATEMENT:
                std::cout << "Unrecognized keyword at start of '" << inputBuffer.getBuffer() << "'.\n";
                break;
        }
    }
    
    return 0;
}