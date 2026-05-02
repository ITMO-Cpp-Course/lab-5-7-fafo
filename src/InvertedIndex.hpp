#pragma once

#include <string>
#include <string_view>
#include <unordered_map>
#include <utility>
#include <vector>

#include "Document.hpp"

class InvertedIndex
{
  public:
    using SearchResult = std::vector<std::pair<int, int>>;

    void addDocument(Document doc);

    void removeDocument(int id);

    SearchResult search(std::string_view word) const;

    const Document* getDocument(int id) const;

  private:
    std::unordered_map<std::string, std::unordered_map<int, int>> index_;

    std::unordered_map<int, Document> documents_;
};