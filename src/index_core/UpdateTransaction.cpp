#include "UpdateTransaction.hpp"
#include "IndexStore.hpp"

UpdateTransaction::UpdateTransaction(IndexStore& store, InvertedIndex&& snapshot)
    : store_(&store), tempIndex_(std::move(snapshot)), active_(true), committed_(false)
{
}

UpdateTransaction::~UpdateTransaction()
{
    if (active_ && !committed_)
    {
    }
}

UpdateTransaction::UpdateTransaction(UpdateTransaction&& other) noexcept
    : store_(other.store_), tempIndex_(std::move(other.tempIndex_)), active_(other.active_),
      committed_(other.committed_)
{
    other.active_ = false;
    other.committed_ = false;
    other.store_ = nullptr;
}

UpdateTransaction& UpdateTransaction::operator=(UpdateTransaction&& other) noexcept
{
    if (this != &other)
    {
        store_ = other.store_;
        tempIndex_ = std::move(other.tempIndex_);
        active_ = other.active_;
        committed_ = other.committed_;
        other.active_ = false;
        other.committed_ = false;
        other.store_ = nullptr;
    }
    return *this;
}

VoidResult UpdateTransaction::addDocument(Document doc)
{
    if (!active_)
    {
        return std::unexpected(ErrorCode::kTransactionInactive);
    }
    int id = doc.id();
    if (tempIndex_.getDocument(id) != nullptr)
    {
        return std::unexpected(ErrorCode::kDuplicateDocumentId);
    }
    tempIndex_.addDocument(std::move(doc));
    return {};
}

VoidResult UpdateTransaction::removeDocument(int id)
{
    if (!active_)
    {
        return std::unexpected(ErrorCode::kTransactionInactive);
    }
    if (tempIndex_.getDocument(id) == nullptr)
    {
        return std::unexpected(ErrorCode::kDocumentNotFound);
    }
    tempIndex_.removeDocument(id);
    return {};
}

Result<InvertedIndex::SearchResult> UpdateTransaction::search(std::string_view word) const
{
    if (!active_)
    {
        return std::unexpected(ErrorCode::kTransactionInactive);
    }
    if (word.empty())
    {
        return std::unexpected(ErrorCode::kEmptyWord);
    }
    return tempIndex_.search(word);
}

VoidResult UpdateTransaction::commit()
{
    if (!active_)
    {
        return std::unexpected(ErrorCode::kTransactionInactive);
    }
    if (committed_)
    {
        return std::unexpected(ErrorCode::kTransactionInactive);
    }
    store_->index_ = std::move(tempIndex_);
    committed_ = true;
    active_ = false;
    return {};
}