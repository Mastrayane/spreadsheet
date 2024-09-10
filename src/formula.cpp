#include "formula.h"

#include "FormulaAST.h"

#include <algorithm>
#include <cassert>
#include <cctype>
#include <sstream>

using namespace std::literals;

// Перегрузка оператора вывода для FormulaError
std::ostream& operator<<(std::ostream& output, FormulaError fe) {
	return output << "#ARITHM!";
}

namespace {
	// Класс Formula - обертка для класса FormulaInterface, обрабатывающая ошибки в формулах
	class Formula : public FormulaInterface {
	public:
		explicit Formula(std::string expression)
			: ast_(ParseFormulaAST(std::move(expression))) {}

		// Метод для вычисления значения формулы
		Value Evaluate() const override {
			try {
				return ast_.Execute();
			}
			catch (const FormulaError& fe) {
				return fe;
			}
		}

		// Метод для получения строкового представления формулы
		std::string GetExpression() const override {
			std::ostringstream out;
			ast_.PrintFormula(out);
			return out.str();
		}

	private:
		// Объект AST формулы
		FormulaAST ast_;
	};

}  // namespace

// Функция для парсинга строки выражения и создания объекта формулы
std::unique_ptr<FormulaInterface> ParseFormula(std::string expression) {
	return std::make_unique<Formula>(std::move(expression));
}