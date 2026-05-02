#include "DocumentBuilder.hpp"
#include <algorithm>

static bool isDelimiter(char c)
{
    return std::isspace(static_cast<unsigned char>(c)) || c == ',' || c == '.' || c == '!' || c == '?' || c == ';' ||
           c == ':' || c == '-' || c == '_' || c == '(' || c == ')' || c == '[' || c == ']' || c == '{' || c == '}' ||
           c == '<' || c == '>' || c == '"' || c == '\'' || c == '/' || c == '\\';
}

std::vector<std::string_view> DocumentBuilder::splitWords(std::string_view text)
{
    std::vector<std::string_view> words;
    size_t start = 0;

    while (start < text.size())
    {
        while (start < text.size() && isDelimiter(text[start]))
        {
            ++start;
        }
        if (start >= text.size())
            break;

        size_t end = start;
        while (end < text.size() && !isDelimiter(text[end]))
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