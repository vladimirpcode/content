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
    return c >= 48 && c <= 57;
}

bool is_alpha(char c){
    return (c >= 65 && c <= 90) || (c >= 97 && c <= 122);
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
            return;
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

void term();
void factor();

// Выражение = Слагаемое {ОперСлож Слагаемое}
void expression(){
    term();     //слагаемое
    while(lexer->token == Lex::MINUS || lexer->token == Lex::PLUS){
        lexer->next_lex();
        term();
    }
}

//Слагаемое = Множитель {ОперУмнож Множитель}
void term(){
    factor();
    while (lexer->token == Lex::MULTIPLY || lexer->token == Lex::DIVIDE){
        lexer->next_lex();
        factor();
    }
}

//Множитель = Число | Идентификатор | "(" Выражение ")"
void factor(){
    if (lexer->token == Lex::NUMBER){
        lexer->next_lex();
    }
    else if (lexer->token == Lex::IDENT){
        // C++ способ найти значение в map. 
        // В других языках по другому.
        if (variables.find(lexer->name) == variables.end()){
            error("имя  не существует");
        }
        lexer->next_lex();
    }
    else if (lexer->token == Lex::LBRACE){
        lexer->next_lex();
        expression();
        if (lexer->token != Lex::RBRACE){
            expected(")");
        }
        lexer->next_lex();
    } 
}

int main(){
    string expr;
    cout << "Введите выражение: ";
    getline(cin, expr);

    variables["myVar"] = 100;
    variables["pi"] = 3.14;
    lexer = new Lexer(expr);
    expression();
    if (lexer->token != Lex::EOT){
        expected("EOT");
    }
    cout << "__________OK\n";
    delete lexer;
    
    return 0;
}