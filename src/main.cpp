#include "common.h"
#include "test_runner_p.h"

// ����������� �������� << ��� ������ ������� ���� Position � �����
inline std::ostream& operator<<(std::ostream& output, Position pos) {
	return output << "(" << pos.row << ", " << pos.col << ")";
}

// ���������� ������� _pos, ������� ����������� ������ � ������ ���� Position
inline Position operator"" _pos(const char* str, std::size_t) {
	return Position::FromString(str);
}

// ����������� �������� << ��� ������ ������� ���� Size � �����
inline std::ostream& operator<<(std::ostream& output, Size size) {
	return output << "(" << size.rows << ", " << size.cols << ")";
}

// ����������� �������� << ��� ������ �������� ������ � �����
inline std::ostream& operator<<(std::ostream& output, const CellInterface::Value& value) {
	// ���������� std::visit ��� ������ �������� � ����������� �� ��� ����
	std::visit(
		[&](const auto& x) {
			output << x;
		},
		value);
	return output;
}

namespace {

	// ���� �� ������ �������
	void TestEmpty() {
		auto sheet = CreateSheet(); // ������� ����� ����
		ASSERT_EQUAL(sheet->GetPrintableSize(), (Size{ 0, 0 })); // ���������, ��� ������ ���������� ������� ����� (0, 0)
	}

	// ���� �� ���������� �������
	void TestInvalidPosition() {
		auto sheet = CreateSheet(); // ������� ����� ������ �������
		try {
			sheet->SetCell(Position{ -1, 0 }, ""); // �������� ���������� ������ �� ���������� �������
		}
		catch (const InvalidPositionException&) {
			// ������� ���������� InvalidPositionException
		}
		try {
			sheet->GetCell(Position{ 0, -2 }); // �������� �������� ������ �� ���������� �������
		}
		catch (const InvalidPositionException&) {
			// ������� ���������� InvalidPositionException
		}
		try {
			sheet->ClearCell(Position{ Position::MAX_ROWS, 0 }); // �������� �������� ������ �� ���������� �������
		}
		catch (const InvalidPositionException&) {
			// ������� ���������� InvalidPositionException
		}
	}

	// ���� �� ��������� ������ � ������
	void TestSetCellPlainText() {
		auto sheet = CreateSheet(); // ������� ����� ������ �������

		// ������-������� ��� �������� ��������� � ��������� ������ � ������
		auto checkCell = [&](Position pos, std::string text) {
			sheet->SetCell(pos, text); // ������������� ����� � ������
			CellInterface* cell = sheet->GetCell(pos); // �������� ������
			ASSERT(cell != nullptr); // ���������, ��� ������ �� nullptr
			ASSERT_EQUAL(cell->GetText(), text); // ���������, ��� ����� � ������ ��������� � �������������
			ASSERT_EQUAL(std::get<std::string>(cell->GetValue()), text); // ���������, ��� �������� ������ ��������� � �������
			};

		checkCell("A1"_pos, "Hello"); // ��������� ��������� ������ � ������ A1
		checkCell("A1"_pos, "World"); // ��������� ��������� ������ � ������ A1
		checkCell("B2"_pos, "Purr"); // ��������� ��������� ������ � ������ B2
		checkCell("A3"_pos, "Meow"); // ��������� ��������� ������ � ������ A3

		const SheetInterface& constSheet = *sheet; // ������� ����������� ������ �� �������
		ASSERT_EQUAL(constSheet.GetCell("B2"_pos)->GetText(), "Purr"); // ��������� ����� � ������ B2

		sheet->SetCell("A3"_pos, "'=escaped"); // ������������� ����� � ������ A3
		CellInterface* cell = sheet->GetCell("A3"_pos); // �������� ������ A3
		ASSERT_EQUAL(cell->GetText(), "'=escaped"); // ��������� ����� � ������
		ASSERT_EQUAL(std::get<std::string>(cell->GetValue()), "=escaped"); // ��������� �������� ������
	}

	// ���� �� ������� ������
	void TestClearCell() {
		auto sheet = CreateSheet(); // ������� ����� ����

		sheet->SetCell("C2"_pos, "Me gusta"); // ������������� ����� � ������ C2
		sheet->ClearCell("C2"_pos); // ������� ������ C2
		ASSERT(sheet->GetCell("C2"_pos) == nullptr); // ���������, ��� ������ C2 �������

		sheet->ClearCell("A1"_pos); // ������� ������ A1
		sheet->ClearCell("J10"_pos); // ������� ������ J10
	}

	// ���� �� ������ �������
	void TestPrint() {
		auto sheet = CreateSheet(); // ������� ����� ������ �������
		sheet->SetCell("A2"_pos, "meow"); // ������������� ����� � ������ A2
		sheet->SetCell("B2"_pos, "=1+2"); // ������������� ������� � ������ B2
		sheet->SetCell("A1"_pos, "=1/0"); // ������������� ������� � ������ A1

		ASSERT_EQUAL(sheet->GetPrintableSize(), (Size{ 2, 2 })); // ��������� ������ ���������� �������

		std::ostringstream texts; // ������� ����� ��� �������
		sheet->PrintTexts(texts); // �������� ������ ����� � �����
		ASSERT_EQUAL(texts.str(), "=1/0\t\nmeow\t=1+2\n"); // ���������, ��� ������ ���������

		std::ostringstream values; // ������� ����� ��� ��������
		sheet->PrintValues(values); // �������� �������� ����� � �����
		ASSERT_EQUAL(values.str(), "#ARITHM!\t\nmeow\t3\n"); // ���������, ��� �������� ���������

		sheet->ClearCell("B2"_pos); // ������� ������ B2
		ASSERT_EQUAL(sheet->GetPrintableSize(), (Size{ 2, 1 })); // ��������� ������ ���������� ������� ����� �������
	}

}  // namespace

int main() {
	TestRunner tr; 
	RUN_TEST(tr, TestEmpty); 
	RUN_TEST(tr, TestInvalidPosition); 
	RUN_TEST(tr, TestSetCellPlainText); 
	RUN_TEST(tr, TestClearCell); 
	RUN_TEST(tr, TestPrint); 
}