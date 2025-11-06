#pragma once

#include <cstdint>

// Shared constants
constexpr uint32_t COLUMN_USERNAME_SIZE = 32;
constexpr uint32_t COLUMN_EMAIL_SIZE = 255;
constexpr uint32_t TABLE_MAX_PAGES = 100;
constexpr uint32_t PAGE_SIZE = 4096;

// Derived storage layout constants
// Keep in sync with Row layout (id:uint32_t, username[COLUMN_USERNAME_SIZE], email[COLUMN_EMAIL_SIZE])
constexpr uint32_t ROW_SIZE_BYTES = sizeof(uint32_t) + COLUMN_USERNAME_SIZE + COLUMN_EMAIL_SIZE;
constexpr uint32_t ROWS_PER_PAGE = PAGE_SIZE / ROW_SIZE_BYTES;
constexpr uint32_t TABLE_MAX_ROWS = ROWS_PER_PAGE * TABLE_MAX_PAGES;
