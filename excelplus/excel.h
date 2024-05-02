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
        // ������
        Vector(int n = 1);
        // �� �ڿ� ���ο� ���� �߰�
        void push_back(string s);
        // ������ ��ġ ���� ����
        string operator[](int i);
        // x ��° ��ġ�� ���� ����
        void remove(int x);
        // ���� ������ ũ��
        int size();

        ~Vector();
    };

    class Stack {
        struct Node {
            Node* prev; // �ڱ⺸�� ���� ��� ����Ű�� ������
            string s; // ������ ���� ��

            Node(Node* prev, string s) : prev(prev), s(s) {}
        };
        Node* current; // �ֻ��� ��带 ����Ű�� ������
        Node start; // �� �Ʒ� �̷�� ��� Node(Node* prev, string s) => start(NULL,"") 

    public:
        Stack();
        // �ֻ�ܿ� ���ο� ���� �߰�
        void push(string s);
        // �ֻ���� ���Ҹ� ���� �� ��ȯ
        string pop();
        // �ֻ���� ���� ��ȯ. (���� x)
        string peek();
        // ������ ����ִ����� ���� ��ȯ
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
    // ū ���̺��� �� ĭ�� �ǹ��ϴ� ��ü��
    class Cell {
    protected:
        typedef MyExcel::Table Table;
        int x, y;
        Table* table; // ��� ���̺� ��ġ�� �ִ����� ���õ� ����

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
        // tiem_t : 1970�� 1�� 1�� 00:00:00�κ��� �ð��� ��(second)������ ī��Ʈ
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

        // �켱 ���� 
        int precedence(char c);
        // ���� ǥ��� => ���� ǥ���
        void parse_expression();

    public:
        ExprCell(string data, int x, int y, Table* t);
        ~ExprCell();

        string stringify();
        int to_numeric();
    };

    // Table Ŭ������ Cell ��ü���� 2���� �迭�� ����
    class Table {
    protected:
        // �� �� ���� �ִ� ũ��
        int max_row_size, max_col_size;
        // �����͸� �����ϴ� ���̺�
        Cell*** data_table;

    public:
        Table(int max_row_size, int max_col_size);
        ~Table();

        // ���ο� ���� row �� col ���� ���
        void reg_cell(Cell* c, int row, int col);
        // �ش� ���� ������ ��ȯ
        // s: �� �̸�(ex. A2, B3)
        int to_numeric(const string& s);
        // �� �� �� ��ȣ�� �� ȣ��
        int to_numeric(int row, int col);
        // �ش� ���� ���ڿ� ��ȯ
        string stringify(const string& s);
        string stringify(int row, int col);

        // ���� ���� �Լ� => ���ǰ� �̷����� �ʰ� �Լ��� ����
        // �̷��� ����� ���� ���� �Լ��� �ִٸ� �̸� �߻� Ŭ������ �θ�
        virtual string print_table() = 0;
    };

    // virtual string print_table() = 0; 
    // ���������Լ��� ������ Table Ŭ������ �߻�Ŭ������ Table Ŭ������ ��ü ������ �� ����
    // Txt ���̺�
    class TxtTable : public Table {
        string repeat_char(int n, char c);
        // ���ڷ� �� �� ��ȣ�� A, B, .... Z, AA, AB, ...  �̷� ������ �Ű��ش�.
        string col_num_to_str(int n);
    public:
        TxtTable(int row, int col);

        // �ؽ�Ʈ�� ǥ�� �����ϰ� ���
        string print_table();
    };

    // HTML ���̺�
    class HtmlTable : public Table {
    public:
        HtmlTable(int row, int col);

        string print_table();
    };

    // CSV ���̺�
    class CSVTable : public Table {
    public:
        CSVTable(int row, int col);

        string print_table();
    };

    //
}

#endif
