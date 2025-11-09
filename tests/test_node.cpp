#include <gtest/gtest.h>
#include "node.hpp"
#include <cstdio>

class NodeTest : public ::testing::Test {
protected:
    void SetUp() override {
        test_filename = "test_node.db";
        std::remove(test_filename.c_str());  // Clean up any existing file
        node = std::make_unique<Node>(test_filename);
    }
    
    void TearDown() override {
        node.reset();
        std::remove(test_filename.c_str());
    }
    
    std::string test_filename;
    std::unique_ptr<Node> node;
};