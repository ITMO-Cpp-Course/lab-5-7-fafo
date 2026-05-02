#include "DocumentBuilder.hpp"
#include <algorithm>

std::vector<std::string_view> DocumentBuilder::splitWords(std::string_view text)
{
    std::vector<std::string_view> words;
    size_t start = 0;
    while (start < text.size())
    {
        while (start < text.size() && std::isspace(static_cast<unsigned char>(text[start])))
        {
            ++start;
        }
        if (start >= text.size())
            break;

        size_t end = start;
        while (end < text.size() && !std::isspace(static_cast<unsigned char>(text[end])))
        {
            ++end;
        }
        words.emplace_back(text.substr(start, end - start));

        start = end;
    }
    return words;
}

void DocumentBuilder::toLower(std::string& word)
{
    std::transform(word.begin(), word.end(), word.begin(), [](unsigned char c) { return std::tolower(c); });
}
