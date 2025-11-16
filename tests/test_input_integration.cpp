#include <gtest/gtest.h>
#include <sstream>
#include <iostream>
#include <cstdio>

#include "input_buffer.hpp"
#include "meta_command_processor.hpp"
#include "statement_processor.hpp"
#include "table.hpp"

class InputIntegrationTest : public ::testing::Test {
protected:
    void SetUp() override {
        test_filename = "test_integration.db";
        std::remove(test_filename.c_str());  // Clean up any existing file
        
        table = std::make_unique<Table>(test_filename);
        processor = std::make_unique<StatementProcessor>(*table);
        meta_processor = std::make_unique<MetaCommandProcessor>();
    }
    
    void TearDown() override {
        // Only reset if they haven't been reset already
        if (processor) {
            processor.reset();
        }
        if (table) {
            std::cout << "trying...\n";
             table.reset();
            std::cout << "tried\n";
        }
        if (meta_processor) {
            meta_processor.reset();
        }
        std::remove(test_filename.c_str());
    }
    
    std::string test_filename;
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
    
    Row retrieved = table->getRow(1);
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
    
    MetaCommandResult result = meta_processor->execute(buffer.getBuffer(), table.get());
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

// TEST_F(InputIntegrationTest, PrintsErrorOnFullTable) {
//     // 14 is the number of rows per page
//     ExecuteResult loop_result = ExecuteResult::EXECUTE_SUCCESS;
//     for (int i = 0; i < TABLE_MAX_PAGES * 14; i++) {
//        try {
//         table->insertRow(Row(i, "bob", "bob@test.com"));
//        } catch (const std::out_of_range& e) {
//         loop_result = ExecuteResult::EXECUTE_FAILURE;
//        }
//     }
    
//     // Test table->execute_insert directly
//     std::vector<std::string> tokens = {"insert", "999", "newuser", "newuser@test.com"};
//     ExecuteResult result = table->execute_insert(tokens);
        
//     EXPECT_EQ(result, ExecuteResult::EXECUTE_TABLE_FULL);
//     EXPECT_EQ(loop_result, ExecuteResult::EXECUTE_SUCCESS);
// }

TEST_F(InputIntegrationTest, AllowsStringsAtMaxLen) {
    std::string long_username(32, 'x');
    std::string long_email(255, 'y');
    std::string full_command = "insert 1 " + long_username + " " + long_email;
    PrepareResult result = processor->execute(full_command);
    EXPECT_EQ(result, PrepareResult::PREPARE_SUCCESS);
}

TEST_F(InputIntegrationTest, FailOnNegativeId) {
    std::string full_command = "insert -1 stefan stefan@example.com";
    PrepareResult result = processor->execute(full_command);
    // Should fail because -1 can't be converted to unsigned int
    EXPECT_EQ(result, PrepareResult::PREPARE_INTERNAL_FAILURE);
}

// THIS IS WHY YOU NEED unique_ptr!
TEST_F(InputIntegrationTest, KeepsDataAfterClosingConnection) {
    // Insert data
    std::string first_command = "insert 1 stefan stefan@example.com";
    std::string second_command = "insert 2 other other@example.com";
    PrepareResult first = processor->execute(first_command);
    PrepareResult second = processor->execute(second_command);
    EXPECT_EQ(first, PrepareResult::PREPARE_SUCCESS);
    EXPECT_EQ(second, PrepareResult::PREPARE_SUCCESS);
    EXPECT_EQ(table->getNumRows(), 2);
    Row retrieved_old = table->getRow(1);
    EXPECT_EQ(retrieved_old.getId(), 1);
    EXPECT_STREQ(retrieved_old.getUsername(), "stefan");
    EXPECT_STREQ(retrieved_old.getEmail(), "stefan@example.com");
    // *** CLOSE THE CONNECTION *** (destroy table and processor)
    processor.reset();  // Delete processor
    table.reset();      // Delete table - closes file, flushes data

    // *** REOPEN THE CONNECTION *** (create new table with same file)
    table = std::make_unique<Table>(test_filename);
    processor = std::make_unique<StatementProcessor>(*table);
    // *** VERIFY DATA PERSISTED ***
    EXPECT_EQ(table->getNumRows(), 2);  // Data should still be there!
    Row retrieved = table->getRow(1);
    EXPECT_EQ(retrieved.getId(), 1);
    EXPECT_STREQ(retrieved.getUsername(), "stefan");
    EXPECT_STREQ(retrieved.getEmail(), "stefan@example.com");
}
