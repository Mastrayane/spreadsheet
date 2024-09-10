#pragma once

#include "FormulaLexer.h"
#include "common.h"

#include <forward_list>
#include <functional>
#include <stdexcept>

// Объявляем пространство имен для AST (Abstract Syntax Tree)
namespace ASTImpl {
    class Expr;
}

// Класс для обработки ошибок парсинга
class ParsingError : public std::runtime_error {
    // Используем конструктор базового класса
    using std::runtime_error::runtime_error;
};

// Класс для представления AST формулы
class FormulaAST {
public:
    // Конструктор, принимающий указатель на корневое выражение AST
    explicit FormulaAST(std::unique_ptr<ASTImpl::Expr> root_expr);

    FormulaAST(FormulaAST&&) = default;
    FormulaAST& operator=(FormulaAST&&) = default;

    ~FormulaAST();

    // Метод для выполнения формулы и получения результата
    double Execute() const;

    // Метод для печати AST в поток вывода
    void Print(std::ostream& out) const;

    // Метод для печати формулы в поток вывода
    void PrintFormula(std::ostream& out) const;

private:
    // Указатель на корневое выражение AST
    std::unique_ptr<ASTImpl::Expr> root_expr_;
};

// Функция для парсинга AST формулы из потока ввода
FormulaAST ParseFormulaAST(std::istream& in);

// Функция для парсинга AST формулы из строки
FormulaAST ParseFormulaAST(const std::string& in_str);