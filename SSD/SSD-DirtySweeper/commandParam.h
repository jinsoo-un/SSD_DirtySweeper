#pragma once
#include <string>

using std::string;

struct commandParams
{
    string op;
    int addr;
    string value;
    int size;
    int argCount;
};