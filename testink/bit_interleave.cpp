#include <iostream>
#include <string>
#include <vector>
#include <algorithm>

std::string readInput()
{
  std::string input;
  std::getline(std::cin, input);
  return input;
}

inline void print(std::string str)
{
  std::cout << str << std::endl;
}

inline void print(int i)
{
  std::cout << std::to_string(i) << std::endl;
}

void printVec(std::vector<unsigned int>& str)
{
    //std::cout << std::string(str.begin(), str.end()) << std::endl;
    for(unsigned int i = 0; i < str.size(); i++)
    {
        std::cout << std::to_string(str[i]) << std::endl;
    }
}

void printBit(uint32_t val)
{
    for(int i = 31; i >= 0; i--)
    {
        std::cout << ((val >> i) & 1);
    }
    std::cout << std::endl;
}

// from copilot
void morton_naive(uint32_t x, uint32_t y, uint32_t z, uint32_t& out)
{
    out = 0;
    for(int i = 0; i < 32; i++)
    {
        out |= ((x >> i) & 1) << (3 * i);
        out |= ((y >> i) & 1) << (3 * i + 1);
        out |= ((z >> i) & 1) << (3 * i + 2);
    }
}

void morton_smart(uint32_t x, uint32_t y, uint32_t z, uint32_t& out)
{
    // Make x1 x1 x1, x2 x2 x2, x3 x3 x3, then index 
    // for 4 bit example:
    // x = 0b1010 --> 
}

int main()
{
    //uint32_t test = 0x55555555; // 0101
    //uint32_t test = 0xAAAAAAAA; // 1010
    //printBit(test);

    uint32_t test = 0x00003333;
    uint32_t test2 = test & (test << 1);
    printBit(test);
}