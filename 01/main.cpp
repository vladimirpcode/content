#include <iostream>
#include <string>
#include <map>

using namespace std;

constexpr char EOT = 3;

enum class Lex{
    NUMBER,     // Число
    IDENT,      // Идентификатор
    PLUS,       // +
    MINUS,      // -
    EQUAL,      // =
    MULTIPLY,   // *
    DIVIDE,     // /
    LBRACE,     // (
    RBRACE,     // )
    EOT,        // Конец текста
    NONE        // Отсутствие лексемы
};

void error(string msg){
    cout << "Ошибка: " << msg << endl;
    exit(-1);
}

void expected(string msg){
    cout << "Ожидалось " << msg << endl;
    exit(-2);
}

bool is_whitespace(char c){
    return c == 9 || c == 32;   // 9 - TAB, 32 - space
}

bool is_number(char c){
    return c >= '0' && c <= '9';
}

bool is_alpha(char c){
    return (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z');
}

class Node{
public:
    Node* left;
    Node* right;
    Lex operation;
    double value;

    Node(){
        left = nullptr;
        right = nullptr;
        operation = Lex::NONE;
        value = 0;
    }

    ~Node(){
        if (left != nullptr){
            delete left;
        }
        if (right != nullptr){
            delete right;
        }
    }
};

class Wrapper{
public:
    Wrapper(string _data){
        data = _data;
        pos = 0;
        next_char();
    }

    void next_char(){   // считать следующий символ
        if (pos == data.length()){
            ch = EOT;
            return;
        }
        ch = data[pos];
        pos++;
        while (is_whitespace(ch) && pos < data.length()){
            ch = data[pos];
            pos++;
        }
        if (is_whitespace(ch)){
            ch = EOT;
        }
    }

    char ch;            // текущий символ
    
private:
    string data;
    int pos;            // номер следующего символа
};

class Lexer{
public:
    Lexer(string s):wrap(s){
        next_lex();
    }
    Lex token;
    int number_value;   // на случай если лексема - число
    string name;        // на случай если лексема - идентификатор
    void next_lex(){
        if (is_alpha(wrap.ch)){
            name = ident();
            token = Lex::IDENT;
        } else if(is_number(wrap.ch)){
            number_value = number();
            token = Lex::NUMBER;
        } else {
            switch (wrap.ch){
                case '+': token = Lex::PLUS; break;
                case '-': token = Lex::MINUS; break;
                case '*': token = Lex::MULTIPLY; break;
                case '/': token = Lex::DIVIDE; break;
                case '(': token = Lex::LBRACE; break;
                case ')': token = Lex::RBRACE; break;
                case '=': token = Lex::EQUAL; break;
                case EOT: token = Lex::EOT; break;
                default: error("неизвестный токен");
            }
            wrap.next_char();
        }
    }
private:
    Wrapper wrap;

    string ident(){
        string result = "";
        while (is_alpha(wrap.ch)){
            result = result + wrap.ch;
            wrap.next_char();
        }
        return result;
    }
    
    int number(){
        int result = 0;
        while (is_number(wrap.ch)){
            // 48 - код нуля (0) по таблице ASCII
            // преобразуем символ в цифру 
            result = result * 10 + (wrap.ch - 48); 
            wrap.next_char();
        }
        return result;
    }
};

Lexer* lexer;
map<string, double> variables;

void term(Node *&node);
void factor(Node *&node);

// Выражение = Слагаемое {ОперСлож Слагаемое}
void expression(Node *&node){
    term(node);     //слагаемое
    while(lexer->token == Lex::MINUS || lexer->token == Lex::PLUS){
        Node *tmp = node;               // запоминаем старое поддерево
        node = new Node();              // создаем новую ноду
        node->operation = lexer->token; // не забываем про операцию
        node->left = tmp;               // подцепляем старое поддерево слева
        lexer->next_lex();
        node->right = new Node();       // по соглашению создаем объект в памяти заранее
        term(node->right);              // правую часть дерева отдаем на откуп term()
    }
}

//Слагаемое = Множитель {ОперУмнож Множитель}
void term(Node *&node){
    factor(node);
    while (lexer->token == Lex::MULTIPLY || lexer->token == Lex::DIVIDE){
        Node *tmp = node;
        node = new Node();
        node->left = tmp;
        node->operation = lexer->token;
        lexer->next_lex();
        node->right = new Node();
        factor(node->right);
    }
}

//Множитель = Число | Идентификатор | "(" Выражение ")"
void factor(Node *&node){
    if (lexer->token == Lex::NUMBER){
        node->value = lexer->number_value;      // не забываем числовое значение
        lexer->next_lex();
    }
    else if (lexer->token == Lex::IDENT){
        // C++ способ найти значение в map. 
        // В других языках по другому.
        if (variables.find(lexer->name) == variables.end()){
            error("имя  не существует");
        }
        node->value = variables[lexer->name];   // не забываем значение переменной
        lexer->next_lex();
    }
    else if (lexer->token == Lex::LBRACE){
        lexer->next_lex();
        expression(node);
        if (lexer->token != Lex::RBRACE){
            expected(")");
        }
        lexer->next_lex();
    } 
    else {
        expected("Имя, Число или (Выражение)");
    }
}

double calculate(Node* node){
    if (node->left == nullptr && node->right == nullptr){
        return node->value;
    }
    double left_result = calculate(node->left);
    double right_result = calculate(node->right);
    switch (node->operation){
        case Lex::PLUS: return left_result + right_result;
        case Lex::MINUS: return left_result - right_result;
        case Lex::DIVIDE: return left_result / right_result;
        case Lex::MULTIPLY: return left_result * right_result;
        default: error("неизвестная операция. Вычисление не удалось");
    }
    return 0;
}

int main(){
    string expr;
    cout << "Введите выражение: ";
    getline(cin, expr);

    variables["myVar"] = 100;
    variables["pi"] = 3.14;
    lexer = new Lexer(expr);
    Node* tree_root = new Node();   // создаем корень дерева
    expression(tree_root);
    if (lexer->token != Lex::EOT){
        expected("EOT");
    }
    double result = calculate(tree_root);   // вычисляем выражение
    cout << " = " << result << "\n";
    cout << "__________OK\n";
    delete lexer;
    
    return 0;
}