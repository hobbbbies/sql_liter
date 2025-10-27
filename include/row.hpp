#pragma once

#include <cstdint>
#include <string>

#include "constants.hpp"

class Row {
private:
    uint32_t id;
    char username[COLUMN_USERNAME_SIZE];
    char email[COLUMN_EMAIL_SIZE];

    static constexpr uint32_t ID_SIZE = sizeof(id);
    static constexpr uint32_t USERNAME_SIZE = sizeof(username);
    static constexpr uint32_t EMAIL_SIZE = sizeof(email);

    static constexpr uint32_t ID_OFFSET = 0;
    static constexpr uint32_t USERNAME_OFFSET = ID_OFFSET + ID_SIZE;
    static constexpr uint32_t EMAIL_OFFSET = USERNAME_OFFSET + USERNAME_SIZE;
    static constexpr uint32_t ROW_SIZE = ID_SIZE + USERNAME_SIZE + EMAIL_SIZE;
public:
    Row(uint32_t rowId, const std::string& user, const std::string& emailAddr);
    Row();

    void serialize(void* destination) const;
    static Row deserialize(const void* source);

    void printRow() const;

    static constexpr uint32_t getRowSize() { return ROW_SIZE; }
    uint32_t getId() const { return id; }
    const char* getUsername() const { return username; }
    const char* getEmail() const { return email; }
};
