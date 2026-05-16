#pragma once

#include "Result.hpp"
#include "InvertedIndex.hpp"
#include "UpdateTransaction.hpp"

class IndexStore
{
public:
    IndexStore() = default;

    // Базовые операции
    VoidResult addDocument(Document doc);
    VoidResult removeDocument(int id);
    Result<InvertedIndex::SearchResult> search(std::string_view word) const;
    const Document* getDocument(int id) const;

    // Начать транзакцию
    Result<UpdateTransaction> beginTransaction();

private:
    InvertedIndex index_;

    friend class UpdateTransaction;
};
