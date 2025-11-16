#include "table.hpp"
#include "pager.hpp"
#include "cursor.hpp"
#include "node.hpp"
#include <cstring>
#include <stdexcept>
#include <iostream>

Table::Table(std::string filename) {
    pager = new Pager(filename);
    rootPageNum = 0;

    // Empty file ?
    if (pager->getNumPages() == 0) {
        uint8_t* node_data = pager->getPage(rootPageNum);
        Node node(node_data);
        node.initializeLeafNode();
        node.setNodeRoot(true);
    }
}

Table::~Table() {     
    pager->flushAllPages(num_rows, Row::getRowSize());
    delete pager;
}

// Returns ptr row to serialize; initializes page if needed

uint8_t* Table::getPageAddress(uint32_t pageNum) const{
    if (pageNum >= TABLE_MAX_PAGES) {
        throw std::out_of_range("Page number exceeds maximum pages");
    }
    return pager->getPage(pageNum);
}

void Table::insertRow(const Row& row) {
    uint8_t* nodeData = getPageAddress(rootPageNum);
    Node node(nodeData);
    
    // should get insertion position
    Cursor cursor(*this, row.getId());
    
    // numCells will include the new node to be inserted ?
    uint32_t numCells = *node.leafNodeNumCells();
    
    if (*node.leafNodeNumCells() >= LEAF_NODE_MAX_CELLS) {
        leafNodeSplitAndInsert(row.getId(), &row, cursor.getCellNum()); 
        return;   
    }
    
    // Check if we're inserting at a position with existing cells
    if (cursor.getCellNum() < numCells) {
        uint32_t keyAtPosition = *node.leafNodeKey(cursor.getCellNum());
        if (keyAtPosition == row.getId()) {
            throw std::invalid_argument("Duplicate key");
        }        
    } 
    node.leafNodeInsert(row.getId(), &row, cursor.getCellNum());    
}

Row Table::getRow(uint32_t key) {    
    Cursor cursor(*this, key);
    
    // Check if the key actually exists
    uint8_t* nodeData = getPageAddress(rootPageNum);
    Node node(nodeData);
    uint32_t numCells = *node.leafNodeNumCells();
    
    // If cursor position is beyond valid cells, key doesn't exist
    if (cursor.getCellNum() >= numCells) {
        throw std::out_of_range("Key not found");
    }
    
    // Check if the key at cursor position matches the requested key
    uint32_t keyAtPosition = *node.leafNodeKey(cursor.getCellNum());
    if (keyAtPosition != key) {
        throw std::out_of_range("Key not found");
    }
    
    void* rowAddress = cursor.cursorSlot();        
    return Row::deserialize(rowAddress);
} 

ExecuteResult Table::execute_insert(const std::vector<std::string> tokens) {
    uint8_t* node_data = getPageAddress(rootPageNum);
    Node node(node_data);
    std::cout << "num cells: " << *node.leafNodeNumCells() << "\n";
    // delete below soon 
    // if (*node.leafNodeNumCells() == LEAF_NODE_MAX_CELLS) {
    //     return ExecuteResult::EXECUTE_TABLE_FULL;
    // }

    if (tokens.size() < 4) {
        return ExecuteResult::EXECUTE_FAILURE;
    }
    try {
        if (!tokens[1].empty() && tokens[1][0] == '-') {
            std::cout << "Error: Row ID cannot be negative\n";
            return ExecuteResult::EXECUTE_FAILURE;
        }
        uint32_t rowNum = static_cast<uint32_t>(std::stoul(tokens[1]));
        std::cout << "row num: " << rowNum << "\n";
        std::cout << "num rows: " << num_rows << "\n";
        std::string username = tokens[2];
        std::string email = tokens[3];

        Row newRow(rowNum, username, email);
        insertRow(newRow);
        return ExecuteResult::EXECUTE_SUCCESS;
    } catch (const std::invalid_argument& e) {
        std::cout << "Error: " << e.what() << "\n";
        return ExecuteResult::EXECUTE_DUPLICATE_KEY;
    } catch (const std::exception& e) {
        std::cout << "Error parsing insert values: " << e.what() << "\n";
        return ExecuteResult::EXECUTE_FAILURE;
    }
}

ExecuteResult Table::execute_select_all() {
    try {
        for (uint32_t i = 0; i < num_rows; ++i) {
            Row row = getRow(i);
            row.printRow();
        }

        if (num_rows == 0) {
            std::cout << "No rows in table.\n";
        }

        return ExecuteResult::EXECUTE_SUCCESS;
    } catch (const std::out_of_range& e) {
        throw std::out_of_range("Failed to retrivew a row.\n");
    } catch (const std::exception& e) {
        std::cout << "Error selecting rows: " << e.what() << "\n";
        return ExecuteResult::EXECUTE_FAILURE;
    }
}

ExecuteResult Table::execute_select(const std::vector<std::string>&) {
    // Not implemented yet
    return ExecuteResult::EXECUTE_SUCCESS;
}

uint32_t Table::getNumRows() const {
    uint8_t* node_data = getPageAddress(rootPageNum);
    Node node(node_data);
    return *node.leafNodeNumCells();
}

void Table::leafNodeSplitAndInsert(uint32_t key, const Row* value, uint32_t cellNumToInsertAt) {
    // left node
    uint8_t* oldNodeData = getPageAddress(rootPageNum);
    Node oldNode(oldNodeData);

    // right node 
    uint32_t newPageNum = getUnusedPageNum();
    uint8_t* newNodeData = getPageAddress(newPageNum);
    Node newNode(newNodeData);
    newNode.initializeLeafNode();

    // copy all cells to vector (holds key value pair)
    std::vector<std::pair<uint32_t, Row>> allCells;

    // fill out vector
    uint32_t numExistingCells = *oldNode.leafNodeNumCells();
    for (uint32_t i = 0; i < numExistingCells; i++) {
        uint32_t node_key = *oldNode.leafNodeKey(i);
        Row node_row = Row::deserialize(oldNode.leafNodeValue(i));
        allCells.emplace_back(node_key, node_row);
    }
    // insert new cell
    allCells.emplace(allCells.begin() + cellNumToInsertAt, key, *value);

    // fill left node
    for (uint32_t i = 0; i < LEAF_NODE_LEFT_SPLIT_COUNT; i++) {
        *oldNode.leafNodeKey(i) = allCells[i].first;
        allCells[i].second.serialize(oldNode.leafNodeValue(i));
    } 

    // fill right node
    for (uint32_t i = 0; i < LEAF_NODE_RIGHT_SPLIT_COUNT; i++) {
        uint32_t globalIndex = LEAF_NODE_LEFT_SPLIT_COUNT + i;
        *newNode.leafNodeKey(i) = allCells[globalIndex].first;
        allCells[globalIndex].second.serialize(newNode.leafNodeValue(i));
    }

    // update cell count of both nodes
    *oldNode.leafNodeNumCells() = LEAF_NODE_LEFT_SPLIT_COUNT;
    *newNode.leafNodeNumCells() = LEAF_NODE_RIGHT_SPLIT_COUNT;

    std::cout << "Old node is root?: " << oldNode.isRootNode() << "\n";
    if (oldNode.isRootNode()) {
        return createNewRoot(newPageNum);
    } else {
        std::cout << "implement updating paretn after splitting\n";
        exit(EXIT_FAILURE);
    }
}

// Creates new root (after allocating and splitting to right node)
// and sets left and right node as children of new root 

// should this be switched to non sequential storage?
void Table::createNewRoot(uint32_t rightChildPageNum) {
    // Get the old root (which will become the left child)
    uint8_t* rootData = getPageAddress(rootPageNum);
    Node root(rootData);
    
    uint8_t* rightChildData = getPageAddress(rightChildPageNum);
    Node rightChild(rightChildData);
    
    // Allocate a new page for the left child
    uint32_t leftChildPageNum = getUnusedPageNum();
    uint8_t* leftChildData = getPageAddress(leftChildPageNum);
    
    // Copy the old root's entire page to the left child
    memcpy(leftChildData, rootData, PAGE_SIZE);
    
    Node leftChild(leftChildData);
    leftChild.setNodeRoot(false);
    
    // Now transform the old root page into an internal node
    root.initializeInternalNode();
    root.setNodeRoot(true);
    *root.internalNodeNumKeys() = 1;
    
    // Set up the internal node structure
    *root.internalNodeChild(0) = leftChildPageNum;
    uint32_t leftChildMaxKey = leftChild.getNodeMaxKey();
    *root.internalNodeKey(0) = leftChildMaxKey;  // ← Fixed: dereference
    *root.internalNodeRightChild() = rightChildPageNum;
    
    // Update parent pointers in both children
    // (You'll need to implement setParentPointer if not already done)
    // leftChild.setParentPointer(rootPageNum);
    // rightChild.setParentPointer(rootPageNum);
}
