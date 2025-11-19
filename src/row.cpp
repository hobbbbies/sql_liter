#include "row.hpp"

#include <cstring>
#include <iostream>

Row::Row(uint32_t rowId, const std::string& user, const std::string& emailAddr) : id(rowId) {
    std::strncpy(username, user.c_str(), COLUMN_USERNAME_SIZE - 1);
    username[COLUMN_USERNAME_SIZE - 1] = '\0';

    std::strncpy(email, emailAddr.c_str(), COLUMN_EMAIL_SIZE - 1);
    email[COLUMN_EMAIL_SIZE - 1] = '\0';
}

Row::Row() : id(0) {
    username[0] = '\0';
    email[0] = '\0';
}

void Row::serialize(void* destination) const {
    std::memcpy(static_cast<char*>(destination) + ID_OFFSET, &id, ID_SIZE);
    std::memcpy(static_cast<char*>(destination) + USERNAME_OFFSET, username, USERNAME_SIZE);
    std::memcpy(static_cast<char*>(destination) + EMAIL_OFFSET, email, EMAIL_SIZE);
}

Row Row::deserialize(const void* source) {
    Row row;
    std::memcpy(&row.id, static_cast<const char*>(source) + ID_OFFSET, ID_SIZE);
    std::memcpy(row.username, static_cast<const char*>(source) + USERNAME_OFFSET, USERNAME_SIZE);
    std::memcpy(row.email, static_cast<const char*>(source) + EMAIL_OFFSET, EMAIL_SIZE);
    return row;
}

void Row::printRow() const {
    std::cout << "(" << id << ", " << email << ", " << username << ")\n";
}

const char* Row::getUsername() const {
    return username;
}

