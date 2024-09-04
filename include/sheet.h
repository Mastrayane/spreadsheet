#pragma once

#include "cell.h"
#include "common.h"

#include <functional>

class CellHasher {
public:
    size_t operator()(const Position p) const {
        return std::hash<std::string>()(p.ToString());
    }
};

class CellComparator {
public:
    bool operator()(const Position& lhs, const Position& rhs) const {
        return lhs == rhs;
    }
};

class Sheet : public SheetInterface {
public:
    using Table = std::unordered_map<Position, Cell, CellHasher, CellComparator>;

    ~Sheet();

    void SetCell(Position pos, std::string text) override;

    const CellInterface* GetCell(Position pos) const override;
    CellInterface* GetCell(Position pos) override;

    void ClearCell(Position pos) override;

    Size GetPrintableSize() const override;

    void PrintValues(std::ostream& output) const override;
    void PrintTexts(std::ostream& output) const override;

private:
	Table cells_;
};