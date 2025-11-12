#include "cursor.hpp"
#include "node.hpp"

Cursor::Cursor(Table& table, uint32_t key) : table(table), endOfTable(false) {
    // Start at root page
    uint32_t rootPageNum = table.getRootPageNum();
    uint8_t* nodeData = table.getPageAddress(rootPageNum);
    Node node(nodeData);     

    // find key in node - Either leaf or internal node
    NodeType nodeType = node.getNodeType();
    if (nodeType == NodeType::NODE_LEAF) {
        leafNodeFind(key, rootPageNum);
    } else {
        // stub for internal nodes
    }
}


Cursor::~Cursor() {}

// sets cursor to correct cell within given row (pageNum)
void Cursor::leafNodeFind(uint32_t key, uint32_t pageNum) {
    this->pageNum = pageNum;

    uint8_t* nodeData = table.getPageAddress(pageNum);
    Node node(nodeData);
    
    uint32_t minIndex = 0;
    uint32_t onePastMaxIndex = *node.leafNodeNumCells();
    while (minIndex < onePastMaxIndex) { 
        uint32_t currentIndex = (minIndex + onePastMaxIndex) / 2;
        uint32_t currentKey = *node.leafNodeKey(currentIndex);
        if (currentKey == key) {
            this->cellNum = currentIndex;
            return;
        } else if (currentKey < key) {
            minIndex = currentIndex + 1;
        } else {
            onePastMaxIndex = currentIndex;
        }
    }

    // if key is not found, cellNum will point to its insertion position; follows BST property
    this->cellNum = minIndex;
}

void Cursor::leafNodeSplitAndInsert(uint32_t key, const Row* value) {
    // left node
    uint8_t* oldNodeData = table.getPageAddress(pageNum);
    Node oldNode(oldNodeData);

    // right node 
    uint32_t newPageNum = table.getUnusedPageNum();
    uint8_t* newNodeData = table.getPageAddress(newPageNum);
    Node newNode(newNodeData);
    newNode.initializeLeafNode();

    for (uint32_t i = LEAF_NODE_MAX_CELLS; i > 0; i--) {
        Node* temp;
        // if else block to find which node to use (left or right)
        if (i >= LEAF_NODE_LEFT_SPLIT_COUNT) {
            temp = &oldNode;
        } else {
            temp = &newNode;
        }

        uint32_t localIndex = i % LEAF_NODE_LEFT_SPLIT_COUNT;
        void* destinationCell = temp->leafNodeCell(localIndex);

        if (i == cellNum) { // insertion position for i
            temp->leafNodeInsert(key, value, localIndex);
        } 
        // else if index is greater than insertion pos, 
        // move node up one position to make space for new cell
        else if (i > cellNum) {
            uint32_t currentKey = *temp->leafNodeKey(i-1);
            const void* currentValue = temp->leafNodeValue(i-1);
            temp->leafNodeInsert(currentKey, &Row::deserialize(currentValue), localIndex);
        } 
        // else just copy the node over 
        // may be redundant if temp == oldNode
        else {
            uint32_t currentKey = *temp->leafNodeKey(i);
            const void* currentValue = temp->leafNodeValue(i);
            temp->leafNodeInsert(currentKey, &Row::deserialize(currentValue), localIndex);
        }

        // update cell count of both nodes
        *oldNode.leafNodeNumCells() = LEAF_NODE_LEFT_SPLIT_COUNT;
        *newNode.leafNodeNumCells() = LEAF_NODE_RIGHT_SPLIT_COUNT;
    }
}

// Gives pointer in memory to row 
void* Cursor::cursorSlot() {
    uint8_t* nodeData = table.getPageAddress(pageNum); 
    Node node(nodeData);
    return node.leafNodeValue(cellNum);
}

void Cursor::cursorAdvance() {
    cellNum += 1;
    uint8_t* nodeData = table.getPageAddress(pageNum);
    Node node(nodeData);
    uint32_t numCells = *node.leafNodeNumCells();
    endOfTable = cellNum >= numCells;
}

