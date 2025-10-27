#include <gtest/gtest.h>
#include "row.hpp"

class RowTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Setup code here
    }
};

TEST_F(RowTest, ConstructorSetsValues) {
    Row row(1, "testuser", "test@example.com");
    
    EXPECT_EQ(row.getId(), 1);
    EXPECT_STREQ(row.getUsername(), "testuser");
    EXPECT_STREQ(row.getEmail(), "test@example.com");
}

TEST_F(RowTest, DefaultConstructor) {
    Row row;
    
    EXPECT_EQ(row.getId(), 0);
    EXPECT_STREQ(row.getUsername(), "");
    EXPECT_STREQ(row.getEmail(), "");
}

TEST_F(RowTest, SerializeDeserialize) {
    Row original(42, "alice", "alice@test.com");
    
    // Serialize
    char buffer[Row::getRowSize()];
    original.serialize(buffer);
    
    // Deserialize
    Row deserialized = Row::deserialize(buffer);
    
    EXPECT_EQ(deserialized.getId(), 42);
    EXPECT_STREQ(deserialized.getUsername(), "alice");
    EXPECT_STREQ(deserialized.getEmail(), "alice@test.com");
}

TEST_F(RowTest, LongStringsTruncated) {
    std::string longUsername(50, 'x');  // 50 x's
    std::string longEmail(300, 'y');    // 300 y's
    
    Row row(1, longUsername, longEmail);
    
    // Should be truncated to fit the buffer sizes
    EXPECT_LT(strlen(row.getUsername()), 32);  // COLUMN_USERNAME_SIZE
    EXPECT_LT(strlen(row.getEmail()), 255);    // COLUMN_EMAIL_SIZE
}
