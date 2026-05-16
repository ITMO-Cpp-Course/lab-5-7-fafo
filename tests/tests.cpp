#include "DocumentBuilder.hpp"
#include "IndexStore.hpp"
#include <catch2/catch_all.hpp>

using namespace std;



TEST_CASE("IndexStore: addDocument успешно добавляет документ", "[IndexStore]")
{
    IndexStore store;
    Document doc(1, "test.txt", "hello world");

    auto result = store.addDocument(std::move(doc));
    REQUIRE(result.has_value());

    const Document* retrieved = store.getDocument(1);
    REQUIRE(retrieved != nullptr);
    REQUIRE(retrieved->name() == "test.txt");
    REQUIRE(retrieved->text() == "hello world");
}

TEST_CASE("IndexStore: addDocument возвращает ошибку при дубликате id", "[IndexStore]")
{
    IndexStore store;
    Document doc1(1, "first.txt", "content");
    Document doc2(1, "second.txt", "other");

    REQUIRE(store.addDocument(std::move(doc1)).has_value());
    auto result = store.addDocument(std::move(doc2));
    REQUIRE(!result.has_value());
    REQUIRE(result.error() == ErrorCode::kDuplicateDocumentId);
}

TEST_CASE("IndexStore: removeDocument успешно удаляет документ", "[IndexStore]")
{
    IndexStore store;
    Document doc(1, "test.txt", "hello");
    store.addDocument(std::move(doc));

    auto result = store.removeDocument(1);
    REQUIRE(result.has_value());
    REQUIRE(store.getDocument(1) == nullptr);
}

TEST_CASE("IndexStore: removeDocument возвращает ошибку, если документ не найден", "[IndexStore]")
{
    IndexStore store;
    auto result = store.removeDocument(999);
    REQUIRE(!result.has_value());
    REQUIRE(result.error() == ErrorCode::kDocumentNotFound);
}

TEST_CASE("IndexStore: search находит слова и возвращает корректные вхождения", "[IndexStore]")
{
    IndexStore store;
    Document doc(1, "doc.txt", "hello world hello");
    store.addDocument(std::move(doc));

    auto result = store.search("hello");
    REQUIRE(result.has_value());
    REQUIRE(result->size() == 1);
    REQUIRE((*result)[0].first == 1);
    REQUIRE((*result)[0].second == 2);
}

TEST_CASE("IndexStore: search возвращает пустой результат, если слово отсутствует", "[IndexStore]")
{
    IndexStore store;
    Document doc(1, "doc.txt", "hello world");
    store.addDocument(std::move(doc));

    auto result = store.search("missing");
    REQUIRE(result.has_value());
    REQUIRE(result->empty());
}

TEST_CASE("IndexStore: search возвращает ошибку на пустое слово", "[IndexStore]")
{
    IndexStore store;
    auto result = store.search("");
    REQUIRE(!result.has_value());
    REQUIRE(result.error() == ErrorCode::kEmptyWord);
}

TEST_CASE("IndexStore: getDocument возвращает nullptr для несуществующего id", "[IndexStore]")
{
    IndexStore store;
    REQUIRE(store.getDocument(42) == nullptr);
}

TEST_CASE("UpdateTransaction: успешный commit применяет изменения", "[Transaction]")
{
    IndexStore store;

    auto transResult = store.beginTransaction();
    REQUIRE(transResult.has_value());
    auto& trans = *transResult;

    Document doc(1, "test.txt", "transaction content");
    REQUIRE(trans.addDocument(std::move(doc)).has_value());

    REQUIRE(store.getDocument(1) == nullptr);
    REQUIRE(store.search("transaction").value().empty());

    auto commitResult = trans.commit();
    REQUIRE(commitResult.has_value());

    const Document* retrieved = store.getDocument(1);
    REQUIRE(retrieved != nullptr);
    REQUIRE(retrieved->text() == "transaction content");

    auto searchResult = store.search("transaction");
    REQUIRE(searchResult.has_value());
    REQUIRE(searchResult->size() == 1);
}

TEST_CASE("UpdateTransaction: автоматический откат при разрушении без commit", "[Transaction]")
{
    IndexStore store;

    {
        auto transResult = store.beginTransaction();
        REQUIRE(transResult.has_value());
        auto& trans = *transResult;

        Document doc(1, "temp.txt", "temporary");
        trans.addDocument(std::move(doc));
    }

    REQUIRE(store.getDocument(1) == nullptr);
    REQUIRE(store.search("temporary").value().empty());
}

TEST_CASE("UpdateTransaction: операции внутри транзакции не видны снаружи до commit", "[Transaction]")
{
    IndexStore store;
    Document original(1, "original.txt", "original text");
    store.addDocument(std::move(original));

    auto transResult = store.beginTransaction();
    REQUIRE(transResult.has_value());
    auto& trans = *transResult;

    REQUIRE(trans.removeDocument(1).has_value());

    REQUIRE(store.getDocument(1) != nullptr);

    Document doc2(2, "new.txt", "new content");
    REQUIRE(trans.addDocument(std::move(doc2)).has_value());

    REQUIRE(store.getDocument(2) == nullptr);

    REQUIRE(trans.commit().has_value());

    REQUIRE(store.getDocument(1) == nullptr);
    REQUIRE(store.getDocument(2) != nullptr);
}

TEST_CASE("UpdateTransaction: ошибки внутри транзакции не портят основной индекс", "[Transaction]")
{
    IndexStore store;
    Document original(1, "keep.txt", "important");
    store.addDocument(std::move(original));

    auto transResult = store.beginTransaction();
    REQUIRE(transResult.has_value());
    auto& trans = *transResult;

    Document duplicate(1, "duplicate.txt", "bad");
    auto addResult = trans.addDocument(std::move(duplicate));
    REQUIRE(!addResult.has_value());
    REQUIRE(addResult.error() == ErrorCode::kDuplicateDocumentId);

    auto searchInTrans = trans.search("bad");
    REQUIRE(searchInTrans.has_value());
    REQUIRE(searchInTrans->empty());

    REQUIRE(trans.commit().has_value());

    REQUIRE(store.getDocument(1) != nullptr);
    REQUIRE(store.getDocument(1)->text() == "important");
}

TEST_CASE("UpdateTransaction: поиск внутри транзакции работает на снимке", "[Transaction]")
{
    IndexStore store;
    Document doc1(1, "one.txt", "apple banana");
    Document doc2(2, "two.txt", "apple cherry");
    store.addDocument(std::move(doc1));
    store.addDocument(std::move(doc2));

    auto transResult = store.beginTransaction();
    REQUIRE(transResult.has_value());
    auto& trans = *transResult;

    REQUIRE(trans.removeDocument(2).has_value());

    auto searchResult = trans.search("apple");
    REQUIRE(searchResult.has_value());
    REQUIRE(searchResult->size() == 1);
    REQUIRE((*searchResult)[0].first == 1);

    auto outsideSearch = store.search("apple");
    REQUIRE(outsideSearch.has_value());
    REQUIRE(outsideSearch->size() == 2);
}

TEST_CASE("UpdateTransaction: повторный commit невозможен", "[Transaction]")
{
    IndexStore store;
    auto transResult = store.beginTransaction();
    REQUIRE(transResult.has_value());
    auto& trans = *transResult;

    REQUIRE(trans.commit().has_value());
    auto secondCommit = trans.commit();
    REQUIRE(!secondCommit.has_value());
    REQUIRE(secondCommit.error() == ErrorCode::kTransactionInactive);
}

TEST_CASE("UpdateTransaction: операции после commit возвращают ошибку", "[Transaction]")
{
    IndexStore store;
    auto transResult = store.beginTransaction();
    REQUIRE(transResult.has_value());
    auto& trans = *transResult;

    REQUIRE(trans.commit().has_value());

    Document doc(1, "late.txt", "too late");
    auto addResult = trans.addDocument(std::move(doc));
    REQUIRE(!addResult.has_value());
    REQUIRE(addResult.error() == ErrorCode::kTransactionInactive);

    auto removeResult = trans.removeDocument(1);
    REQUIRE(!removeResult.has_value());

    auto searchResult = trans.search("anything");
    REQUIRE(!searchResult.has_value());
}

TEST_CASE("UpdateTransaction: перемещение транзакции работает корректно", "[Transaction]")
{
    IndexStore store;
    auto transResult = store.beginTransaction();
    REQUIRE(transResult.has_value());

    UpdateTransaction movedTrans = std::move(*transResult);

    Document doc(1, "moved.txt", "content");
    REQUIRE(movedTrans.addDocument(std::move(doc)).has_value());
    REQUIRE(movedTrans.commit().has_value());

    REQUIRE(store.getDocument(1) != nullptr);
}

TEST_CASE("IndexStore: beginTransaction создаёт копию состояния", "[Transaction]")
{
    IndexStore store;
    Document doc(1, "original.txt", "data");
    store.addDocument(std::move(doc));

    auto transResult = store.beginTransaction();
    REQUIRE(transResult.has_value());

    Document doc2(2, "new.txt", "new");
    store.addDocument(std::move(doc2));

    auto& trans = *transResult;
    auto searchResult = trans.search("data");
    REQUIRE(searchResult.has_value());
    REQUIRE(searchResult->size() == 1);

    auto searchNew = trans.search("new");
    REQUIRE(searchNew.has_value());
    REQUIRE(searchNew->empty());

    REQUIRE(trans.commit().has_value());
    REQUIRE(store.getDocument(2) == nullptr);
    REQUIRE(store.getDocument(1) != nullptr);
}