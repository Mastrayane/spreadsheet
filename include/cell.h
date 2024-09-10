#pragma once

#include "common.h"
#include "formula.h"

//  ласс ячейка
class Cell : public CellInterface {
public:
    
    Cell();
    ~Cell();

    // ћетод дл€ установки значени€ €чейки
    void Set(std::string text);

    // ћетод дл€ очистки значени€ €чейки
    void Clear();

    // ћетод дл€ получени€ значени€ €чейки (переопределенный из интерфейса)
    Value GetValue() const override;

    // ћетод дл€ получени€ текста €чейки (переопределенный из интерфейса)
    std::string GetText() const override;

private:
    // јбстрактный базовый класс дл€ реализации различных типов €чеек
    class Impl {
    public:
        // „исто виртуальный метод дл€ получени€ значени€ €чейки
        virtual Value GetValue() const = 0;

        // „исто виртуальный метод дл€ получени€ текста €чейки
        virtual std::string GetText() const = 0;

    protected:
        // «начение €чейки
        Value value_;

        // “екст €чейки
        std::string text_;
    };

    // –еализаци€ дл€ пустой €чейки
    class EmptyImpl : public Impl {
    public:
        //  онструктор по умолчанию, инициализирует значение и текст пустыми строками
        EmptyImpl() {
            value_ = text_ = "";
        }

        // ћетод дл€ получени€ значени€ €чейки
        Value GetValue() const override {
            return value_;
        }

        // ћетод дл€ получени€ текста €чейки
        std::string GetText() const override {
            return text_;
        }
    };

    // –еализаци€ дл€ текстовой €чейки
    class TextImpl : public Impl {
    public:
        //  онструктор, принимающий текст €чейки
        TextImpl(std::string_view text) {
            text_ = text;
            // ≈сли текст начинаетс€ с апострофа, удал€ем его
            if (text[0] == '\'') {
                text = text.substr(1);
            }
            value_ = std::string(text);
        }

        // ћетод дл€ получени€ значени€ €чейки
        Value GetValue() const override {
            return value_;
        }

        // ћетод дл€ получени€ текста €чейки
        std::string GetText() const override {
            return text_;
        }
    };

    // –еализаци€ дл€ €чейки с формулой
    class FormulaImpl : public Impl {
    public:
        //  онструктор, принимающий выражение формулы
        FormulaImpl(std::string_view expression) {
            // ”дал€ем знак '=' в начале выражени€
            expression = expression.substr(1);
            value_ = std::string(expression);
            // ѕарсим формулу и сохран€ем указатель на объект формулы
            formula_ptr_ = ParseFormula(std::move(std::string(expression)));
            text_ = "=" + formula_ptr_->GetExpression();
        }

        // ћетод дл€ получени€ значени€ €чейки
        Value GetValue() const override {
            // ¬ычисл€ем значение формулы
            auto value = formula_ptr_->Evaluate();
            // ≈сли значение €вл€етс€ числом, возвращаем его
            if (std::holds_alternative<double>(value)) {
                return std::get<double>(value);
            }
            // »наче возвращаем ошибку формулы
            return std::get<FormulaError>(value);
        }

        // ћетод дл€ получени€ текста €чейки
        std::string GetText() const override {
            return text_;
        }

    private:
        // ”казатель на объект формулы
        std::unique_ptr<FormulaInterface> formula_ptr_;
    };

    // ”казатель на конкретную реализацию €чейки
    std::unique_ptr<Impl> impl_;
};