#include "InvertedIndex.hpp"
#include "DocumentBuilder.hpp"
#include <string>

void InvertedIndex::addDocument(Document doc)
{
    int id = doc.id();
    if (documents_.find(id) != documents_.end())
    {
        removeDocument(id);
    }
    documents_[id] = std::move(doc);
    const std::string& text = documents_[id].text();

    auto words = DocumentBuilder::splitWords(text);

    for (std::string_view wordView : words)
    {
        std::string word(wordView);
        DocumentBuilder::toLower(word);

        ++index_[std::move(word)][id];
    }
}

void InvertedIndex::removeDocument(int id)
{

    documents_.erase(id);
    for (auto itWord = index_.begin(); itWord != index_.end();)
    {
        auto& docMap = itWord->second;
        docMap.erase(id);
        if (docMap.empty())
        {
            itWord = index_.erase(itWord);
        }
        else
        {
            ++itWord;
        }
    }
}

InvertedIndex::SearchResult InvertedIndex::search(std::string_view word) const
{

    std::string lowerWord(word);
    DocumentBuilder::toLower(lowerWord);

    SearchResult result;

    auto it = index_.find(lowerWord);

    if (it != index_.end())
    {

        const auto& docMap = it->second;

        result.reserve(docMap.size());
        for (const auto& [docId, count] : docMap)
        {
            result.emplace_back(docId, count);
        }
    }
    return result;
}

const Document* InvertedIndex::getDocument(int id) const
{
    auto it = documents_.find(id);
    if (it != documents_.end())
    {
        return &it->second;
    }
    return nullptr;
}