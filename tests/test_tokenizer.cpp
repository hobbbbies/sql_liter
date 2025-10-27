#include <gtest/gtest.h>
#include "tokenizer.hpp"

TEST(TokenizerTest, EmptyString) {
    auto tokens = tokenize("");
    EXPECT_TRUE(tokens.empty());
}

TEST(TokenizerTest, SingleWord) {
    auto tokens = tokenize("hello");
    ASSERT_EQ(tokens.size(), 1);
    EXPECT_EQ(tokens[0], "hello");
}

TEST(TokenizerTest, MultipleWords) {
    auto tokens = tokenize("insert 1 john doe");
    ASSERT_EQ(tokens.size(), 4);
    EXPECT_EQ(tokens[0], "insert");
    EXPECT_EQ(tokens[1], "1");
    EXPECT_EQ(tokens[2], "john");
    EXPECT_EQ(tokens[3], "doe");
}

TEST(TokenizerTest, ExtraSpaces) {
    auto tokens = tokenize("  hello   world  ");
    ASSERT_EQ(tokens.size(), 2);
    EXPECT_EQ(tokens[0], "hello");
    EXPECT_EQ(tokens[1], "world");
}

TEST(TokenizerTest, TabsAndSpaces) {
    auto tokens = tokenize("select\t*\tfrom\ttable");
    ASSERT_EQ(tokens.size(), 4);
    EXPECT_EQ(tokens[0], "select");
    EXPECT_EQ(tokens[1], "*");
    EXPECT_EQ(tokens[2], "from");
    EXPECT_EQ(tokens[3], "table");
}
