#include "statement_processor.hpp"

#include <unordered_map>
#include <functional>
#include <iostream>

#include "tokenizer.hpp"

class StatementProcessor::Impl {
public:
    std::unordered_map<std::string, std::function<PrepareResult(const std::string&)>> statements;
    Table& table;

    explicit Impl(Table& db_table) : table(db_table) {
        statements["insert"] = [this](const std::string& fullCommand) {
            std::cout << "Full command: " << fullCommand << "\n";
            auto tokens = tokenize(fullCommand);
            if (tokens.size() >= 4) {
                ExecuteResult result = table.execute_insert(tokens);
                if (result == ExecuteResult::EXECUTE_FAILURE) {
                    return PrepareResult::PREPARE_INTERNAL_FAILURE;
                } else if (result == ExecuteResult::EXECUTE_TABLE_FULL) {
                    std::cout << "Table is full\n";
                    return PrepareResult::PREPARE_INTERNAL_FAILURE;
                } else if (result == ExecuteResult::EXECUTE_DUPLICATE_KEY) {
                    std::cout << "Duplicate key error\n";
                    return PrepareResult::PREPARE_INTERNAL_FAILURE;
                }
                std::cout << "Table: " << tokens[2] << "\n";
            } else {
                std::cout << "exiting";
                return PrepareResult::PREPARE_SYNTAX_ERROR;
            }
            return PrepareResult::PREPARE_SUCCESS;
        };

        statements["select"] = [this](const std::string& fullCommand) {
            table.execute_select_all();
            return PrepareResult::PREPARE_SUCCESS;
        };
    }

    PrepareResult execute(const std::string& statement) {
        size_t spacePos = statement.find(' ');
        std::string commandType = (spacePos != std::string::npos) ? statement.substr(0, spacePos) : statement;

        auto it = statements.find(commandType);
        if (it != statements.end()) {
            return it->second(statement);
        }
        return PrepareResult::PREPARE_UNRECOGNIZED_STATEMENT;
    }
};

StatementProcessor::StatementProcessor(Table& db_table) : pimpl(new Impl(db_table)) {}
StatementProcessor::~StatementProcessor() { delete pimpl; }

PrepareResult StatementProcessor::execute(const std::string& statement) {
    return pimpl->execute(statement);
}
