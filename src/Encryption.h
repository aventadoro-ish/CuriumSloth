#pragma once

#include <string>

// std::string XORencrypt(std::string toEncrypt, char c);
void* XORencrypt(void* toEncrypt, size_t len, char c);
std::string XORdecrypt(std::string toDecrypt, char a);