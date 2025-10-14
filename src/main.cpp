#include <iostream>
// Use a reference to alias std::cout
auto& out = std::cout;
struct CPU
{
    using Byte = unsigned char;
    using Word = unsigned short;
};


int main() {

    out << "Hello, World!" << std::endl;
    out << "Size of CPU::Byte: " << sizeof(CPU::Byte) * 8<< " bits" << std::endl;
    out << "Size of CPU::Word: " << sizeof(CPU::Word) * 8 << " bits" << std::endl;
    return 0;
}