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


## Key Features


### Disk Storage 

### 🌳 Storage Engine
- **B+ Tree Implementation**: Efficient indexing with automatic node splitting and balancing
- **Page-based Storage**: 4KB page management system with disk persistence  
- **Memory Management**: Custom pager with intelligent caching and flush mechanisms
- **Data Persistence**: Everything gets saved to disk, just like a real database!

### 🔍 Query Processing
- **SQL Parser**: Custom tokenizer and statement processor for INSERT and SELECT
- **Interactive REPL**: Type commands and see results instantly
- **Type System**: Structured row format with serialization/deserialization
- **Smart Error Handling**: Helpful error messages when things go wrong

### ⚡ Architecture Highlights
- **Clean Design**: Modular components that work together seamlessly
- **Cursor Navigation**: Efficient tree traversal and positioning system
- **Extensible**: Well-structured foundation ready for more SQL features

## Technical Implementation

### Core Components
- **Table**: High-level database table interface with row operations
- **Node**: B+ tree node management (leaf and internal nodes)
- **Cursor**: Tree navigation and positioning system
- **Pager**: Page-level storage and memory management
- **Row**: Data serialization and type management

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
