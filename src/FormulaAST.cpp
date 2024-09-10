#include "FormulaAST.h"

#include "FormulaBaseListener.h"
#include "FormulaLexer.h"
#include "FormulaParser.h"

#include <cassert>
#include <cmath>
#include <memory>
#include <optional>
#include <sstream>

namespace ASTImpl {

    // Перечисление для определения приоритета операций
    enum ExprPrecedence {
        EP_ADD,    // Сложение
        EP_SUB,    // Вычитание
        EP_MUL,    // Умножение
        EP_DIV,    // Деление
        EP_UNARY,  // Унарные операции
        EP_ATOM,   // Атомарные выражения (числа, переменные)
        EP_END,    // Конец списка приоритетов
    };

    // Флаги для определения необходимости скобок
    enum PrecedenceRule {
        PR_NONE = 0b00,                // Скобки не нужны
        PR_LEFT = 0b01,                // Скобки нужны для левого потомка
        PR_RIGHT = 0b10,               // Скобки нужны для правого потомка
        PR_BOTH = PR_LEFT | PR_RIGHT,  // Скобки нужны для обоих потомков
    };

    // PRECEDENCE_RULES[родительский][дочерний] определяет, нужно ли вставлять круглые скобки
    // между родительским и дочерним узлами с определенным приоритетом;
    // для некоторых узлов правила различаются для левого и правого дочерних узлов:
    // (X c Y) p Z против X p (Y c Z)
    //
    // Интересны случаи, когда удаление круглых скобок изменило бы AST.
    // Это может произойти, когда наши правила приоритета для круглых скобок отличаются от
    // грамматического приоритета операций.
    //
    // Разбор падежей:
    // A + (B + C) - всегда в порядке (справа не могло быть записано ничего с более низким грамматическим приоритетом)
    // (например, если бы у нас было A + (B + C) / D, это не было бы обработано таким образом
    //, что дало бы нам A + (B + C) в качестве подвыражения, с которым нужно было бы иметь дело)
    // A + (B - C) - всегда в порядке (справа не могло быть записано ничего с более низким грамматическим приоритетом)
    // A - (B + C) - никогда не бывает в порядке A - (B - C) - никогда не бывает в порядке A * (B * C) - всегда в порядке(родительский
    // имеет наивысший грамматический приоритет) A * (B / C) - всегда в порядке (родительский имеет наивысший грамматический приоритет
    // грамматический приоритет) A / (B * C) - никогда не подходит A / (B / C) - никогда не подходит
    // -(A + B) - никогда не подходит
    // -(A - B) - никогда не подходит
    // -(A * B) - всегда в порядке (результирующая двоичная операция имеет наивысший грамматический приоритет)
    // -(A / B) - всегда в порядке (результирующая двоичная операция имеет наивысший грамматический приоритет)
    // +(A + B) - **иногда нормально ** (например, скобки в +(A + B) / C ** необязательны)
    // (в настоящее время в таблице мы всегда ставим круглые скобки)
    // +(A - B) - **иногда нормально ** (то же самое)
    // (в настоящее время в таблице мы всегда указываем в круглых скобках)
    // +(A * B) - всегда нормально (результирующая двоичная операция имеет наивысший грамматический приоритет)
    // +(A / B) - всегда в порядке (результирующая двоичная операция имеет наивысший грамматический приоритет)

    // Таблица правил приоритетов для определения необходимости скобок
constexpr PrecedenceRule PRECEDENCE_RULES[EP_END][EP_END] = {
    /* EP_ADD */ {PR_NONE, PR_NONE, PR_NONE, PR_NONE, PR_NONE, PR_NONE},
    /* EP_SUB */ {PR_RIGHT, PR_RIGHT, PR_NONE, PR_NONE, PR_NONE, PR_NONE},
    /* EP_MUL */ {PR_BOTH, PR_BOTH, PR_NONE, PR_NONE, PR_NONE, PR_NONE},
    /* EP_DIV */ {PR_BOTH, PR_BOTH, PR_RIGHT, PR_RIGHT, PR_NONE, PR_NONE},
    /* EP_UNARY */ {PR_BOTH, PR_BOTH, PR_NONE, PR_NONE, PR_NONE, PR_NONE},
    /* EP_ATOM */ {PR_NONE, PR_NONE, PR_NONE, PR_NONE, PR_NONE, PR_NONE},
};

// Абстрактный базовый класс для выражений
class Expr {
public:
    virtual ~Expr() = default;
    virtual void Print(std::ostream& out) const = 0;
    virtual void DoPrintFormula(std::ostream& out, ExprPrecedence precedence) const = 0;
    // Метод для вычисления значения выражения.
    virtual double Evaluate() const = 0;

    // Возвращает приоритет выражения
    virtual ExprPrecedence GetPrecedence() const = 0;

    // Метод для печати формулы с учетом приоритетов
    void PrintFormula(std::ostream& out, ExprPrecedence parent_precedence,
                      bool right_child = false) const {
        auto precedence = GetPrecedence();
        auto mask = right_child ? PR_RIGHT : PR_LEFT;
        bool parens_needed = PRECEDENCE_RULES[parent_precedence][precedence] & mask;
        if (parens_needed) {
            out << '(';
        }

        DoPrintFormula(out, precedence);

        if (parens_needed) {
            out << ')';
        }
    }
};

namespace {
    // Класс для бинарных операций
class BinaryOpExpr final : public Expr {
public:
    enum Type : char {
        Add = '+',
        Subtract = '-',
        Multiply = '*',
        Divide = '/',
    };

public:
    explicit BinaryOpExpr(Type type, std::unique_ptr<Expr> lhs, std::unique_ptr<Expr> rhs)
        : type_(type)
        , lhs_(std::move(lhs))
        , rhs_(std::move(rhs)) {
    }

    void Print(std::ostream& out) const override {
        out << '(' << static_cast<char>(type_) << ' ';
        lhs_->Print(out);
        out << ' ';
        rhs_->Print(out);
        out << ')';
    }

    void DoPrintFormula(std::ostream& out, ExprPrecedence precedence) const override {
        lhs_->PrintFormula(out, precedence);
        out << static_cast<char>(type_);
        rhs_->PrintFormula(out, precedence, /* right_child = */ true);
    }

    // Возвращает приоритет выражения
    ExprPrecedence GetPrecedence() const override {
        switch (type_) {
            case Add:
                return EP_ADD;
            case Subtract:
                return EP_SUB;
            case Multiply:
                return EP_MUL;
            case Divide:
                return EP_DIV;
            default:
                // have to do this because VC++ has a buggy warning
                assert(false);
                return static_cast<ExprPrecedence>(INT_MAX);
        }
    }

// Метод для вычисления значения выражения для бинарных операций.
    double Evaluate() const override {
        double lhs_value = lhs_->Evaluate();
        double rhs_value = rhs_->Evaluate();
        double result;

        switch (type_) {
            case Add:
                result = lhs_value + rhs_value;
                break;
            case Subtract:
                result = lhs_value - rhs_value;
                break;
            case Multiply:
                result = lhs_value * rhs_value;
                break;
            case Divide:
                if (rhs_value == 0) {
                    throw FormulaError("ARITHM");
                }
                result = lhs_value / rhs_value;
                break;
            default:
                assert(false);
                return 0;
        }

        if (!std::isfinite(result)) {
            throw FormulaError("ARITHM");
        }

        return result;
    }

private:
    Type type_;
    std::unique_ptr<Expr> lhs_;
    std::unique_ptr<Expr> rhs_;
};

// Класс для унарных операций
class UnaryOpExpr final : public Expr {
public:
    enum Type : char {
        UnaryPlus = '+',
        UnaryMinus = '-',
    };

public:
    explicit UnaryOpExpr(Type type, std::unique_ptr<Expr> operand)
        : type_(type)
        , operand_(std::move(operand)) {
    }

    void Print(std::ostream& out) const override {
        out << '(' << static_cast<char>(type_) << ' ';
        operand_->Print(out);
        out << ')';
    }

    void DoPrintFormula(std::ostream& out, ExprPrecedence precedence) const override {
        out << static_cast<char>(type_);
        operand_->PrintFormula(out, precedence);
    }

    ExprPrecedence GetPrecedence() const override {
        return EP_UNARY;
    }

// Метод для вычисления значения выражения для унарных операций.
    double Evaluate() const override {
        double operand_value = operand_->Evaluate();
        double result;

        switch (type_) {
            case UnaryPlus:
                result = operand_value;
                break;
            case UnaryMinus:
                result = -operand_value;
                break;
            default:
                assert(false);
                return 0;
        }

        if (!std::isfinite(result)) {
            throw FormulaError("ARITHM");
        }

        return result;
    }

private:
    Type type_;
    std::unique_ptr<Expr> operand_;
};

// Класс для числовых выражений
class NumberExpr final : public Expr {
public:
    explicit NumberExpr(double value)
        : value_(value) {
    }

    void Print(std::ostream& out) const override {
        out << value_;
    }

    void DoPrintFormula(std::ostream& out, ExprPrecedence /* precedence */) const override {
        out << value_;
    }

    ExprPrecedence GetPrecedence() const override {
        return EP_ATOM;
    }

// Для чисел метод возвращает значение числа.
    double Evaluate() const override {
        return value_;
    }

private:
    double value_;
};

// Класс для обработки AST и создания дерева выражений
class ParseASTListener final : public FormulaBaseListener {
public:
    std::unique_ptr<Expr> MoveRoot() {
        assert(args_.size() == 1);
        auto root = std::move(args_.front());
        args_.clear();

        return root;
    }

public:
    void exitUnaryOp(FormulaParser::UnaryOpContext* ctx) override {
        assert(args_.size() >= 1);

        auto operand = std::move(args_.back());

        UnaryOpExpr::Type type;
        if (ctx->SUB()) {
            type = UnaryOpExpr::UnaryMinus;
        } else {
            assert(ctx->ADD() != nullptr);
            type = UnaryOpExpr::UnaryPlus;
        }

        auto node = std::make_unique<UnaryOpExpr>(type, std::move(operand));
        args_.back() = std::move(node);
    }

    void exitLiteral(FormulaParser::LiteralContext* ctx) override {
        double value = 0;
        auto valueStr = ctx->NUMBER()->getSymbol()->getText();
        std::istringstream in(valueStr);
        in >> value;
        if (!in) {
            throw ParsingError("Invalid number: " + valueStr);
        }

        auto node = std::make_unique<NumberExpr>(value);
        args_.push_back(std::move(node));
    }

    void exitBinaryOp(FormulaParser::BinaryOpContext* ctx) override {
        assert(args_.size() >= 2);

        auto rhs = std::move(args_.back());
        args_.pop_back();

        auto lhs = std::move(args_.back());

        BinaryOpExpr::Type type;
        if (ctx->ADD()) {
            type = BinaryOpExpr::Add;
        } else if (ctx->SUB()) {
            type = BinaryOpExpr::Subtract;
        } else if (ctx->MUL()) {
            type = BinaryOpExpr::Multiply;
        } else {
            assert(ctx->DIV() != nullptr);
            type = BinaryOpExpr::Divide;
        }

        auto node = std::make_unique<BinaryOpExpr>(type, std::move(lhs), std::move(rhs));
        args_.back() = std::move(node);
    }

    void visitErrorNode(antlr4::tree::ErrorNode* node) override {
        throw ParsingError("Error when parsing: " + node->getSymbol()->getText());
    }

private:
    std::vector<std::unique_ptr<Expr>> args_;
};

// Класс для обработки ошибок лексического анализа
class BailErrorListener : public antlr4::BaseErrorListener {
public:
    void syntaxError(antlr4::Recognizer* /* recognizer */, antlr4::Token* /* offendingSymbol */,
                     size_t /* line */, size_t /* charPositionInLine */, const std::string& msg,
                     std::exception_ptr /* e */
                     ) override {
        throw ParsingError("Error when lexing: " + msg);
    }
};

}  // namespace
}  // namespace ASTImpl

// Функция для парсинга AST формулы из потока ввода
FormulaAST ParseFormulaAST(std::istream& in) {
    using namespace antlr4;

    ANTLRInputStream input(in);

    FormulaLexer lexer(&input);
    ASTImpl::BailErrorListener error_listener;
    lexer.removeErrorListeners();
    lexer.addErrorListener(&error_listener);

    CommonTokenStream tokens(&lexer);

    FormulaParser parser(&tokens);
    auto error_handler = std::make_shared<BailErrorStrategy>();
    parser.setErrorHandler(error_handler);
    parser.removeErrorListeners();

    tree::ParseTree* tree = parser.main();
    ASTImpl::ParseASTListener listener;
    tree::ParseTreeWalker::DEFAULT.walk(&listener, tree);

    return FormulaAST(listener.MoveRoot());
}

// Функция для парсинга AST формулы из строки
FormulaAST ParseFormulaAST(const std::string& in_str) {
    std::istringstream in(in_str);
    try {
        return ParseFormulaAST(in);
    } catch (const std::exception& exc) {
        std::throw_with_nested(FormulaException(exc.what()));
    }
}

// Метод для печати AST в поток вывода
void FormulaAST::Print(std::ostream& out) const {
    root_expr_->Print(out);
}

// Метод для печати формулы в поток вывода
void FormulaAST::PrintFormula(std::ostream& out) const {
    root_expr_->PrintFormula(out, ASTImpl::EP_ATOM);
}

// Метод для выполнения формулы и получения результата
double FormulaAST::Execute() const {
    return root_expr_->Evaluate();
}

// Конструктор класса FormulaAST
FormulaAST::FormulaAST(std::unique_ptr<ASTImpl::Expr> root_expr)
    : root_expr_(std::move(root_expr)) {
}

// Деструктор класса FormulaAST
FormulaAST::~FormulaAST() = default;