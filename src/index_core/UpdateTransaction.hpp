#pragma once

#include "InvertedIndex.hpp"
#include "Result.hpp"

class IndexStore;

class UpdateTransaction
{
  public:
    UpdateTransaction(IndexStore& store, InvertedIndex&& snapshot);
    ~UpdateTransaction();

    UpdateTransaction(const UpdateTransaction&) = delete;
    UpdateTransaction& operator=(const UpdateTransaction&) = delete;

    UpdateTransaction(UpdateTransaction&& other) noexcept;
    UpdateTransaction& operator=(UpdateTransaction&& other) noexcept;

    VoidResult addDocument(Document doc);
    VoidResult removeDocument(int id);
    Result<InvertedIndex::SearchResult> search(std::string_view word) const;

    VoidResult commit();

  private:
    IndexStore* store_;
    InvertedIndex tempIndex_;
    bool active_;
    bool committed_;
};