#include <cctype>
#include "lexer.h"

using std::string;
using std::cout;
using namespace Lexer;

Cell_stream Lexer::cs {std::cin};

Cell Cell_stream::get() {
    // get 1 char, decide what kind of cell is incoming,
    // appropriately get more char then return Cell
    char c = 0;

    do {  // skip all whitespace including newline
        if(!ip->get(c)) return ct = {Kind::end};  // no char can be get from ip
    } while (isspace(c));

    switch (c) {
        case '*':
        case '/':
        case '+':
        case '-':
        case '(':
        case ')':
        case '<':
        case '>':
        case '=':
        case '\'':
            return ct = {static_cast<Kind>(c)}; // primitive operators
        case '0':
        case '1':
        case '2':
        case '3':
        case '4':
        case '5':
        case '6':
        case '7':
        case '8':
        case '9':
        case '.': {  // possibly make . list shorthand
            ip->putback(c);
            double temp;
            *ip >> temp;
            return ct = {temp};
        }
        case 'c':
        case 'd':
        case 'e':
        case 'l': { // primitives coincidentally only start with c, d, e, or l...
            ip->putback(c);
            string temp;
            *ip >> temp;
            while (temp.back() == ')') {    // greedy reading of string adjacent to )
                temp.pop_back();
                ip->putback(')');
            }
            if (temp == "define") ct.kind = Kind::define;
            else if (temp == "lambda") ct.kind = Kind::lambda;
            else if (temp == "cond") ct.kind = Kind::cond;
            else if (temp == "cat") ct.kind = Kind::cat;
            else if (temp == "cons") ct.kind = Kind::cons;
            else if (temp == "car") ct.kind = Kind::car;
            else if (temp == "cdr") ct.kind = Kind::cdr;
            else if (temp == "list") ct.kind = Kind::list;
            else if (temp == "else") ct.kind = Kind::els;
            else { ct.kind = Kind::name; ct.data = temp; }
            return ct;
        }
        default: {    // name
            ip->putback(c);
            string temp;
            *ip >> temp;
            while (temp.back() == ')') {    // greedy reading of string adjacent to )
                temp.pop_back();
                ip->putback(')');
            }
            ct.data = temp;
            ct.kind = Kind::name;
            return ct;
        }
    }
}

void Lexer::print(const Cell& cell) {
    if(cell.kind != Kind::number && cell.kind != Kind::name && cell.kind != Kind::expr) cout << static_cast<char>(cell.kind);    // primitive
    boost::apply_visitor(print_visitor(), cell.data);
}

std::ostream& Lexer::operator<<(ostream& os, const Cell& c) {
    print(c);
    return os;
}

bool Lexer::operator<(const Cell& a, const Cell& b) {
    if (a.kind == Kind::number)
        return boost::apply_visitor(less_visitor(boost::get<double>(a.data)), b.data);
    return boost::apply_visitor(less_visitor(boost::get<string>(a.data)), b.data);
}
bool Lexer::operator==(const Cell& a, const Cell& b) {
     if (a.kind == Kind::number)
        return boost::apply_visitor(equal_visitor(boost::get<double>(a.data)), b.data);
    return boost::apply_visitor(equal_visitor(boost::get<string>(a.data)), b.data);
}
