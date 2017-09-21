#pragma once 
#include <string>
#include <vector>

std::string_view stringUntil(const std::string_view& from, const char c, size_t offset = 0);
std::vector<std::string_view> ParseCommand(const std::string_view& cmd);