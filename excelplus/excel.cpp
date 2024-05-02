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
	// Cell Ŭ����(�߻�_) 
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
		// time_t ������ ��ȯ�� ��¥
		int year = atoi(s.c_str());
		int month = atoi(s.c_str()+5);
		int day = atoi(s.c_str()+8);

		// struct tm(tm ����ü) : ����� ����ϴ� ��¥ ���� ������ ����ü
		tm timeinfo;

		timeinfo.tm_year = year - 1900;
		timeinfo.tm_mon = month - 1;
		timeinfo.tm_mday = day;
		timeinfo.tm_hour = 0;
		timeinfo.tm_min = 0;
		timeinfo.tm_sec = 0;

		// tm -> time_t ������ ��ȯ
		data = mktime(&timeinfo); 
	}

	string DateCell::stringify() {
		char buf[50];
		tm temp;
		// localtime_s : ���� ������ ��
		// time_t ������ ����Ͻú��� ���·� ������ �Ͻ�(struct tm ����)�� ��ȯ
		// ����� �ִ� tm ����ü�� ���� ������ ��ȯ
		localtime_s(&temp, &data);
		// string���� ��ȯ
		// %F => ISO ��¥ ����(%Y-%m-%d�� ����).
		strftime(buf, 50, "%F", &temp);

		return string(buf);
	}

	int DateCell::to_numeric() {
		// Ÿ�� ĳ��Ʈ ������
		return static_cast<int>(data);
	}

	ExprCell::ExprCell(string data, int x, int y, Table* t)
		: data(data), Cell(x, y, t), parsed_expr(new MyExcel::Vector()) {
		// �����ڿ��� ������ �Ľ��Ͽ� exp_vec �� ����
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
		
		// ���� ��ü�� ()�� ���� exp_vec �� �����ִ� �����ڵ��� push �ǵ���... (data ��)
		data.insert(0, "(");
		data.push_back(')');		

		for (int i = 0; i < data.length(); i++) {
			// isalpha : ���ĺ����� => ����ڸ� 1, �ҹ��ڸ� 2, �ƴϸ� 0 ��ȯ
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
		// parsed_expr �� exp_vec �ּ� �Ҵ� =================================================================================>>>>
		// std::string* Myexcel::ExprCell << Myexcel::Vector Myexcel::ExprCell		
		// ==> parsed_expr�� Vector* �� �ٲ� (excel.h Ȯ��)
		
		parsed_expr = &exp_vec;
	}

	// ���� ǥ��� ����
	int ExprCell::to_numeric() {
		// double result = 0;
		NumStack stack;

		for (int i = 0; i < parsed_expr->size(); i++) {
			string s = (*parsed_expr)[i];

			// �� �� ���(������ ��x), isalpha : ���ĺ����� => ����ڸ� 1, �ҹ��ڸ� 2, �ƴϸ� 0 ��ȯ
			if (isalpha(s[0])) {
				stack.push(table->to_numeric(s));
			}
			// ����(�ǿ�����)�� ��� (�� �ڸ��� ����), isdigit : �������� => ���ڸ� 0�� �ƴ� ��, ���� => 0
			else if (isdigit(s[0])) {
				stack.push(atoi(s.c_str()));
			}
			// �������� ���
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
		//result = stack.pop(); // ���� ��		
		//return result;
		return stack.pop(); 
	}

	string ExprCell::stringify() {
		return this->data;
	}	


	// (�����Ҵ�����) Cell* �迭 ���� �� Cell ��ü �ʿ� �� �� ���� ���� 
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

		// �ش� ��ġ�� �̹� �ٸ� �� ��ü�� ��ϵǾ� ���� ��� ����
		if (data_table[row][col]) {
			delete data_table[row][col];
		}
		// ���
		data_table[row][col] = c;
	}

	int Table::to_numeric(const string& s) {
		// Cell �̸����� �޴´�.
		int col = s[0] - 'A';
		// atoi : ���ڿ��� ���� Ÿ������, c_str() : string�� ù��° ������ �ּ� ��ȯ
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
		// Cell �̸����� �޴´�. 
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
	// ostream Ŭ������ << ������ �����ε��ϴ� �Լ�
	std::ostream& operator<<(std::ostream& o, Table& table) {
		o << table.print_table();
		return o;
	}

	// Table Ŭ������ ��ӹ޴� Ŭ����
	// TxtTable�� �����ڿ��� Table�� �����ڿ� ���ڸ� ������ �� ����
	TxtTable::TxtTable(int row, int col) : Table(row, col) {}

	// �ؽ�Ʈ�� ǥ�� �����ϰ� ���
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

		// �� ��ܿ� �� ���� ǥ��
		total_table += "    "; // ���� ��� ���� ���
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
		// �ϴ� �⺻������ �ִ� 9999 ��° �� ���� �����Ѵٰ� ����
		for (int i = 0; i < max_row_size; i++) {
			total_table += repeat_char(total_wide, '-'); // ---------------
			total_table += "\n" + to_string(i + 1); // �� ��ȣ : 1,2,3,4
			total_table += repeat_char(4 - to_string(i + 1).length(), ' '); // 1,2,3,4 �ڴ� ���� ���

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

	// ���ڿ� �ݺ�
	// repeat_char(total_wide, '-'); => total_wide ��ŭ '-' ���
	string TxtTable::repeat_char(int n, char c) {
		string s = "";
		for (int i = 0; i < n; i++) s.push_back(c);

		return s;
	}
	// ���ڷ� �� �� ��ȣ�� A, B, .... Z, AA, AB, ...  �̷� ������ �Ű��ش�.
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

	// HTML ���̺�
	HtmlTable::HtmlTable(int row, int col) : Table(row, col) {}

	string HtmlTable::print_table() {
		string s = "<table border='1' cellpadding='10'>";
		for (int i = 0; i < max_row_size; i++) {
			s += "<tr>"; // ���� ���� ������ �ļ� ��ġ�ǵ���(max_row_size ��ŭ �� ����)
			for (int j = 0; j < max_col_size; j++) {
				// �� �࿡ �� ��ġ(max_col_size ��ŭ ����)
				s += "<td>";
				if (data_table[i][j]) {
					// ������ �� ������ �ش� �� ���
					s += data_table[i][j]->stringify();
				}
				s += "</td>";
			}
			s += "</tr>";
		}
		s += "< /table>";
		return s;
	}

	// CSV ���̺�
	CSVTable::CSVTable(int row, int col) :Table(row, col) {}

	string CSVTable::print_table() {
		string s = "";
		for (int i = 0; i < max_row_size; i++) {
			for (int j = 0; j < max_col_size; j++) {
				if (j >= 1) s += ",";
				// CSV ���� ��Ģ�� ���� ���ڿ��� ū����ǥ�� ������ "" ���� ġȯ
				string temp;
				if (data_table[i][j]) {
					temp = data_table[i][j]->stringify();
				}

				for (int k = 0; k < temp.length(); k++) {
					if (temp[k] == '"') {
						// k ��ġ�� " �� �� �� �� �ִ´�.
						temp.insert(k, 1, '"');
						// �̹� �߰��� " �� �ٽ� Ȯ���ϴ� ���� ���� �ϱ� ���� k�� �� ĭ �� �̵�
						k++;
					}
				}
				temp = '"' + temp + '"';
				s += temp; // s => "string ��"
			}
			s += '\n';
		}
		return s;
	}

	//
}

int main() {
	// ***** TxtTable ��ü�� ������ �� ���ڷ� ���޵Ǵ� ���� Table Ŭ������ �����ڿ� ���޵Ǿ�� ��
	MyExcel::Table* table = new MyExcel::TxtTable(5, 5);

	table->reg_cell(new MyExcel::NumberCell(2, 1, 1, table), 1, 1);
	table->reg_cell(new MyExcel::NumberCell(3, 1, 2, table), 1, 2);
	table->reg_cell(new MyExcel::NumberCell(4, 2, 1, table), 2, 1);
	table->reg_cell(new MyExcel::NumberCell(5, 2, 2, table), 2, 2);

	// �ش� ������ ��� ��
	table->reg_cell(new MyExcel::ExprCell("B2+B3*(C2+C3-2)", 3, 3, table), 3, 2);

	// ������ �ƴ� ���� ��� ����� ����ϵ��� ����
	// �ش� ���� ���� ���� ������ Ȯ���ϰ�, ������ ����� ����� ���



	table->reg_cell(new MyExcel::StringCell("B2+B3*(C2+C3-2)=", 3, 2, table), 3, 1);
	
	std::cout << std::endl << *table;

	return 0;
}
