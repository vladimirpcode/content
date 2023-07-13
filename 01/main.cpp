#include <iostream>
#include <string>

using namespace std;

constexpr char EOT = 4;

enum class Lex{
    NUMBER=10,
    IDENT,
    PLUS,
    MINUS,
    EQUAL,
    MULTIPLY,
    DIVIDE,
    LBRACE,
    RBRACE,
    EOT
};

class Node{
public:
    Node* left;
    Node* right;
    Lex operation;
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
    return (c >= 65 && c <= 90) && (c >= 97 && c <= 122);
}

class Wrapper{
public:
    char ch;

    Wrapper(string _str){
        str = _str;
        pos = 0;
        get_next();
    }

    char get_next(){
        if (pos == str.length()){
            ch = EOT;
            return EOT;
        }
        ch = str[pos];
        pos++;
        while (is_whitespace(ch) && pos < str.length()){
            ch = str[pos];
            pos++;
        }
        if (is_whitespace(ch)){
            ch = EOT;
            return EOT;
        }
        return ch;
    }

private:
    string str;
    int pos;
};

class Lexer{
public:
    Lex token;
    int number_value;
    string name;
    Lexer(string s):wrap(s){
        next_lex();
    }
    Lex next_lex(){
        if (is_alpha(wrap.ch)){
            name = ident();
            token = Lex::IDENT;
            return token;
        } else if(is_number(wrap.ch)){
            number_value = number();
            token = Lex::NUMBER;
            return token;
        } else {
            switch (wrap.ch){
                case '+': token = Lex::PLUS; break;
                case '-': token = Lex::MINUS; break;
                case '*': token = Lex::MULTIPLY; break;
                case '/': token = Lex::DIVIDE; break;
                case '(': token = Lex::LBRACE; break;
                case ')': token = Lex::RBRACE; break;
                case '=': token = Lex::EQUAL; break;
                case EOT: token = Lex::EOT; return token;
                default: error("неизвестный токен " + wrap.ch);
            }
            wrap.get_next();
            return token;
        }
    }
private:
    Wrapper wrap;
    
    string ident(){
        string result = "";
        while (is_alpha(wrap.ch)){
            result = result + wrap.ch;
            wrap.get_next();
        }
        return result;
    }
    
    int number(){
        int result = 0;
        while (is_number(wrap.ch)){
            result = result * 10 + (wrap.ch - 48); 
            wrap.get_next();
        }
        return result;
    }
};

////////////////////////
// парсер
// Выражение = Слагаемое {ЗнакСлож Слагаемое}
// Слагаемое = Множитель {ЗнакУмнож Множитель}
// Множитель = Число | Идентификатор | "(" Выражение ")"
// ЗнакСлож = "+" | "-"
// ЗнакУмнож = "*" | "/"
void expression(){
    
}

////////////////////////

int main()
{
    

    return 0;
}
