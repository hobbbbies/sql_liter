#include <gtest/gtest.h>
#include <sstream>
#include <iostream>

#include "input_buffer.hpp"
#include "meta_command_processor.hpp"
#include "statement_processor.hpp"
#include "table.hpp"

class InputIntegrationTest : public ::testing::Test {
protected:
    void SetUp() override {
        table = std::make_unique<Table>();
        processor = std::make_unique<StatementProcessor>(*table);
        meta_processor = std::make_unique<MetaCommandProcessor>();
    }
    
    std::unique_ptr<Table> table;
    std::unique_ptr<StatementProcessor> processor;
    std::unique_ptr<MetaCommandProcessor> meta_processor;
};

TEST_F(InputIntegrationTest, InputBufferReadsFromStringStream) {
    std::istringstream input("insert 1 john john@example.com");
    InputBuffer buffer(&input);
    
    buffer.readInput();
    
    EXPECT_EQ(buffer.getBuffer(), "insert 1 john john@example.com");
}

TEST_F(InputIntegrationTest, ProcessInsertCommand) {
    std::istringstream input("insert 1 alice alice@test.com");
    InputBuffer buffer(&input);
    
    buffer.readInput();
    PrepareResult result = processor->execute(buffer.getBuffer());
    
    EXPECT_EQ(result, PrepareResult::PREPARE_SUCCESS);
    EXPECT_EQ(table->getNumRows(), 1);
    
    Row retrieved = table->getRow(0);
    EXPECT_EQ(retrieved.getId(), 1);
    EXPECT_STREQ(retrieved.getUsername(), "alice");
    EXPECT_STREQ(retrieved.getEmail(), "alice@test.com");
}

TEST_F(InputIntegrationTest, ProcessSelectCommand) {
    // First insert some data
    table->insertRow(Row(1, "bob", "bob@test.com"));
    table->insertRow(Row(2, "charlie", "charlie@test.com"));
    
    std::istringstream input("select");
    InputBuffer buffer(&input);
    
    buffer.readInput();
    PrepareResult result = processor->execute(buffer.getBuffer());
    
    EXPECT_EQ(result, PrepareResult::PREPARE_SUCCESS);
}

TEST_F(InputIntegrationTest, ProcessInvalidCommand) {
    std::istringstream input("invalid_command");
    InputBuffer buffer(&input);
    
    buffer.readInput();
    PrepareResult result = processor->execute(buffer.getBuffer());
    
    EXPECT_EQ(result, PrepareResult::PREPARE_UNRECOGNIZED_STATEMENT);
}

TEST_F(InputIntegrationTest, ProcessMetaCommand) {
    std::istringstream input(".help");
    InputBuffer buffer(&input);
    
    buffer.readInput();
    
    // Check if it's a meta command
    EXPECT_EQ(buffer.getBuffer()[0], '.');
    
    MetaCommandResult result = meta_processor->execute(buffer.getBuffer());
    EXPECT_EQ(result, MetaCommandResult::META_COMMAND_SUCCESS);
}

TEST_F(InputIntegrationTest, ProcessMultipleCommands) {
    // Test a sequence of commands
    std::vector<std::string> commands = {
        "insert 1 user1 user1@test.com",
        "insert 2 user2 user2@test.com", 
        "select"
    };
    
    for (const auto& cmd : commands) {
        std::istringstream input(cmd);
        InputBuffer buffer(&input);
        buffer.readInput();
        
        PrepareResult result = processor->execute(buffer.getBuffer());
        EXPECT_EQ(result, PrepareResult::PREPARE_SUCCESS);
    }
    
    EXPECT_EQ(table->getNumRows(), 2);
}

TEST_F(InputIntegrationTest, PrintsErrorOnFullTable) {
    // 14 is the number of rows per page
    ExecuteResult loop_result = ExecuteResult::EXECUTE_SUCCESS;
    for (int i = 0; i < TABLE_MAX_PAGES*14; i++) {
       try {
        table->insertRow(Row(i, "bob", "bob@test.com"));
       } catch (const std::out_of_range& e) {
        loop_result = ExecuteResult::EXECUTE_FAILURE;
       }
    }
    
    // Test table->execute_insert directly
    std::vector<std::string> tokens = {"insert", "999", "newuser", "newuser@test.com"};
    ExecuteResult result = table->execute_insert(tokens);
        
    EXPECT_EQ(result, ExecuteResult::EXECUTE_TABLE_FULL);
    EXPECT_EQ(loop_result, ExecuteResult::EXECUTE_SUCCESS);
}

TEST_F(InputIntegrationTest, AllowsStringsAtMaxLen) {
    std::string long_username(32, 'x');
    std::string long_email(255, 'y');
    std::string full_command = "insert 1 " + long_username + " " + long_email;
    PrepareResult result = processor->execute(full_command);
    EXPECT_EQ(result, PrepareResult::PREPARE_SUCCESS);
}

// rownum is unsigend so this never fails dumbass
TEST_F(InputIntegrationTest, FailOnNegativeId) {
    std::string full_command = "insert -1 stefan vitanov";
    PrepareResult result = processor->execute(full_command);
    std::cout << "result: " << static_cast<int>(result) << "\n";
    EXPECT_EQ(result, PrepareResult::PREPARE_INTERNAL_FAILURE);
}

TEST_F(InputIntegrationTest, KeepsDataAfterClosingConnection) {
    std::string full_command = "insert 1 stefan vitanov";
    PrepareResult result = processor->execute(full_command);
    EXPECT_EQ(result, PrepareResult::PREPARE_SUCCESS);

    
}
