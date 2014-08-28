#ifndef bc_lexer
#define bc_lexer
#include <string>
#include <iostream>
#include <map>
#include <memory>   // shared_ptr
#include "boost/variant.hpp"
#include "forward.h"

namespace Lexer {
    using namespace std;

    enum class Kind : char {
        Include,
        Cat, Cons, Car, Cdr, List,  // primitive procs
        Define = 'd', Lambda = 'l', Number = '#', Name = 'n', Expr = 'e', Proc = 'p', False = 'f', True = 't', Cond = 'c', Else = ',', End = '.', Empty = ' ',   // special cases
        Quote = '\'', Lp = '(', Rp = ')', And = '&', Not = '!', Or = '|',
        Mul = '*', Add = '+', Sub = '-', Div = '/', Less = '<', Equal = '=', Greater = '>',  // primitive operators
        Comment = ';'
    };

    struct Proc {
        List params;    
        List body;
        Environment::Env* env;
    };

    using Data = boost::variant<string, double, Proc*, List>;  // could make List into List*, but then introduce more management issues and indirection

    struct Cell {
        Kind kind;
        Data data;

        // constructors
        Cell() : kind{Kind::End} {} // need default for vector storage
        Cell(Kind k) : kind{k} {}
        Cell(const double n) : kind{Kind::Number}, data{n} {}
        Cell(const string& s) : kind{Kind::Name}, data{s} {}
        Cell(const char* s) : kind{Kind::Name}, data{s} {}
        Cell(Proc* p) : kind{Kind::Proc}, data{p} {}
        Cell(List l) : kind{Kind::Expr}, data{l} {}
        explicit Cell(bool b) : kind{b? Kind::True : Kind::False} {}

        // copy and move constructors
        Cell(const Cell&) = default;
        Cell& operator=(const Cell&) = default;
        Cell(Cell&&) = default;
        Cell& operator=(Cell&&) = default;

        ~Cell() = default;

        // conversion operators
        operator bool() { return kind != Kind::False; }
    };

    class Cell_stream {
    public:
        Cell_stream(istream& instream_ref) : ip{&instream_ref} {}
        Cell_stream(istream* instream_pt)  : ip{instream_pt}, owns{instream_pt} {}

        Cell get();    // get and return next cell
        const Cell& current() { return ct; } // most recently get cell
        bool eof() { return ip->eof(); }
        bool base() { return old.size() == 0; }
        void reset() { if (owns.back() == ip) { delete owns.back(); owns.pop_back(); } ip = old.back(); old.pop_back(); }
        void ignoreln() { ip->ignore(9001, '\n'); }

        void set_input(istream& instream_ref) { old.push_back(ip); ip = &instream_ref; }
        void set_input(istream* instream_pt) { old.push_back(ip); ip = instream_pt; owns.push_back(ip); }

    private:
        // void close() { if (owns) delete ip; }
        istream* ip;    // input stream pointer
        vector<istream*> old;  // for switching between input streams through include
        vector<istream*> owns;
        Cell ct {Kind::End};   // current token, default value in case of misuse
    };

    // overloaded operators 
    ostream& operator<<(ostream&, const Cell&);
    bool operator<(const Cell&, const Cell&);
    bool operator==(const Cell&, const Cell&);
    void print(const Cell&);

    extern Cell_stream cs;
    extern map<string, Kind> keywords;



    // visitors
    class print_visitor : public boost::static_visitor<> {
        string end {" "};
    public:
        print_visitor() {}
        print_visitor(string e) : end{e} {}
        void operator()(const string& str) const {
            cout << str << end;
        }
        void operator()(const double num) const {
            cout << num << end;
        }
        void operator()(const Lexer::Proc* proc) const {
            cout << "proc" << end;
        }
        void operator()(const Lexer::List list) const {
            cout << '(';
            if (list.size() > 0) {
                auto p = list.begin();
                if(p->kind != Kind::Number && p->kind != Kind::Name && p->kind != Kind::Expr) cout << static_cast<char>(p->kind);    // primitive
                for (;p + 1 != list.end(); ++p) 
                    boost::apply_visitor(print_visitor(), p->data);
                boost::apply_visitor(print_visitor(""), p->data);
            }
            cout << ')' << end;
        }
    };

    class less_visitor : public boost::static_visitor<bool> {
        // first elements stored, second elements taken as operand
        string str;
        double num;
        List list;
        Proc* proc;
    public:
        less_visitor(const string& s) : str{s} {}
        less_visitor(const double d) : num{d} {}
        less_visitor(Proc* const p) : proc(p) {}
        less_visitor(const List& l) : list{l} {}
        bool operator()(const string& s) const { return str < s; }
        bool operator()(const double n) const { return num < n; }
        bool operator()(Proc* const p) const { return (*proc).body < (*p).body; }
        bool operator()(const List& l) const { return list < l; }
    };

    class equal_visitor : public boost::static_visitor<bool> {
        string str;
        double num;
        List list;
        Proc* proc;
    public:
        equal_visitor(const string& s) : str{s} {}
        equal_visitor(const double d) : num{d} {}
        equal_visitor(Proc* const p) : proc(p) {}
        equal_visitor(const List& l) : list{l} {}
        bool operator()(const string& s) const { return str == s; }
        bool operator()(const double n) const { return num == n; }
        bool operator()(Proc* const p) const { return proc == p; }
        bool operator()(const List& l) const { return list == l; }
    };
}
#endif
