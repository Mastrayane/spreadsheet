#include "common.h"

#include <cctype>
#include <sstream>
#include <algorithm>

// Константы для работы с позициями и размерами
const int LETTERS = 26; // Количество букв в алфавите
const int MAX_POSITION_LENGTH = 17; // Максимальная длина строки для позиции
const int MAX_POS_LETTER_COUNT = 3; // Максимальное количество букв в позиции

// Статическая константа для невалидной позиции
const Position Position::NONE = { -1, -1 };

// Оператор сравнения на равенство для Position
bool Position::operator==(const Position rhs) const {
    return row == rhs.row && col == rhs.col;
}

// Оператор сравнения на меньше для Position
bool Position::operator<(const Position rhs) const {
    return std::tie(row, col) < std::tie(rhs.row, rhs.col);
}

// Проверка, является ли позиция валидной
bool Position::IsValid() const {
    return row >= 0 && col >= 0 && row < MAX_ROWS && col < MAX_COLS;
}

// Преобразование позиции в строку
std::string Position::ToString() const {
    // Проверяем, является ли позиция валидной
    if (!IsValid()) {
        return ""; // Если позиция невалидна, возвращаем пустую строку
    }

    std::string result; // Создаем строку для результата
    result.reserve(MAX_POSITION_LENGTH); // Резервируем память для строки, чтобы избежать многократных перераспределений
    int c = col; // Копируем значение столбца в переменную c

    // Преобразуем столбец в буквенное представление
    while (c >= 0) {
        // Вставляем символ 'A' плюс остаток от деления c на LETTERS в начало строки
        result.insert(result.begin(), 'A' + c % LETTERS);
        // Обновляем значение c, деля его на LETTERS и вычитая 1
        c = c / LETTERS - 1;
    }

    // Добавляем строковое представление строки (корректируя индекс на 1)
    result += std::to_string(row + 1);

    return result; // Возвращаем результирующую строку
}

// Преобразование строки в позицию
Position Position::FromString(std::string_view str) {
    // Находим первый символ в строке, который не является буквой верхнего регистра
    auto it = std::find_if(str.begin(), str.end(), [](const char c) {
        // Лямбда-функция, которая возвращает true, если символ не является буквой верхнего регистра
        return !(std::isalpha(c) && std::isupper(c));
        });
    auto letters = str.substr(0, it - str.begin()); // Извлекаем буквенную часть
    auto digits = str.substr(it - str.begin()); // Извлекаем цифровую часть

    // Проверяем, что обе части не пустые
    if (letters.empty() || digits.empty()) {
        return Position::NONE;
    }
    // Проверяем, что буквенная часть не превышает максимальное количество букв
    if (letters.size() > MAX_POS_LETTER_COUNT) {
        return Position::NONE;
    }

    // Проверяем, что первый символ цифровой части является цифрой
    if (!std::isdigit(digits[0])) {
        return Position::NONE;
    }

    int row;
    std::istringstream row_in{ std::string{digits} };
    // Преобразуем цифровую часть в число и проверяем, что она корректна
    if (!(row_in >> row) || !row_in.eof()) {
        return Position::NONE;
    }

    int col = 0;
    // Преобразуем буквенную часть в число
    for (char ch : letters) {
        col *= LETTERS;
        col += ch - 'A' + 1;
    }

    // Возвращаем позицию, корректируя индексы
    return { row - 1, col - 1 };
}

// Оператор сравнения на равенство для Size
bool Size::operator==(Size rhs) const {
    return cols == rhs.cols && rows == rhs.rows;
}