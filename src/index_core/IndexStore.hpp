#pragma once

#include "InvertedIndex.hpp"
#include "Result.hpp"
#include "UpdateTransaction.hpp"

class IndexStore
{
  public:
    IndexStore() = default;

    VoidResult addDocument(Document doc);
    VoidResult removeDocument(int id);
    Result<InvertedIndex::SearchResult> search(std::string_view word) const;
    const Document* getDocument(int id) const;

    Result<UpdateTransaction> beginTransaction();

  private:
    InvertedIndex index_;

    friend class UpdateTransaction;
};
