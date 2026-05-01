#include "DocumentBuilder.hpp"
#include <algorithm> // Для std::transform

std::vector<std::string_view> DocumentBuilder::splitWords(std::string_view text) {
    std::vector<std::string_view> words;
    size_t start = 0;
    while (start < text.size()) {
        // Пропускаем пробельные символы
        while (start < text.size() && std::isspace(static_cast<unsigned char>(text[start]))) {
            ++start;
        }
        //char c = -128;  // char может быть signed
        //std::isspace(c);  //  ОПАСНО! Если c отрицательный — Undefined Behavior!
        if (start >= text.size()) break;

        size_t end = start;
        // Идём до следующего пробельного символа
        while (end < text.size() && !std::isspace(static_cast<unsigned char>(text[end]))) {
            ++end;
        }
        // [start, end) – одно слово как string_view без копирования
        words.emplace_back(text.substr(start, end - start));
        //  │         │           │       │     │
        //  │         │           │       │     └── длина слова (end - start)
        //  │         │           │       └── начальная позиция слова
        //  │         │           └── метод string_view, создаёт наблюдатель на часть строки
        //  │         └── метод вектора, создаёт элемент "на месте"
        //  └── вектор, в который добавляем
        //words.emplace_back(...) — это просто добавление нового элемента в конец вектора word(push_back медленее из-за создания времменой переменной)
        start = end;
    }
    return words;
}

void DocumentBuilder::toLower(std::string& word) {
    // std::transform применяет операцию к каждому символу строки на месте.
    std::transform(word.begin(), word.end(), word.begin(),
                   [](unsigned char c) { return std::tolower(c); });
}
// Параметры:
//  *   1. word.begin()  — начало диапазона (откуда читаем)
//  *   2. word.end()    — конец диапазона (где заканчиваем читать)
//  *   3. word.begin()  — куда записываем результат (в начало той же строки)
//  *   4. лямбда-функция — что делаем с каждым символом
//  *
//  * Лямбда [](unsigned char c) { return std::tolower(c); }
// *   — это безымянная функция, которая:
// *     * принимает один символ типа unsigned char ОПАСНО: char может быть отрицательным
// *     * возвращает этот символ в нижнем регистре