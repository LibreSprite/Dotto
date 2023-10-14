#include "String.hpp"

#include <cstddef>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>

std::vector<std::string> split(const std::string& line, const std::string& del) {
    std::vector<std::string> acc;
    int end = -del.size();
    do {
        int start = end + del.size();
        end = line.find(del, start);
        acc.emplace_back(line.substr(start, end - start));
    } while (end != std::string::npos);
    return acc;
}

std::vector<std::string> parseCommandLine(const std::string& line) {
    std::vector<std::string> parts;
    std::string acc;
    bool escape = false;
    bool quote = false;
    for (auto ch : line) {
	if (escape) {
	    escape = false;
	    if (ch == 't')
		ch = '\t';
	    else if (ch == 'n')
		ch = '\n';
	    else if (ch == '\\')
		ch = '\\';
	    else
		continue;
	    acc.push_back(ch);
	    continue;
	} else if (ch == '\\') {
	    escape = true;
	    continue;
	}

	if (acc.empty()) {
	    if (ch == '"') {
		quote = !quote;
	    } else if (ch > ' ') {
		acc.push_back(ch);
	    }
	    continue;
	}

	if ((ch <= ' ' && !quote) || (ch == '"' && quote)) {
	    if (!acc.empty()) {
		parts.push_back(acc);
		acc.clear();
	    }
	    quote = false;
	    continue;
	}

	acc.push_back(ch);
    }
    if (!acc.empty()) {
	parts.push_back(acc);
	acc.clear();
    }
    return parts;
}

std::string trim(const std::string& input) {
    std::size_t start = 0;
    std::size_t end = input.size();
    for (auto c : input) {
        if (c > ' ')
            break;
        start++;
    }

    if (start == end)
        return "";

    while (input[end - 1] <= ' ')
        end--;

    return input.substr(start, end - start);
}

uint32_t popUTF8(const std::string& str, std::size_t& it) {
    auto max = str.size();
    if (it >= max)
        return 0;
    uint32_t acc = 0;
    uint32_t b;
    do {
        b = str[it++];
        acc <<= 7;
        acc |= b & 0x7F;
    } while ((b & 0x80) && it < max);
    return acc;
}

std::string readTextFile(const std::string& path) {
    std::ifstream stream;
    stream.open(path);
    std::stringstream strStream;
    strStream << stream.rdbuf();
    return strStream.str();
}

std::vector<std::byte> readBinaryFile(const std::string& path) {
    std::ifstream stream(path, std::ifstream::binary);

    stream.seekg(0, stream.end);
    auto size = stream.tellg();
    stream.seekg(0, stream.beg);

    std::vector<std::byte> data;
    if (size > 0) {
	data.resize(size);
	stream.read(reinterpret_cast<char*>(data.data()), size);
    }
    return data;
}
