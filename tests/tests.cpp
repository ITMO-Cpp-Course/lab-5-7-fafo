#include "DocumentBuilder.hpp"
#include "InvertedIndex.hpp"
#include <catch2/catch_all.hpp>

using namespace std;

TEST_CASE("Document: конструктор и геттеры", "[Document]")
{
    Document doc(42, "test.txt", "some content");

    REQUIRE(doc.id() == 42);
    REQUIRE(doc.name() == "test.txt");
    REQUIRE(doc.text() == "some content");
}

TEST_CASE("Document: конструктор по умолчанию", "[Document]")
{
    Document doc;

    REQUIRE(doc.id() == 0);
    REQUIRE(doc.name().empty());
    REQUIRE(doc.text().empty());
}

TEST_CASE("DocumentBuilder: build создаёт документ", "[DocumentBuilder]")
{
    Document doc = DocumentBuilder::build("myfile.txt", "Hello world");

    CHECK(doc.name() == "myfile.txt");
    CHECK(doc.text() == "Hello world");
}

TEST_CASE("DocumentBuilder: splitWords разбивает текст на слова", "[DocumentBuilder]")
{
    SECTION("Обычный текст с пробелами")
    {
        string text = "hello world from index";
        auto words = DocumentBuilder::splitWords(text);

        REQUIRE(words.size() == 4);
        CHECK(words[0] == "hello");
        CHECK(words[1] == "world");
        CHECK(words[2] == "from");
        CHECK(words[3] == "index");
    }

    SECTION("Текст с запятыми и точками")
    {
        string text = "Hello, world! This is a test.";
        auto words = DocumentBuilder::splitWords(text);

        REQUIRE(words.size() == 6);
        CHECK(words[0] == "Hello");
        CHECK(words[1] == "world");
        CHECK(words[2] == "This");
        CHECK(words[3] == "is");
        CHECK(words[4] == "a");
        CHECK(words[5] == "test");
    }

    SECTION("Текст со скобками и кавычками")
    {
        string text = "(Hello) 'world' [test]";
        auto words = DocumentBuilder::splitWords(text);

        REQUIRE(words.size() == 3);
        CHECK(words[0] == "Hello");
        CHECK(words[1] == "world");
        CHECK(words[2] == "test");
    }

    SECTION("Несколько пробелов между словами")
    {
        string text = "hello    world     index";
        auto words = DocumentBuilder::splitWords(text);

        REQUIRE(words.size() == 3);
        CHECK(words[0] == "hello");
        CHECK(words[1] == "world");
        CHECK(words[2] == "index");
    }

    SECTION("Пустая строка")
    {
        string text = "";
        auto words = DocumentBuilder::splitWords(text);
        REQUIRE(words.empty());
    }

    SECTION("Только разделители")
    {
        string text = "   \n\t ,.!?;:() [] {} ";
        auto words = DocumentBuilder::splitWords(text);
        REQUIRE(words.empty());
    }

    SECTION("Слова с дефисами (дефис - разделитель)")
    {
        string text = "hello-world test";
        auto words = DocumentBuilder::splitWords(text);

        // Дефис считается разделителем, поэтому "hello" и "world" отдельно
        REQUIRE(words.size() == 3);
        CHECK(words[0] == "hello");
        CHECK(words[1] == "world");
        CHECK(words[2] == "test");
    }
}

TEST_CASE("DocumentBuilder: toLower приводит к нижнему регистру", "[DocumentBuilder]")
{
    SECTION("Английские буквы")
    {
        string word = "HelloWorld";
        DocumentBuilder::toLower(word);
        CHECK(word == "helloworld");
    }

    SECTION("Смешанный регистр")
    {
        string word = "HeLlO WoRlD";
        DocumentBuilder::toLower(word);
        CHECK(word == "hello world");
    }

    SECTION("Уже нижний регистр")
    {
        string word = "hello";
        DocumentBuilder::toLower(word);
        CHECK(word == "hello");
    }

    SECTION("Цифры и знаки не меняются")
    {
        string word = "Hello123!";
        DocumentBuilder::toLower(word);
        CHECK(word == "hello123!");
    }
}

TEST_CASE("InvertedIndex: добавление одного документа", "[InvertedIndex]")
{
    InvertedIndex index;
    Document doc(1, "doc1.txt", "hello world");
    index.addDocument(std::move(doc));

    SECTION("Поиск существующего слова")
    {
        auto result = index.search("hello");
        REQUIRE(result.size() == 1);
        CHECK(result[0].first == 1);
        CHECK(result[0].second == 1);
    }

    SECTION("Поиск другого существующего слова")
    {
        auto result = index.search("world");
        REQUIRE(result.size() == 1);
        CHECK(result[0].first == 1);
        CHECK(result[0].second == 1);
    }

    SECTION("Поиск несуществующего слова")
    {
        auto result = index.search("nonexistent");
        REQUIRE(result.empty());
    }
}

TEST_CASE("InvertedIndex: подсчёт частоты слов", "[InvertedIndex]")
{
    InvertedIndex index;
    Document doc(1, "doc1.txt", "hello hello world hello");
    index.addDocument(std::move(doc));

    auto result = index.search("hello");
    REQUIRE(result.size() == 1);
    CHECK(result[0].second == 3);

    auto result2 = index.search("world");
    REQUIRE(result2.size() == 1);
    CHECK(result2[0].second == 1);
}

TEST_CASE("InvertedIndex: несколько документов", "[InvertedIndex]")
{
    InvertedIndex index;

    Document doc1(1, "file1.txt", "hello world");
    Document doc2(2, "file2.txt", "hello index");
    Document doc3(3, "file3.txt", "world index");

    index.addDocument(std::move(doc1));
    index.addDocument(std::move(doc2));
    index.addDocument(std::move(doc3));

    SECTION("Поиск 'hello' - в 2 документах")
    {
        auto result = index.search("hello");
        REQUIRE(result.size() == 2);

        bool hasDoc1 = false, hasDoc2 = false;
        for (const auto& [id, count] : result)
        {
            if (id == 1)
            {
                hasDoc1 = true;
                CHECK(count == 1);
            }
            if (id == 2)
            {
                hasDoc2 = true;
                CHECK(count == 1);
            }
        }
        CHECK(hasDoc1);
        CHECK(hasDoc2);
    }

    SECTION("Поиск 'world' - в 2 документах (1 и 3)")
    {
        auto result = index.search("world");
        REQUIRE(result.size() == 2);
    }

    SECTION("Поиск 'index' - в 2 документах (2 и 3)")
    {
        auto result = index.search("index");
        REQUIRE(result.size() == 2);
    }
}

TEST_CASE("InvertedIndex: регистронезависимость", "[InvertedIndex]")
{
    InvertedIndex index;
    Document doc(1, "doc.txt", "Hello WORLD hello");
    index.addDocument(std::move(doc));

    auto result1 = index.search("hello");
    REQUIRE(result1.size() == 1);
    CHECK(result1[0].second == 2); // Hello и hello

    auto result2 = index.search("WORLD");
    REQUIRE(result2.size() == 1);
    CHECK(result2[0].second == 1);
}

TEST_CASE("InvertedIndex: удаление документа", "[InvertedIndex]")
{
    InvertedIndex index;

    Document doc1(1, "file1.txt", "hello world");
    Document doc2(2, "file2.txt", "hello index");

    index.addDocument(std::move(doc1));
    index.addDocument(std::move(doc2));

    CHECK(index.search("hello").size() == 2);

    index.removeDocument(1);

    auto result = index.search("hello");
    REQUIRE(result.size() == 1);
    CHECK(result[0].first == 2);
    CHECK(index.search("world").empty());
}

TEST_CASE("InvertedIndex: удаление несуществующего документа", "[InvertedIndex]")
{
    InvertedIndex index;
    Document doc(1, "doc.txt", "hello world");
    index.addDocument(std::move(doc));

    index.removeDocument(999); // не существует

    auto result = index.search("hello");
    REQUIRE(result.size() == 1);
    CHECK(result[0].first == 1);
}

TEST_CASE("InvertedIndex: getDocument возвращает документ", "[InvertedIndex]")
{
    InvertedIndex index;
    Document doc(42, "mydoc.txt", "some content");
    index.addDocument(std::move(doc));

    SECTION("Существующий документ")
    {
        const Document* retrieved = index.getDocument(42);
        REQUIRE(retrieved != nullptr);
        CHECK(retrieved->name() == "mydoc.txt");
        CHECK(retrieved->text() == "some content");
    }

    SECTION("Несуществующий документ")
    {
        const Document* retrieved = index.getDocument(999);
        CHECK(retrieved == nullptr);
    }
}

TEST_CASE("InvertedIndex: замена документа с тем же ID", "[InvertedIndex]")
{
    InvertedIndex index;

    Document doc1(1, "first.txt", "hello world");
    Document doc2(1, "second.txt", "goodbye world");

    index.addDocument(std::move(doc1));
    index.addDocument(std::move(doc2));

    // Старое слово "hello" должно исчезнуть
    CHECK(index.search("hello").empty());

    // Новое слово "goodbye" должно быть
    auto result = index.search("goodbye");
    REQUIRE(result.size() == 1);
    CHECK(result[0].first == 1);
    CHECK(result[0].second == 1);

    // "world" должно быть (есть в обоих, но после замены - только в новом)
    auto worldResult = index.search("world");
    REQUIRE(worldResult.size() == 1);
    CHECK(worldResult[0].first == 1);
    CHECK(worldResult[0].second == 1);

    const Document* doc = index.getDocument(1);
    REQUIRE(doc != nullptr);
    CHECK(doc->name() == "second.txt");
}

TEST_CASE("InvertedIndex: пустой документ", "[InvertedIndex]")
{
    InvertedIndex index;
    Document doc(1, "empty.txt", "");
    index.addDocument(std::move(doc));

    CHECK(index.search("anything").empty());
    CHECK(index.getDocument(1) != nullptr);
}

TEST_CASE("InvertedIndex: перекрестные проверки", "[InvertedIndex]")
{
    InvertedIndex index;

    Document doc1(1, "doc1.txt", "cat dog fish");
    Document doc2(2, "doc2.txt", "dog bird cat");
    Document doc3(3, "doc3.txt", "fish bird");

    index.addDocument(std::move(doc1));
    index.addDocument(std::move(doc2));
    index.addDocument(std::move(doc3));

    CHECK(index.search("cat").size() == 2);
    CHECK(index.search("dog").size() == 2);
    CHECK(index.search("fish").size() == 2);
    CHECK(index.search("bird").size() == 2);
    CHECK(index.search("elephant").empty());
}