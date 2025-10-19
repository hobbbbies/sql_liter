#include <iostream>
#include <string>
#include <cstdlib>
#include <unordered_map>
#include <functional>
#include <sstream>
#include <vector>
#include <cstdint>
#include <cstring>
#include <stdexcept>
#define COLUMN_USERNAME_SIZE 32
#define COLUMN_EMAIL_SIZE 255
#define TABLE_MAX_PAGES 100

enum class MetaCommandResult {
    META_COMMAND_SUCCESS,
    META_COMMAND_UNRECOGNIZED_COMMAND
};

enum class PrepareResult { 
    PREPARE_SUCCESS, 
    PREPARE_UNRECOGNIZED_STATEMENT,
    PREPARE_SYNTAX_ERROR
};

enum class ExecuteResult { 
    EXECUTE_SUCCESS, 
    EXECUTE_FAILURE
};

std::vector<std::string> tokenize(const std::string& str) {
    std::istringstream iss(str);
    std::vector<std::string> tokens;
    std::string token;
    
    while (iss >> token) {
        tokens.push_back(token);
    }
    
    return tokens;
}

class Row {
private:
    uint32_t id;
    char username[COLUMN_USERNAME_SIZE];
    char email[COLUMN_EMAIL_SIZE];

    static constexpr uint32_t ID_SIZE = sizeof(id);
    static constexpr uint32_t USERNAME_SIZE = sizeof(username);
    static constexpr uint32_t EMAIL_SIZE = sizeof(email);

    static constexpr uint32_t ID_OFFSET = 0;
    static constexpr uint32_t USERNAME_OFFSET = ID_OFFSET + ID_SIZE;
    static constexpr uint32_t EMAIL_OFFSET = USERNAME_OFFSET + USERNAME_SIZE;
    static constexpr uint32_t ROW_SIZE = ID_SIZE + USERNAME_SIZE + EMAIL_SIZE;
public:
    // Constructor that takes id, username, and email
    Row(uint32_t rowId, const std::string& user, const std::string& emailAddr) : id(rowId) {
        // Copy strings to char arrays, ensuring null termination
        std::strncpy(username, user.c_str(), COLUMN_USERNAME_SIZE - 1);
        username[COLUMN_USERNAME_SIZE - 1] = '\0';
        
        std::strncpy(email, emailAddr.c_str(), COLUMN_EMAIL_SIZE - 1);
        email[COLUMN_EMAIL_SIZE - 1] = '\0';
    }
    
    // Default constructor
    Row() : id(0) {
        username[0] = '\0';
        email[0] = '\0';
    }

    void serialize(void* destination) const {
        std::memcpy(static_cast<char*>(destination) + ID_OFFSET, &id, ID_SIZE);
        std::memcpy(static_cast<char*>(destination) + USERNAME_OFFSET, username, USERNAME_SIZE);
        std::memcpy(static_cast<char*>(destination) + EMAIL_OFFSET, email, EMAIL_SIZE);
    }

    static Row deserialize(const void* source) {
        Row row;
        std::memcpy(&row.id, static_cast<const char*>(source) + ID_OFFSET, ID_SIZE);
        std::memcpy(row.username, static_cast<const char*>(source) + USERNAME_OFFSET, USERNAME_SIZE);
        std::memcpy(row.email, static_cast<const char*>(source) + EMAIL_OFFSET, EMAIL_SIZE);
        return row;
    }

    void printRow() const {
        std::cout << "(" << id << ", " << email << ", " << username << ")\n";
    }

    static constexpr uint32_t getRowSize() { return ROW_SIZE; }
    uint32_t getId() const { return id; }
    const char* getUsername() const { return username; }
    const char* getEmail() const { return email; }
};

class Table {
private:
    uint32_t num_rows;
    uint8_t* pages[TABLE_MAX_PAGES];

    static constexpr uint32_t PAGE_SIZE = 4096;
    static constexpr uint32_t ROWS_PER_PAGE = PAGE_SIZE / Row::getRowSize();
public:
    Table() : num_rows(0) {
        // Initialize all page pointers to nullptr
        for (int i = 0; i < TABLE_MAX_PAGES; ++i) {
            pages[i] = nullptr;
        }
    }

    ~Table() {
        for (int i = 0; i < TABLE_MAX_PAGES; ++i) {
            delete[] pages[i]; 
        }
    }

    void* row_slot(uint32_t row_num) {
        uint32_t page_num = row_num / ROWS_PER_PAGE;
        
        // Bounds checking
        if (page_num >= TABLE_MAX_PAGES) {
            throw std::out_of_range("Page number exceeds maximum pages");
        }
        
        uint8_t* page = pages[page_num];
        if (page == nullptr) {
            // Allocate memory only when we try to access page
            page = pages[page_num] = new uint8_t[PAGE_SIZE];
            // Initialize the page to zero
            std::memset(page, 0, PAGE_SIZE);
        }
        
        uint32_t row_offset = row_num % ROWS_PER_PAGE;
        uint32_t byte_offset = row_offset * Row::getRowSize();
        return page + byte_offset;
    }

    uint32_t getNumRows() const { return num_rows; }
    void incrementRows() { ++num_rows; }
    
    // Helper method to insert a row
    void insertRow(const Row& row) {
        void* slot = row_slot(num_rows);
        row.serialize(slot);
        ++num_rows;
    }
    
    // Helper method to get a row
    Row getRow(uint32_t row_num) const {
        if (row_num >= num_rows) {
            throw std::out_of_range("Row number exceeds table size");
        }
        
        uint32_t page_num = row_num / ROWS_PER_PAGE;
        uint32_t row_offset = row_num % ROWS_PER_PAGE;
        uint32_t byte_offset = row_offset * Row::getRowSize();
        
        return Row::deserialize(pages[page_num] + byte_offset);
    }

    EXECUTERESULT

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
    std::unordered_map<std::string, std::function<PrepareResult(const std::string&)>> statements;
    Table& table;
    
public:
    StatementProcessor(Table& db_table) : table(db_table) {
        statements["insert"] = [](const std::string& fullCommand) {
            std::cout << "Full command: " << fullCommand << "\n";
            
            // tokenize the command
            auto tokens = tokenize(fullCommand);
            if (tokens.size() >= 4) { 
                std::cout << "Table: " << tokens[2] << "\n";
            } else {
                return PrepareResult::PREPARE_SYNTAX_ERROR;
            }
            
            return PrepareResult::PREPARE_SUCCESS;
        };
        
        statements["select"] = [](const std::string& fullCommand) {
            std::cout << "This is where we would do a select.\n";
            std::cout << "Full command: " << fullCommand << "\n";
            return PrepareResult::PREPARE_SUCCESS;
        };
    }

    PrepareResult execute(const std::string& statement) {
        // Extract first word for command type
        size_t spacePos = statement.find(' ');
        std::string commandType = (spacePos != std::string::npos) ? 
                                 statement.substr(0, spacePos) : statement;
        
        auto it = statements.find(commandType);
        if (it != statements.end()) {
            return it->second(statement);  // Pass full command as parameter
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
    Table db_table;
    StatementProcessor statementProcessor =  StatementProcessor(db_table);
    
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

