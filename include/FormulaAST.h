#pragma once

#include "FormulaLexer.h"
#include "common.h"

#include <forward_list>
#include <functional>
#include <stdexcept>

// ��������� ������������ ���� ��� AST (Abstract Syntax Tree)
namespace ASTImpl {
    class Expr;
}

// ����� ��� ��������� ������ ��������
class ParsingError : public std::runtime_error {
    // ���������� ����������� �������� ������
    using std::runtime_error::runtime_error;
};

// ����� ��� ������������� AST �������
class FormulaAST {
public:
    // �����������, ����������� ��������� �� �������� ��������� AST
    explicit FormulaAST(std::unique_ptr<ASTImpl::Expr> root_expr);

    FormulaAST(FormulaAST&&) = default;
    FormulaAST& operator=(FormulaAST&&) = default;

    ~FormulaAST();

    // ����� ��� ���������� ������� � ��������� ����������
    double Execute() const;

    // ����� ��� ������ AST � ����� ������
    void Print(std::ostream& out) const;

    // ����� ��� ������ ������� � ����� ������
    void PrintFormula(std::ostream& out) const;

private:
    // ��������� �� �������� ��������� AST
    std::unique_ptr<ASTImpl::Expr> root_expr_;
};

// ������� ��� �������� AST ������� �� ������ �����
FormulaAST ParseFormulaAST(std::istream& in);

// ������� ��� �������� AST ������� �� ������
FormulaAST ParseFormulaAST(const std::string& in_str);