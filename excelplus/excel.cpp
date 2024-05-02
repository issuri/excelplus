#include "excel.h"
#include <fstream>

namespace MyExcel {
	Vector::Vector(int n) : data(new string[n]), capacity(n), length(0) {}
	void Vector::push_back(string s) {
		if (capacity <= length) {
			string* tmp = new string[capacity * 2];
			for (int i = 0; i < length; i++) {
				tmp[i] = data[i];
			}
			delete[] data;
			data = tmp;
			capacity *= 2;
		}
		data[length] = s;
		length++;
	}
	string Vector::operator[](int i) { return data[i]; }
	void Vector::remove(int x) {
		for (int i = x + 1; i < length; i++) {
			data[i - 1] = data[i];
		}
		length--;
	}
	int Vector::size() { return length; }
	Vector::~Vector() {
		if (data) {
			delete[] data;
		}
	}

	Stack::Stack() : start(NULL, "") { current = &start; }
	void Stack::push(string s) {
		Node* n = new Node(current, s);
		current = n;
	}
	string Stack::pop() {
		if (current == &start)
			return "";

		string s = current->s;
		Node* prev = current;
		current = current->prev;

		delete prev;
		return s;
	}
	string Stack::peek() { return current->s; }
	bool Stack::is_empty() {
		if (current == &start)
			return true;
		return false;
	}
	Stack::~Stack() {
		while (current != &start) {
			Node* prev = current;
			current = current->prev;
			delete prev;
		}
	}
	NumStack::NumStack() : start(NULL, 0) { current = &start; }
	void NumStack::push(double s) {
		Node* n = new Node(current, s);
		current = n;
	}
	double NumStack::pop() {
		if (current == &start)
			return 0;

		double s = current->s;
		Node* prev = current;
		current = current->prev;

		delete prev;
		return s;
	}
	double NumStack::peek() { return current->s; }
	bool NumStack::is_empty() {
		if (current == &start)
			return true;
		return false;
	}
	NumStack::~NumStack() {
		while (current != &start) {
			Node* prev = current;
			current = current->prev;
			delete prev;
		}
	}

	// Myexcel
	// Cell 클래스(추상_) 
	Cell::Cell(int x, int y, Table* table)
		: x(x), y(y), table(table) {}

	// StringCell
	StringCell::StringCell(string data, int x, int y, Table* t)
		: data(data), Cell(x, y, t) {}
	string StringCell::stringify() { return data; }
	int StringCell::to_numeric() { return 0; }

	// NumberCell
	NumberCell::NumberCell(int data, int x, int y, Table* t)
		: data(data), Cell(x, y, t) {}
	string NumberCell::stringify() { return to_string(data); }
	int NumberCell::to_numeric() { return data; }

	// *** DateCell
	DateCell::DateCell(string s, int x, int y, Table* t) : Cell(x, y, t) {
		// Date format => yyyy-mm-dd
		// time_t 값으로 변환할 날짜
		int year = atoi(s.c_str());
		int month = atoi(s.c_str()+5);
		int day = atoi(s.c_str()+8);

		// struct tm(tm 구조체) : 사람이 사용하는 날짜 개념 적용한 구조체
		tm timeinfo;

		timeinfo.tm_year = year - 1900;
		timeinfo.tm_mon = month - 1;
		timeinfo.tm_mday = day;
		timeinfo.tm_hour = 0;
		timeinfo.tm_min = 0;
		timeinfo.tm_sec = 0;

		// tm -> time_t 값으로 변환
		data = mktime(&timeinfo); 
	}

	string DateCell::stringify() {
		char buf[50];
		tm temp;
		// localtime_s : 실행 시점의 값
		// time_t 형식을 년월일시분초 형태로 분할한 일시(struct tm 형식)로 변환
		// 멤버가 있는 tm 구조체에 대한 포인터 반환
		localtime_s(&temp, &data);
		// string으로 반환
		// %F => ISO 날짜 형식(%Y-%m-%d와 동일).
		strftime(buf, 50, "%F", &temp);

		return string(buf);
	}

	int DateCell::to_numeric() {
		// 타입 캐스트 연산자
		return static_cast<int>(data);
	}

	ExprCell::ExprCell(string data, int x, int y, Table* t)
		: data(data), Cell(x, y, t), parsed_expr(new MyExcel::Vector()) {
		// 생성자에서 수식을 파싱하여 exp_vec 에 저장
		parse_expression();
	}

	ExprCell::~ExprCell() {
		if (parsed_expr) {
			delete[] parsed_expr;
			parsed_expr = nullptr;
		}
	}

	int ExprCell::precedence(char c) {
		switch (c) {
		case '(':
		case '[':
		case '{':
			return 0;
		case '+':
		case '-':
			return 1;
		case '*':
		case '/':
			return 2;
		}
		return 0;
	}

	void ExprCell::parse_expression() {
		Stack stack;
		
		// 수식 전체를 ()로 묶고 exp_vec 에 남아있는 연산자들이 push 되도록... (data 값)
		data.insert(0, "(");
		data.push_back(')');		

		for (int i = 0; i < data.length(); i++) {
			// isalpha : 알파벳인지 => 대분자면 1, 소문자면 2, 아니면 0 반환
			if (isalpha(data[i])) {
				exp_vec.push_back(data.substr(i, 2));
				i++;
			}
			else if (isdigit(data[i])) {
				exp_vec.push_back(data.substr(i, 1));
			}
			else if (data[i] == '(' || data[i] == '[' || data[i] == '{') {
				stack.push(data.substr(i, 1));
			}
			else if (data[i] == ')' || data[i] == ']' || data[i] == '}') {
				string t = stack.pop();
				while (t != "(" && t != "[" && t != "{") {
					exp_vec.push_back(t);
					t = stack.pop();
				}
			}
			else if (data[i] == '+' || data[i] == '-' || data[i] == '*' || data[i] == '/') {
				while (!stack.is_empty() && precedence(stack.peek()[0]) >= precedence(data[i])) {
					exp_vec.push_back(stack.pop());
				}
				stack.push(data.substr(i, 1));
			}
		}		
		// parsed_expr 에 exp_vec 주소 할당 =================================================================================>>>>
		// std::string* Myexcel::ExprCell << Myexcel::Vector Myexcel::ExprCell		
		// ==> parsed_expr를 Vector* 로 바꿈 (excel.h 확인)
		
		parsed_expr = &exp_vec;
	}

	// 후위 표기법 계산식
	int ExprCell::to_numeric() {
		// double result = 0;
		NumStack stack;

		for (int i = 0; i < parsed_expr->size(); i++) {
			string s = (*parsed_expr)[i];

			// 셀 일 경우(데이터 값x), isalpha : 알파벳인지 => 대분자면 1, 소문자면 2, 아니면 0 반환
			if (isalpha(s[0])) {
				stack.push(table->to_numeric(s));
			}
			// 숫자(피연산자)일 경우 (한 자리라 가정), isdigit : 숫자인지 => 숫자면 0이 아닌 수, 문자 => 0
			else if (isdigit(s[0])) {
				stack.push(atoi(s.c_str()));
			}
			// 연산자일 경우
			else {
				double y = stack.pop();
				double x = stack.pop();
				switch (s[0]) {
				case '+':
					stack.push(x + y);
					break;
				case '-':
					stack.push(x - y);
					break;
				case '*':
					stack.push(x * y);
					break;
				case '/':
					stack.push(x / y);
					break;
				}
			}
		}
		//result = stack.pop(); // 연산 값		
		//return result;
		return stack.pop(); 
	}

	string ExprCell::stringify() {
		return this->data;
	}	


	// (동적할당으로) Cell* 배열 생성 후 Cell 객체 필요 할 때 마다 생성 
	Table::Table(int max_row_size, int max_col_size)
		: max_row_size(max_row_size), max_col_size(max_col_size) {
		data_table = new Cell * *[max_row_size];
		for (int i = 0; i < max_row_size; i++) {
			data_table[i] = new Cell * [max_col_size];
			for (int j = 0; j < max_col_size; j++) {
				data_table[i][j] = NULL;
			}
		}
	}

	Table::~Table() {
		for (int i = 0; i < max_row_size; i++) {
			for (int j = 0; j < max_col_size; j++) {
				if (data_table[i][j])
					delete data_table[i][j];
			}
		}
		for (int i = 0; i < max_row_size; i++) {
			delete[] data_table[i];
		}
		delete[] data_table;
	}

	void Table::reg_cell(Cell* c, int row, int col) {
		if (!(row < max_row_size && col < max_col_size))
			return;

		// 해당 위치에 이미 다른 셀 객체가 등록되어 있을 경우 삭제
		if (data_table[row][col]) {
			delete data_table[row][col];
		}
		// 등록
		data_table[row][col] = c;
	}

	int Table::to_numeric(const string& s) {
		// Cell 이름으로 받는다.
		int col = s[0] - 'A';
		// atoi : 문자열을 정수 타입으로, c_str() : string의 첫번째 문자의 주소 반환
		int row = atoi(s.c_str() + 1) - 1;

		if (row < max_row_size && col < max_col_size) {
			if (data_table[row][col]) {
				return data_table[row][col]->to_numeric();
			}
		}
		return 0;
	}
	int Table::to_numeric(int row, int col) {
		if (row < max_row_size && col < max_col_size && data_table[row][col]) {
			return data_table[row][col]->to_numeric();
		}
		return 0;
	}

	string Table::stringify(const string& s) {
		// Cell 이름으로 받는다. 
		// ex.A8 => 0, B3 => 1
		int col = s[0] - 'A';
		// ex.A8 : s.c_str() => A, s.c_str()+1 => 8
		//         int row = 7   
		int row = atoi(s.c_str() + 1) - 1;

		if (row < max_row_size && col < max_col_size) {
			if (data_table[row][col]) {
				return data_table[row][col]->stringify();
			}
		}
		return "";
	}
	string Table::stringify(int row, int col) {
		if (row < max_row_size && col < max_col_size && data_table[row][col]) {
			return data_table[row][col]->stringify();
		}
		return "";
	}
	// ostream 클래스의 << 연산자 오버로딩하는 함수
	std::ostream& operator<<(std::ostream& o, Table& table) {
		o << table.print_table();
		return o;
	}

	// Table 클래스를 상속받는 클래스
	// TxtTable의 생성자에서 Table의 생성자에 인자를 전달할 수 있음
	TxtTable::TxtTable(int row, int col) : Table(row, col) {}

	// 텍스트로 표를 깨끗하게 출력
	string TxtTable::print_table() {
		string total_table;

		int* col_max_wide = new int[max_col_size];
		for (int i = 0; i < max_col_size; i++) {
			unsigned int max_wide = 2;
			for (int j = 0; j < max_row_size; j++) {
				if (data_table[j][i] &&
					data_table[j][i]->stringify().length() > max_wide) {
					max_wide = data_table[j][i]->stringify().length();
				}
			}
			col_max_wide[i] = max_wide;
		}

		// 맨 상단에 열 정보 표시
		total_table += "    "; // 왼쪽 상단 공백 출력
		int total_wide = 4;
		for (int i = 0; i < max_col_size; i++) {
			if (col_max_wide[i]) {
				int max_len = max(2, col_max_wide[i]);
				total_table += " | " + col_num_to_str(i);
				total_table += repeat_char(max_len - col_num_to_str(i).length(), ' ');

				total_wide += (max_len + 3);
			}
		}

		total_table += "\n";
		// 일단 기본적으로 최대 9999 번째 행 까지 지원한다고 생각
		for (int i = 0; i < max_row_size; i++) {
			total_table += repeat_char(total_wide, '-'); // ---------------
			total_table += "\n" + to_string(i + 1); // 행 번호 : 1,2,3,4
			total_table += repeat_char(4 - to_string(i + 1).length(), ' '); // 1,2,3,4 뒤는 공백 출력

			for (int j = 0; j < max_col_size; j++) {
				if (col_max_wide[j]) {
					int max_len = max(2, col_max_wide[j]);
					string s = "";
					if (data_table[i][j]) {
						s = data_table[i][j]->stringify();
					}
					total_table += " | " + s;
					total_table += repeat_char(max_len - s.length(), ' ');
				}
			}
			total_table += "\n";
		}
		return total_table;
	}

	// 문자열 반복
	// repeat_char(total_wide, '-'); => total_wide 만큼 '-' 출력
	string TxtTable::repeat_char(int n, char c) {
		string s = "";
		for (int i = 0; i < n; i++) s.push_back(c);

		return s;
	}
	// 숫자로 된 열 번호를 A, B, .... Z, AA, AB, ...  이런 순으로 매겨준다.
	string TxtTable::col_num_to_str(int n) {
		string s = "";
		if (n < 26) {
			s.push_back('A' + n); // 'A'+1 = 'B'
		}
		else {
			char first = 'A' + n / 26 - 1; // 'A' ~ 'Z'('A'+25 )
			char second = 'A' + n % 26; // first'A' ~ first'Z'

			s.push_back(first);
			s.push_back(second);
		}
		return s;
	}

	// HTML 테이블
	HtmlTable::HtmlTable(int row, int col) : Table(row, col) {}

	string HtmlTable::print_table() {
		string s = "<table border='1' cellpadding='10'>";
		for (int i = 0; i < max_row_size; i++) {
			s += "<tr>"; // 셀이 다음 행으로 후속 배치되도록(max_row_size 만큼 행 생성)
			for (int j = 0; j < max_col_size; j++) {
				// 한 행에 셀 배치(max_col_size 만큼 생성)
				s += "<td>";
				if (data_table[i][j]) {
					// 데이터 값 있으면 해당 값 출력
					s += data_table[i][j]->stringify();
				}
				s += "</td>";
			}
			s += "</tr>";
		}
		s += "< /table>";
		return s;
	}

	// CSV 테이블
	CSVTable::CSVTable(int row, int col) :Table(row, col) {}

	string CSVTable::print_table() {
		string s = "";
		for (int i = 0; i < max_row_size; i++) {
			for (int j = 0; j < max_col_size; j++) {
				if (j >= 1) s += ",";
				// CSV 파일 규칙에 따라 문자열에 큰따옴표가 있으면 "" 으로 치환
				string temp;
				if (data_table[i][j]) {
					temp = data_table[i][j]->stringify();
				}

				for (int k = 0; k < temp.length(); k++) {
					if (temp[k] == '"') {
						// k 위치에 " 를 한 개 더 넣는다.
						temp.insert(k, 1, '"');
						// 이미 추가된 " 를 다시 확인하는 일이 없게 하기 위해 k를 한 칸 더 이동
						k++;
					}
				}
				temp = '"' + temp + '"';
				s += temp; // s => "string 값"
			}
			s += '\n';
		}
		return s;
	}

	//
}

int main() {
	// ***** TxtTable 객체를 생성할 때 인자로 전달되는 값이 Table 클래스의 생성자에 전달되어야 함
	MyExcel::Table* table = new MyExcel::TxtTable(5, 5);

	table->reg_cell(new MyExcel::NumberCell(2, 1, 1, table), 1, 1);
	table->reg_cell(new MyExcel::NumberCell(3, 1, 2, table), 1, 2);
	table->reg_cell(new MyExcel::NumberCell(4, 2, 1, table), 2, 1);
	table->reg_cell(new MyExcel::NumberCell(5, 2, 2, table), 2, 2);

	// 해당 셀들의 계산 값
	table->reg_cell(new MyExcel::ExprCell("B2+B3*(C2+C3-2)", 3, 3, table), 3, 2);

	// 수식이 아닌 실제 계산 결과를 출력하도록 수정
	// 해당 셀의 값이 수식 셀인지 확인하고, 수식을 계산한 결과로 출력



	table->reg_cell(new MyExcel::StringCell("B2+B3*(C2+C3-2)=", 3, 2, table), 3, 1);
	
	std::cout << std::endl << *table;

	return 0;
}
