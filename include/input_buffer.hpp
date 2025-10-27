#pragma once

#include <string>
#include <iostream>

class InputBuffer {
private:
    std::string buffer;
    std::istream* input_stream;
public:
    InputBuffer(std::istream* stream = &std::cin) : input_stream(stream) {}

    const std::string& getBuffer() const { return buffer; }
    void readInput();
};
