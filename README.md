# SQL Liter 

A lightweight, from-scratch SQL database implementation in C++ featuring a B+ tree storage engine and interactive REPL interface.

## Background 

SQL Liter is a hands-on exploration of database internals, built entirely from scratch. It's something I built as a means to reach a few different goals: 

  1. #### Learn C++ <br>
       Prior to this project, my experience in C++ was limited to simple terminal executables and basic REPL interfaces. I chose this project as a way to dive into the deep end of C++             development, in a "all or nothing" approach.
     
  3. #### Learn Database Internals <br>
       Databases have always been a "black box" to me, just sucking in my data and spitting it out. I recently picked up the well known guide on data-driven apps, "<i>Designing Data-Intensive Applications</i>" by Martin Kleppmann, and           it really opened my eyes into the implementations of real world databases. I had to try it out myself.
     
  4. #### Sharpening my DSA skills <br>
       At the time of writing this, I just passed my Data Structures & Algorithms class at my university. While there was plently of workload, I wanted to find a more interesting way to           study instead of just reading textbooks. That's when I started reading about B-Trees in databases, and decided that if I could build one of those, the rest of my DSA class would be         a breeze.

## The B-Tree
Not to be confused with a binary tree, a B+ tree is a multi-way search tree designed specifically for storage systems. Each node can store many keys and child pointers, which drastically reduces tree depth.

The defining characteristics of a B+ tree are:

* All data lives in leaf nodes

* Internal nodes only store keys and child pointers

* Leaf nodes are linked together for fast range scans
  
This structure is used almost universally in relational databases, and after implementing one myself, it’s easy to see why.


But why not any other type of tree? Red Black Trees and AVL Trees are both self balancing trees that I covered in my DSA class, so I was curious as to why these aren't popular for database implementations. Turns out theres two big reasons:

#### 1. Access to disk is done in large chunks
* In B Trees, typically each node is stored as a 4KB `page` on the disk. This makes one node capable of storing hundreds of cells of data, and since all data is kept in leaf nodes, accessing one part of the table is very likely to result in caching for future queries, minimzing disk reads.

#### 2. Keep the tree very shallow
* Again, since each node is implemented as an entire page on the disk, and each page can typically hold 4KB, trees are built very wide, with hundreds of data cells per node, but remain shallow, typically only reaching a few levels of depth at in most use cases.

#### B Tree Splitting

This was what I found to be the trickiest part. The main downside of implementing nodes that can hold many keys is that when nodes fill up to their capacity, something interesting happens: ***node splitting***

#### 1. Split leaf node
This is the easier of the two. Splitting a leaf node occurs when inserting a new row, and the leaf node that the row would belong to has reached capacity (in my implementation, that's 13 rows per leaf).

In SQL Liter, the split works like this:

1. Copy all existing cells (key/value pairs) into an in-memory array.
2. Insert the new row into that array at the correct sorted position.
3. Write the left half back into the original leaf node.
4. Write the right half into a newly allocated leaf node.
5. Update leaf sibling pointers so the leaf-level linked list remains correct.
6. Update the parent:
   - If the old leaf was the root, create a new root.
   - Otherwise update the old leaf’s max key in the parent, then insert the new leaf as a new child in the parent.

This keeps all row data in leaf nodes, preserves sorted order, and maintains the B+ tree invariant that internal nodes only act as routing structure.

#### 2. Split internal node
Internal node splitting is more complex because you are redistributing routing information (keys + child pointers), not the actual rows.

An internal node split occurs when inserting a new child into a full internal node. The process (in my implementation) works like this:

1. Copy the internal node’s keys and child pointers into in-memory arrays, and insert the new child into the correct sorted position (based on the child’s max key).
2. Choose a middle separator key. This key is used to divide the routing information into two nodes.
3. Rewrite the original node as the left side of the split.
4. Create a new internal node and write the right side of the split into it.
5. Update the parent pointers for any children that moved into the new internal node.
6. Propagate upward:
   - If the node being split is the root, create a new root with two children (tree height increases by 1).
   - Otherwise insert the new internal node into the parent, and update the parent’s key for the original node (since its max key changed after the split).

This is the main mechanism that keeps the tree balanced while allowing it to grow as inserts continue.

---

## On-Disk Storage & Paging

SQL Liter stores all B+ tree nodes directly as fixed-size pages on disk.

- Each page is `PAGE_SIZE` bytes (4KB).
- Every leaf/internal node occupies exactly one page.
- Pages are addressed by page number (0, 1, 2, ...).
- Page 0 starts as the root page, and the root can change over time as splits occur.

### Pager design

The `Pager` is responsible for translating "page number" into an in-memory pointer:

- If the page is already cached in memory, it returns the cached pointer.
- If not cached:
  - Allocate a 4KB buffer for that page.
  - Read from disk if the page exists in the file.
  - Otherwise leave it zeroed (this is how new pages get created).

On shutdown, all cached pages are flushed back to disk (written at `pageNum * PAGE_SIZE`). This keeps the file layout simple and makes disk I/O predictable.

This page-based model is why B+ trees work so well for databases: tree traversal naturally becomes "read a small number of 4KB pages" rather than lots of tiny pointer-chasing reads.

---

## Testing & Maintainability (GoogleTest)

To keep the project maintainable as the codebase grew, I used GoogleTest for automated testing.

The tests mainly focus on:

- Insert correctness (keys end up in the right leaf)
- Split correctness (leaf and internal splits preserve ordering + routing)
- Cursor traversal (walking across leaf siblings produces sorted output)
- Basic persistence expectations (pages flush to disk without corruption)

Having tests made it much easier to refactor low-level code (node layout, pager behavior, split logic) without constantly re-testing everything manually through the REPL.


### Supported Operations
```sql
insert <id> <username> <email>
select
.exit    -- Meta-command to exit
.btree   -- Meta-command to visualize B+ tree structure
```

## Build & Test

### Requirements
- C++17 compatible compiler
- CMake 3.14+
- Google Test (automatically fetched)

### Building
```bash
mkdir build && cd build
cmake ..
make
```

### Running
```bash
./sql_liter database.db
```

### Testing
```bash
./sql_liter_tests
```

## Technical Highlights

### Systems Programming Skills
- **Memory Management**: Manual memory allocation and deallocation patterns
- **File I/O**: Direct file operations with proper error handling
- **Data Structures**: Implementation of complex tree structures from scratch
- **Performance**: Efficient algorithms for search, insertion, and storage

### Software Engineering Practices
- **Testing**: Comprehensive unit test suite with Google Test
- **Architecture**: Clean, modular design with clear separation of concerns
- **Documentation**: Well-commented code with clear interfaces
- **Build System**: Modern CMake configuration with proper dependency management

### Database Concepts Demonstrated
- **Storage Engines**: Understanding of how databases store and retrieve data
- **Indexing**: B+ tree implementation showing grasp of database optimization
- **Concurrency**: Foundation for understanding database transaction systems
- **Query Processing**: Parser and execution engine fundamentals

## Future Enhancements
- Internal node support for larger datasets
- Additional SQL operations (UPDATE, DELETE, WHERE clauses)
- Transaction support and concurrency control
- Query optimization and execution planning

## Project Structure
```
sql_liter/
├── src/           # Core implementation
├── include/       # Header files
├── tests/         # Unit tests
├── CMakeLists.txt # Build configuration
└── README.md      # This file
```
