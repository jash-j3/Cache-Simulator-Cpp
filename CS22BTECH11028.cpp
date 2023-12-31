#include <iostream>
#include <fstream>
#include <vector>
#include <iterator>
#include <sstream>
#include <cmath>
#include <tuple>
#include <iomanip>
using namespace std;
#define MAX_LINES 10000
#define MAX_ASSOCIATIVITY 16
int cacheI = 0;
int replacementP[MAX_LINES][MAX_ASSOCIATIVITY] = {{0}};
int consequtiveBit[MAX_LINES][MAX_ASSOCIATIVITY] = {{0}};
long data1[MAX_LINES][MAX_ASSOCIATIVITY];
int position = 1;
bool hit = false;
int totalHits = 0, totalMisses = 0;

void handleMissRead(int associativity, string replacement, const std::pair<char, std::string> &tuple, int tag);
void handleMissWrite(int associativity, string replacement, string writeback, const std::pair<char, std::string> &tuple, int tag);

void handleRead(int associativity, string replacement, const std::pair<char, std::string> &tuple, int tag)
{
    for (size_t i = 0; i < associativity; i++)
    {
        int digit = data1[cacheI][i];
        int emptySlot = consequtiveBit[cacheI][i];
        if (const auto [replacementType, indexValue] = std::tie(replacement, replacementP[cacheI][i]); emptySlot == 1 && digit == tag)
        {
            totalHits++;
            std::cout << "Address: " << tuple.second << ", "
                      << "Set: 0x" << std::setw(8) << std::setfill('0') << std::hex << cacheI << ", "
                      << "Hit, "
                      << "Tag: 0x" << std::setw(8) << std::setfill('0') << std::hex << tag << std::endl;
            hit = true;
            if (replacementType == "LRU" || (replacementType == "FIFO" && indexValue == 0))
                indexValue = 1;
            break;
        }
    }
    if (!hit)
        handleMissRead(associativity, replacement, tuple, tag);
}

void handleWrite(int associativity, string replacement, string writeback, const std::pair<char, std::string> &tuple, int tag)
{
    for (size_t i = 0; i < associativity; i++)
    {
        bool match = false;
        int tagValue = data1[cacheI][i];
        if (tagValue == tag)
            match = true;
        int emptySlot = consequtiveBit[cacheI][i];
        if (match)
        {
            if (emptySlot == 1)
            {
                hit = true;
                totalHits++;
                std::cout << "Address: " << tuple.second << ", "
                          << "Set: 0x" << std::setw(8) << std::setfill('0') << std::hex << cacheI << ", "
                          << "Hit, "
                          << "Tag: 0x" << std::setw(8) << std::setfill('0') << std::hex << tag << std::endl;
                if (replacement == "LRU" || (replacement == "FIFO" && replacementP[cacheI][i] == 0))
                    replacementP[cacheI][i] = position;
                break;
            }
        }
    }
    if (!hit)
        handleMissWrite(associativity, replacement, writeback, tuple, tag);
}

void handleMissRead(int associativity, string replacement, const std::pair<char, std::string> &tuple, int tag)
{
    totalMisses++;
    bool check = false;
    int replaement = -1;
    std::cout << "Address: " << tuple.second << ", "
              << "Set: 0x" << std::setw(8) << std::setfill('0') << std::hex << cacheI << ", "
              << "Miss, "
              << "Tag: 0x" << std::setw(8) << std::setfill('0') << std::hex << tag << std::endl;
    for (size_t i = 0; i < associativity; i++)
    {
        int emptySlot = consequtiveBit[cacheI][i];
        if (emptySlot == 0)
        {
            check = true;
            replaement = -1;
            data1[cacheI][i] = tag;
            consequtiveBit[cacheI][i] = 1;
            break;
        }
        if ((replacement == "LRU" || replacement == "FIFO") && (replaement == -1 || replacementP[cacheI][i] < replacementP[cacheI][replaement]))
            replaement = i;
    }
    if (!check)
    {
        data1[cacheI][replaement] = tag;
        replacementP[cacheI][replaement] = (replacement == "LRU" || (replacement == "FIFO" && replacementP[cacheI][replaement] == 0)) ? position : replacementP[cacheI][replaement];
    }
}

void handleMissWrite(int associativity, string replacement, string writeback, const std::pair<char, std::string> &tuple, int tag)
{
    totalMisses++;
    int check = 0;
    int replaement = -1;
    std::cout << "Address: " << tuple.second << ", "
              << "Set: 0x" << std::setw(8) << std::setfill('0') << std::hex << cacheI << ", "
              << "Miss, "
              << "Tag: 0x" << std::setw(8) << std::setfill('0') << std::hex << tag << std::endl;
    if (writeback != "WT")
    {
        for (size_t i = 0; i < associativity; i++)
        {
            int emptySlot = consequtiveBit[cacheI][i];
            if (emptySlot == 0)
            {
                check = true;
                replaement = -1;
                data1[cacheI][i] = tag;
                consequtiveBit[cacheI][i] = 1;
                break;
            }
            if ((replacement == "LRU" || replacement == "FIFO") &&
                (replaement == -1 || replacementP[cacheI][i] < replacementP[cacheI][replaement]))
                replaement = i;
        }
        if (!check)
        {
            data1[cacheI][replaement] = tag;
            if (replacement == "LRU" || (replacement == "FIFO" && replacementP[cacheI][replaement] == 0))
                replacementP[cacheI][replaement] = position;
        }
    }
}

int main()
{

    std::ifstream config_file("cache.config");
    std::vector<std::string> cache_config;
    if (!config_file.is_open())
    {
        std::cerr << "Error opening file." << std::endl;
        return 1;
    }
    else
    {
        std::istream_iterator<std::string> file_iterator(config_file);
        std::istream_iterator<std::string> eof;
        cache_config = std::vector<std::string>(file_iterator, eof);
        config_file.close();
    }

    int size = std::stoi(cache_config[0]);
    int block_size = std::stoi(cache_config[1]);
    int associativity1 = std::stoi(cache_config[2]);
    std::istringstream iss1(cache_config[3]);
    std::istringstream iss2(cache_config[4]);
    std::string replacement, writeback;
    iss1 >> replacement;
    iss2 >> writeback;

    std::ifstream input_file("cache.access");

    if (!input_file.is_open())
    {
        std::cerr << "Error opening file." << std::endl;
        return 1;
    }
    std::vector<std::pair<char, std::string>> cache_access;
    std::string line;
    while (getline(input_file, line))
    {
        // if (line=="W: 552")
        // {
        //     line=line;
        // }
        char access_mode;
        char address_buffer[256];
        sscanf(line.c_str(), "%c: %s", &access_mode, address_buffer);
        std::string memory_address(address_buffer);
        cache_access.emplace_back(access_mode, memory_address);
    }
    input_file.close();

    int lines_count, associativity;
    if (associativity1 == 0)
    { // fully associative
        lines_count = size / block_size;
        associativity = lines_count;
    }
    else
    { // set_associative
        lines_count = size / (block_size * associativity1);
        associativity = associativity1;
    }
    int offset_bits = log2(block_size);
    int index_bits = log2(lines_count);
    int tag_bits = 32 - (offset_bits + index_bits);

    for (size_t i = 0; i < cache_access.size(); i++)
    {
        const auto &tuple = cache_access[i];
        hit = false;
        long memoryAddress = std::stoll(tuple.second, nullptr, 16) & 0xFFFFFFFF;
        if (index_bits != 0)
        {
            cacheI = (memoryAddress << tag_bits) & 0xFFFFFFFF;
            cacheI = ((unsigned)cacheI)>>(tag_bits + offset_bits);
        }
        int tag = (memoryAddress >> (offset_bits + index_bits));
        if (tuple.first == 'R')
            handleRead(associativity, replacement, tuple, tag);
        if (tuple.first == 'W')
            handleWrite(associativity, replacement, writeback, tuple, tag);
        position++;
    }
    cout << "Total Hits: " << totalHits << endl;
    cout << "Total Misses: " << totalMisses << endl;
    return 0;
}