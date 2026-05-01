#pragma once

#include <string>
#include <string_view>
#include <vector>

#include "Document.hpp"

class DocumentBuilder
{
  public:
    static Document build(std::string_view name, std::string_view text)
    {

        return Document(0, std::string(name), std::string(text));
    }

    static std::vector<std::string_view> splitWords(std::string_view text);

    static void toLower(std::string& word);
};