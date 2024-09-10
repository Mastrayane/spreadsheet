#include "common.h"
#include "test_runner_p.h"

// Перегружаем оператор << для вывода объекта типа Position в поток
inline std::ostream& operator<<(std::ostream& output, Position pos) {
	return output << "(" << pos.row << ", " << pos.col << ")";
}

// Определяем литерал _pos, который преобразует строку в объект типа Position
inline Position operator"" _pos(const char* str, std::size_t) {
	return Position::FromString(str);
}

// Перегружаем оператор << для вывода объекта типа Size в поток
inline std::ostream& operator<<(std::ostream& output, Size size) {
	return output << "(" << size.rows << ", " << size.cols << ")";
}

// Перегружаем оператор << для вывода значения ячейки в поток
inline std::ostream& operator<<(std::ostream& output, const CellInterface::Value& value) {
	// Используем std::visit для вывода значения в зависимости от его типа
	std::visit(
		[&](const auto& x) {
			output << x;
		},
		value);
	return output;
}

namespace {

	// Тест на пустую таблицу
	void TestEmpty() {
		auto sheet = CreateSheet(); // Создаем новый лист
		ASSERT_EQUAL(sheet->GetPrintableSize(), (Size{ 0, 0 })); // Проверяем, что размер печатаемой области равен (0, 0)
	}

	// Тест на невалидные позиции
	void TestInvalidPosition() {
		auto sheet = CreateSheet(); // Создаем новую пустую таблицу
		try {
			sheet->SetCell(Position{ -1, 0 }, ""); // Пытаемся установить ячейку на невалидной позиции
		}
		catch (const InvalidPositionException&) {
			// Ожидаем исключение InvalidPositionException
		}
		try {
			sheet->GetCell(Position{ 0, -2 }); // Пытаемся получить ячейку на невалидной позиции
		}
		catch (const InvalidPositionException&) {
			// Ожидаем исключение InvalidPositionException
		}
		try {
			sheet->ClearCell(Position{ Position::MAX_ROWS, 0 }); // Пытаемся очистить ячейку на невалидной позиции
		}
		catch (const InvalidPositionException&) {
			// Ожидаем исключение InvalidPositionException
		}
	}

	// Тест на установку текста в ячейку
	void TestSetCellPlainText() {
		auto sheet = CreateSheet(); // Создаем новую пустую таблицу

		// Лямбда-функция для проверки установки и получения текста в ячейке
		auto checkCell = [&](Position pos, std::string text) {
			sheet->SetCell(pos, text); // Устанавливаем текст в ячейку
			CellInterface* cell = sheet->GetCell(pos); // Получаем ячейку
			ASSERT(cell != nullptr); // Проверяем, что ячейка не nullptr
			ASSERT_EQUAL(cell->GetText(), text); // Проверяем, что текст в ячейке совпадает с установленным
			ASSERT_EQUAL(std::get<std::string>(cell->GetValue()), text); // Проверяем, что значение ячейки совпадает с текстом
			};

		checkCell("A1"_pos, "Hello"); // Проверяем установку текста в ячейку A1
		checkCell("A1"_pos, "World"); // Проверяем установку текста в ячейку A1
		checkCell("B2"_pos, "Purr"); // Проверяем установку текста в ячейку B2
		checkCell("A3"_pos, "Meow"); // Проверяем установку текста в ячейку A3

		const SheetInterface& constSheet = *sheet; // Создаем константную ссылку на таблицу
		ASSERT_EQUAL(constSheet.GetCell("B2"_pos)->GetText(), "Purr"); // Проверяем текст в ячейке B2

		sheet->SetCell("A3"_pos, "'=escaped"); // Устанавливаем текст в ячейку A3
		CellInterface* cell = sheet->GetCell("A3"_pos); // Получаем ячейку A3
		ASSERT_EQUAL(cell->GetText(), "'=escaped"); // Проверяем текст в ячейке
		ASSERT_EQUAL(std::get<std::string>(cell->GetValue()), "=escaped"); // Проверяем значение ячейки
	}

	// Тест на очистку ячейки
	void TestClearCell() {
		auto sheet = CreateSheet(); // Создаем новый лист

		sheet->SetCell("C2"_pos, "Me gusta"); // Устанавливаем текст в ячейку C2
		sheet->ClearCell("C2"_pos); // Очищаем ячейку C2
		ASSERT(sheet->GetCell("C2"_pos) == nullptr); // Проверяем, что ячейка C2 очищена

		sheet->ClearCell("A1"_pos); // Очищаем ячейку A1
		sheet->ClearCell("J10"_pos); // Очищаем ячейку J10
	}

	// Тест на печать таблицы
	void TestPrint() {
		auto sheet = CreateSheet(); // Создаем новую пустую таблицу
		sheet->SetCell("A2"_pos, "meow"); // Устанавливаем текст в ячейку A2
		sheet->SetCell("B2"_pos, "=1+2"); // Устанавливаем формулу в ячейку B2
		sheet->SetCell("A1"_pos, "=1/0"); // Устанавливаем формулу в ячейку A1

		ASSERT_EQUAL(sheet->GetPrintableSize(), (Size{ 2, 2 })); // Проверяем размер печатаемой области

		std::ostringstream texts; // Создаем поток для текстов
		sheet->PrintTexts(texts); // Печатаем тексты ячеек в поток
		ASSERT_EQUAL(texts.str(), "=1/0\t\nmeow\t=1+2\n"); // Проверяем, что тексты совпадают

		std::ostringstream values; // Создаем поток для значений
		sheet->PrintValues(values); // Печатаем значения ячеек в поток
		ASSERT_EQUAL(values.str(), "#ARITHM!\t\nmeow\t3\n"); // Проверяем, что значения совпадают

		sheet->ClearCell("B2"_pos); // Очищаем ячейку B2
		ASSERT_EQUAL(sheet->GetPrintableSize(), (Size{ 2, 1 })); // Проверяем размер печатаемой области после очистки
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