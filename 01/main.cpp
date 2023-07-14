#include <iostream>
#include <string>
#include <map>
#include <vector>

using namespace std;

constexpr char EOT = 4;

enum class Lex{
    NUMBER,
    IDENT,
    PLUS,
    MINUS,
    EQUAL,
    MULTIPLY,
    DIVIDE,
    LBRACE,
    RBRACE,
    EOT,
    NONE
};

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
                default: cout << wrap.ch; error("неизвестный токен");
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


Lexer* lexer;
map<string, double> context;

////////////////////////
void term(Node *&node);
void factor(Node *&node);

// парсер
// Выражение = Слагаемое {ЗнакСлож Слагаемое}
// Слагаемое = Множитель {ЗнакУмнож Множитель}
// Множитель = Число | Идентификатор | "(" Выражение ")"
// ЗнакСлож = "+" | "-"
// ЗнакУмнож = "*" | "/"
void expression(Node *&node){
    term(node);
    while (lexer->token == Lex::MINUS || lexer->token == Lex::PLUS){
        Node *tmp = node;
        node = new Node();
        node->operation = lexer->token;
        node->left = tmp;
        lexer->next_lex();
        node->right = new Node();
        term(node->right);
    }
}

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

void factor(Node *&node){
    if (lexer->token == Lex::LBRACE){
        lexer->next_lex();
        expression(node);
        if (lexer->token != Lex::RBRACE){
            expected(")");
        }
        lexer->next_lex();
    } else if (lexer->token == Lex::NUMBER){
        node->value = lexer->number_value;
        lexer->next_lex();
    } else if (lexer->token == Lex::IDENT){
        if (context.find(lexer->name) == context.end()){
            error("имя  не существует");
        }
        node->value = context[lexer->name];
        lexer->next_lex();
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
        default: cout << "U" << (int)node->operation << "__" << node->value<<"|"; return left_result;
    }
    return 0;
}

////////////////////////
// соглашение: каждая процедура получает инициализированный указатель на ноду
int main()
{
    context["myVar"] = 100;
    context["pi"] = 3.14;
    vector<string> exprs = {
        "2+2+7", 
        "10 * 2 + 7", 
        "((2 + 100 * 7) + 1) / 2",
        "((2 + myVar * 7) + 1) / 2",
        "pi * 2 * 2",
        "(2+2)*(3+3)",
        "2 + 2 + 2",
        "(1 + 3 + (4 * 5 - (7/2 + (2+5-4))))"

    };
    for (auto& expr:exprs){
        cout << expr << " = ";
        lexer = new Lexer(expr);
        Node* tree = new Node();
        expression(tree);
        cout << calculate(tree) << endl;
        delete tree;
        delete lexer;
    }
    return 0;
}
