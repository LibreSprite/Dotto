#pragma once

#include <cstdint>
#include <string>
#include <vector>

std::string trim(const std::string&);
std::vector<std::string> split(const std::string& line, const std::string& del);
std::string join(const std::string&);
uint32_t popUTF8(const std::string&, std::size_t& it);
std::string readTextFile(const std::string& path);
std::vector<std::byte> readBinaryFile(const std::string& path);
std::vector<std::string> parseCommandLine(const std::string& cmd);
