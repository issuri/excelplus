#pragma once
#ifndef EXCEL_H
#define EXCEL_H
#include <ctime>
#include <string>
#include <iostream>
using namespace std;

namespace MyExcel {
    class Vector {
        string* data;
        int capacity;
        int length;

    public:
        // 생성자
        Vector(int n = 1);
        // 맨 뒤에 새로운 원소 추가
        void push_back(string s);
        // 임의의 위치 원소 접근
        string operator[](int i);
        // x 번째 위치한 원소 제거
        void remove(int x);
        // 현재 벡터의 크기
        int size();

        ~Vector();
    };

    class Stack {
        struct Node {
            Node* prev; // 자기보다 하위 노드 가리키는 포인터
            string s; // 데이터 보관 중

            Node(Node* prev, string s) : prev(prev), s(s) {}
        };
        Node* current; // 최상위 노드를 가리키는 포인터
        Node start; // 맨 아래 이루는 노드 Node(Node* prev, string s) => start(NULL,"") 

    public:
        Stack();
        // 최상단에 새로운 원소 추가
        void push(string s);
        // 최상단의 원소를 제거 후 반환
        string pop();
        // 최상단의 원소 반환. (제거 x)
        string peek();
        // 스택이 비어있는지의 유무 반환
        bool is_empty();

        ~Stack();
    };
    class NumStack {
        struct Node {
            Node* prev;
            double s;

            Node(Node* prev, double s) : prev(prev), s(s) {}
        };
        Node* current;
        Node start;

    public:
        NumStack();
        void push(double s);
        double pop();
        double peek();
        bool is_empty();

        ~NumStack();
    };

    class Table;
    // 큰 테이블에서 한 칸을 의미하는 객체로
    class Cell {
    protected:
        typedef MyExcel::Table Table;
        int x, y;
        Table* table; // 어느 테이블에 위치해 있는지에 관련된 정보

    public:
        Cell(int x, int y, Table* table);

        virtual string stringify() = 0;
        virtual int to_numeric() = 0;
    };

    // StringCell
    class StringCell :public Cell {
        string data;


    public:
        StringCell(string data, int x, int y, Table* t);

        string stringify();
        int to_numeric();

    };
    // NumberCell
    class NumberCell :public Cell {
        int data;

    public:
        string stringify();
        int to_numeric();

        NumberCell(int data, int x, int y, Table* t);
    };
    // DateCell
    class DateCell : public Cell {
        // tiem_t : 1970년 1월 1일 00:00:00로부터 시간을 초(second)단위로 카운트
        time_t data;

    public:
        DateCell(string data, int x, int y, Table* t);
        
        string stringify();
        int to_numeric();

    };

    // ExprCell
    class ExprCell :public Cell {       
        string data;
        Vector* parsed_expr;

        Vector exp_vec;

        // 우선 순위 
        int precedence(char c);
        // 중위 표기법 => 후위 표기법
        void parse_expression();

    public:
        ExprCell(string data, int x, int y, Table* t);
        ~ExprCell();

        string stringify();
        int to_numeric();
    };

    // Table 클래스는 Cell 객체들을 2차원 배열로 보관
    class Table {
    protected:
        // 행 및 열의 최대 크기
        int max_row_size, max_col_size;
        // 데이터를 보관하는 테이블
        Cell*** data_table;

    public:
        Table(int max_row_size, int max_col_size);
        ~Table();

        // 새로운 셀을 row 행 col 열에 등록
        void reg_cell(Cell* c, int row, int col);
        // 해당 셀의 정수값 반환
        // s: 셀 이름(ex. A2, B3)
        int to_numeric(const string& s);
        // 행 및 열 번호로 셀 호출
        int to_numeric(int row, int col);
        // 해당 셀의 문자열 반환
        string stringify(const string& s);
        string stringify(int row, int col);

        // 순수 가상 함수 => 정의가 이뤄지지 않고 함수만 선언
        // 이렇게 선언된 순수 가상 함수가 있다면 이를 추상 클래스라 부름
        virtual string print_table() = 0;
    };

    // virtual string print_table() = 0; 
    // 순수가상함수를 포함한 Table 클래스는 추상클래스로 Table 클래스의 객체 생성할 수 없음
    // Txt 테이블
    class TxtTable : public Table {
        string repeat_char(int n, char c);
        // 숫자로 된 열 번호를 A, B, .... Z, AA, AB, ...  이런 순으로 매겨준다.
        string col_num_to_str(int n);
    public:
        TxtTable(int row, int col);

        // 텍스트로 표를 깨끗하게 출력
        string print_table();
    };

    // HTML 테이블
    class HtmlTable : public Table {
    public:
        HtmlTable(int row, int col);

        string print_table();
    };

    // CSV 테이블
    class CSVTable : public Table {
    public:
        CSVTable(int row, int col);

        string print_table();
    };

    //
}

#endif
