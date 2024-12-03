#pragma once
#include <vector>
#include <string>

std::vector<std::string> listAvailableCOMPorts();
std::string selectCOMPort();
void removePort(std::vector<std::string>& ports, const std::string& portToRemove);
