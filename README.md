# SQL Liter

A lightweight, from-scratch SQL database implementation in C++ featuring a B+ tree storage engine and REPL interface.

## Overview

SQL Liter is a educational database project that demonstrates core database concepts including storage management, indexing, and query processing. Built entirely from scratch without external database libraries, it showcases fundamental computer science principles in action.

## Key Features

### Storage Engine
- **B+ Tree Implementation**: Efficient indexing with automatic node splitting and balancing
- **Page-based Storage**: 4KB page management system with disk persistence
- **Memory Management**: Custom pager with LRU-style caching and flush mechanisms
- **ACID Properties**: Atomic operations with proper error handling

### Query Processing
- **SQL Parser**: Tokenizer and statement processor for INSERT and SELECT operations
- **REPL Interface**: Interactive command-line interface with meta-commands
- **Type System**: Structured row format with serialization/deserialization
- **Error Handling**: Comprehensive validation and exception management

### Architecture Highlights
- **Modular Design**: Clean separation between storage, indexing, and query layers
- **Cursor-based Navigation**: Efficient tree traversal and positioning system
- **Extensible Framework**: Well-structured codebase ready for additional SQL features

## Technical Implementation

### Core Components
- **Table**: High-level database table interface with row operations
- **Node**: B+ tree node management (leaf and internal nodes)
- **Cursor**: Tree navigation and positioning system
- **Pager**: Page-level storage and memory management
- **Row**: Data serialization and type management

### Supported Operations
```sql
INSERT INTO table VALUES (id, username, email);
SELECT * FROM table;
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

## Technical Highlights for Recruiters

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

This project demonstrates deep understanding of database internals, systems programming, and software architecture principles essential for backend engineering roles.
