#include "input_buffer.hpp"

#include <iostream>
#include <cstdlib>

void InputBuffer::readInput() {
    std::string line;
    std::getline(*input_stream, line);
    if (input_stream->fail()) {
        std::cout << "Error reading input\n";
        std::exit(EXIT_FAILURE);
    }
    buffer = line;
}
