#pragma once

#include "cell.h"
#include "common.h"

#include <functional>

//  ласс CellHasher используетс€ дл€ вычислени€ хеша позиции €чейки
class CellHasher {
public:
    // ѕерегрузка оператора вызова функции дл€ вычислени€ хеша позиции
    size_t operator()(const Position p) const {
        return std::hash<std::string>()(p.ToString());  // ѕреобразуем позицию в строку и вычисл€ем хеш
    }
};

//  ласс CellComparator используетс€ дл€ сравнени€ двух позиций €чеек
class CellComparator {
public:
    // ѕерегрузка оператора вызова функции дл€ сравнени€ двух позиций
    bool operator()(const Position& lhs, const Position& rhs) const {
        return lhs == rhs;  // —равниваем две позиции на равенство
    }
};

//  ласс Sheet реализует интерфейс SheetInterface и представл€ет собой таблицу €чеек
class Sheet : public SheetInterface {
public:
    // ќпредел€ем тип Table как unordered_map с ключом Position и значением Cell
    using Table = std::unordered_map<Position, Cell, CellHasher, CellComparator>;

    // ƒеструктор класса
    ~Sheet();

    // ћетод дл€ установки значени€ €чейки по заданной позиции
    void SetCell(Position pos, std::string text) override;

    // ћетод дл€ получени€ константной ссылки на €чейку по заданной позиции
    const CellInterface* GetCell(Position pos) const override;

    // ћетод дл€ получени€ ссылки на €чейку по заданной позиции
    CellInterface* GetCell(Position pos) override;

    // ћетод дл€ очистки €чейки по заданной позиции
    void ClearCell(Position pos) override;

    // ћетод дл€ получени€ размера таблицы, который можно вывести на печать
    Size GetPrintableSize() const override;

    // ћетод дл€ вывода значений €чеек в поток вывода
    void PrintValues(std::ostream& output) const override;

    // ћетод дл€ вывода текстов €чеек в поток вывода
    void PrintTexts(std::ostream& output) const override;

private:
    // ’ранилище €чеек таблицы
    Table cells_;
};