#include "formula.h"

#include "FormulaAST.h"

#include <algorithm>
#include <cassert>
#include <cctype>
#include <sstream>

using namespace std::literals;

// ���������� ��������� ������ ��� FormulaError
std::ostream& operator<<(std::ostream& output, FormulaError fe) {
	return output << "#ARITHM!";
}

namespace {
	// ����� Formula - ������� ��� ������ FormulaInterface, �������������� ������ � ��������
	class Formula : public FormulaInterface {
	public:
		explicit Formula(std::string expression)
			: ast_(ParseFormulaAST(std::move(expression))) {}

		// ����� ��� ���������� �������� �������
		Value Evaluate() const override {
			try {
				return ast_.Execute();
			}
			catch (const FormulaError& fe) {
				return fe;
			}
		}

		// ����� ��� ��������� ���������� ������������� �������
		std::string GetExpression() const override {
			std::ostringstream out;
			ast_.PrintFormula(out);
			return out.str();
		}

	private:
		// ������ AST �������
		FormulaAST ast_;
	};

}  // namespace

// ������� ��� �������� ������ ��������� � �������� ������� �������
std::unique_ptr<FormulaInterface> ParseFormula(std::string expression) {
	return std::make_unique<Formula>(std::move(expression));
}