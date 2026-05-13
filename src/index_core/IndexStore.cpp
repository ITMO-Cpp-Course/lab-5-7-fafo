#include "IndexStore.hpp"
#include "DocumentBuilder.hpp"

VoidResult IndexStore::addDocument(Document doc)
{
    int id = doc.id();
    if (index_.getDocument(id) != nullptr)
    {
        return std::unexpected(ErrorCode::kDuplicateDocumentId);
    }
    index_.addDocument(std::move(doc));
    return {};
}

VoidResult IndexStore::removeDocument(int id)
{
    if (index_.getDocument(id) == nullptr)
    {
        return std::unexpected(ErrorCode::kDocumentNotFound);
    }
    index_.removeDocument(id);
    return {};
}

Result<InvertedIndex::SearchResult> IndexStore::search(std::string_view word) const
{
    if (word.empty())
    {
        return std::unexpected(ErrorCode::kEmptyWord);
    }
    return index_.search(word);
}

const Document* IndexStore::getDocument(int id) const
{
    return index_.getDocument(id);
}

Result<UpdateTransaction> IndexStore::beginTransaction()
{
    InvertedIndex snapshot = index_;
    return UpdateTransaction(*this, std::move(snapshot));
}
