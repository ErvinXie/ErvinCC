#include <iostream>
#include <cstdio>
#include <string>
#include <vector>
#include <set>
#include <map>
#include <fstream>
#include "util.h"

using namespace std;

vector<string> punctuators = {"[", "]", "(", ")", "{", "}", ".", "->",
                              "++", "--", "&", "*", "+", "-", "~", "!",
                              "/", "%", "<<", ">>", "<", ">", "<=", ">=",
                              "==", "!=", "^", "|", "&&", "||",
                              "?", ":", ";", "...",
                              "=", "*=", "/=", "%=", "+=", "-=", "<<=", ">>=",
                              "&=", "^=", "|=", ",", "#", "##",
                              "<:", ":>", "<%", "%>", "%:", "%:%:", "\\"};


vector<string> keywords = {"auto", "break", "case", "char", "const", "continue", "default", "do", "double", "else",
                           "enum", "extern", "float", "for", "goto", "if", "inline", "int", "long", "register",
                           "restrict", "return", "short", "signed", "sizeof", "static", "struct", "switch", "typedef",
                           "union", "unsigned", "void", "volatile", "while", "_Alignas", "_Alignof", "_Atomic", "_Bool",
                           "_Complex", "_Generic", "_Imaginary", "_Noreturn", "_Static_assert"};

set<char> digits, nondigits, letters, pchars, symbols, oct_digits, hex_digits, blank;


bool isDigit(char c) { return digits.count(c); }

bool isNonDigit(char c) { return nondigits.count(c); }

bool isPuc(char c) { return pchars.count(c); }


enum NodeType {
    FAIL,
    HEAD,
    KEY_IN,
    KEY_LAST,
    ID_IN,
    PUC_IN,
    PUC_LAST,
    NUMERIC_IN,
    CH_IN,
    ST_IN,
    AC,
    KEYWORDS,
    IDENTIFIER,
    PUNCTUATORS,
    INTEGER_CONSTANTS,
    FLOATING_CONSTANTS,
    CHARACTER_CONSTANTS,
    STRING_LITERALS,
    EOF_AC,

};
vector<string> NodeTypeString = {"FAIL", "HEAD", "KEY_IN", "KEY_LAST", "ID_IN", "PUC_IN", "PUC_LAST", "NUMERIC_IN",
                                 "CH_IN", "ST_IN",
                                 "AC", "KEYWORDS", "IDENTIFIER", "PUNCTUATORS", "INTEGER_CONSTANTS",
                                 "FLOATING_CONSTANTS", "CHARACTER_CONSTANTS", "STRING_LITERALS", "EOF_AC"};

struct node;
typedef node *nodep;
nodep fail;
int scnt = 0;

struct node {
    int cnt = 0;
    NodeType type;
    map<char, nodep> nex;
    nodep default_next;

    node(NodeType nt) : type(nt) {
        scnt++;
        cnt = scnt;
        default_next = fail;
//        cout<<cnt<<" "<<NodeTypeString[nt]<<endl;
    }

    nodep next(char x) {
        if (nex.count(x))
            return nex[x];
        else
            return default_next;
    }

    void set(set<char> those, nodep to) {
        for (auto c:those) {
            this->nex[c] = to;
        }
    }

    void set(string those, nodep to) {
        for (auto c:those) {
            this->nex[c] = to;
        }
    }
};

nodep head;

nodep keyword_ac = new node(KEYWORDS);
nodep identifier_ac = new node(IDENTIFIER);
nodep identifier_interim = new node(ID_IN);

vector<nodep> all_keyword;

void insert_keyword(const string &k) {
    nodep t = head;
    for (auto c:k) {
        if (t->nex.count(c) == 0) {
            t->nex[c] = new node(KEY_IN);
        }
        t = t->nex[c];
        all_keyword.push_back(t);
    }
    t->type = KEY_LAST;
}

void build_keywords_identifier() {
    for (const auto &s:keywords) {
        insert_keyword(s);
    }
    for (auto n:all_keyword) {
        for (auto c:letters) {
            if (n->nex.count(c) == 0)
                n->nex[c] = identifier_interim;
        }
        if (n->type == KEY_IN) {
            n->default_next = identifier_ac;
        }
        if (n->type == KEY_LAST) {
            n->default_next = keyword_ac;
        }
    }

    identifier_interim->set(letters, identifier_interim);
    identifier_interim->default_next = identifier_ac;
    for (auto c:nondigits) {
        if (head->nex.count(c) == 0) {
            head->nex[c] = identifier_interim;
        }
    }
}

nodep punctuator_ac = new node(PUNCTUATORS);
vector<nodep> all_punctuator;

void insert_punctuator(const string &p) {
    nodep t = head;
    for (auto c:p) {
        if (t->nex.count(c) == 0) {
            t->nex[c] = new node(PUC_IN);
        }
        t = t->nex[c];
        all_punctuator.push_back(t);
    }
    t->type = PUC_LAST;
}

void build_punctuators() {
    for (const auto &s : punctuators) {
        insert_punctuator(s);
    }
    for (auto n : all_punctuator) {
        if (n->type == PUC_IN)
            n->default_next = fail;
        if (n->type == PUC_LAST)
            n->default_next = punctuator_ac;
    }
}

nodep decsq = new node(NUMERIC_IN);
nodep octsq = new node(NUMERIC_IN);
nodep hexsq = new node(NUMERIC_IN);
nodep zero = new node(NUMERIC_IN);

void build_integer() {
    vector<nodep> ull;
    for (int i = 0; i < 10; i++) {
        ull.push_back(new node(NUMERIC_IN));
    }

    nodep int_ac = new node(INTEGER_CONSTANTS);
    head->set(digits, decsq);
    head->set("0", zero);

    decsq->set(digits, decsq);

    zero->set("xX", hexsq);
    zero->set(oct_digits, octsq);

    octsq->set(oct_digits, octsq);
    hexsq->set(hex_digits, hexsq);


    for (auto sq:vector<nodep>({decsq, octsq, hexsq, zero})) {
        sq->set("uU", ull[0]);
        sq->set("l", ull[6]);
        sq->set("L", ull[5]);
    }
    ull[0]->set("l", ull[1]);
    ull[0]->set("L", ull[3]);
    ull[1]->set("l", ull[2]);
    ull[3]->set("L", ull[4]);
    ull[5]->set("L", ull[7]);
    ull[5]->set("uU", ull[9]);
    ull[6]->set("l", ull[8]);
    ull[6]->set("uU", ull[9]);
    ull[7]->set("uU", ull[9]);
    ull[8]->set("uU", ull[9]);

    vector<nodep> former_ac{decsq, octsq, hexsq, zero};
    for (auto x:ull) {
        former_ac.push_back(x);
    }
    for (auto l:former_ac) {
        for (auto c:nondigits) {
            if (l->nex.count(c) == 0)
                l->nex[c] = fail;
        }
        l->default_next = int_ac;
    }
}

void build_floating() {
    nodep decfr = new node(NUMERIC_IN);
    nodep hexfr = new node(NUMERIC_IN);
    nodep exp = new node(NUMERIC_IN);
    nodep sign = new node(NUMERIC_IN);
    nodep expd = new node(NUMERIC_IN);
    nodep flFL = new node(NUMERIC_IN);
    nodep float_ac = new node(FLOATING_CONSTANTS);
    if (head->nex.count('.') == 0)
        head->nex['.'] = new node(NUMERIC_IN);
    nodep dot = head->nex['.'];

    dot->set(digits, decfr);
    for (auto x:vector<nodep>({decsq, zero, octsq})) {
        x->set(".", decfr);
    }
    hexsq->set(".", hexfr);

    decfr->set(digits, decfr);
    hexfr->set(hex_digits, hexfr);

    for (auto x:vector<nodep>({decsq, zero, octsq, decfr})) {
        x->set("eE", exp);
    }
    for (auto x:vector<nodep>({hexsq, hexfr})) {
        x->set("pP", exp);
    }


    exp->set("+-", sign);
    for (auto x:vector<nodep>({exp, sign})) {
        x->set(digits, expd);
    }
    expd->set(digits, expd);
    for (auto x:vector<nodep>({decfr, expd})) {
        x->set("flFL", flFL);
    }

    for (auto x:vector<nodep>({decfr, expd, flFL})) {
        for (auto c:nondigits) {
            if (x->nex.count(c) == 0)
                x->nex[c] = fail;
        }
        if (x->nex.count('.') == 0)
            x->nex['.'] = fail;
        x->default_next = float_ac;
    }
}

void build_char() {
    nodep chsq = new node(CH_IN);
    nodep esca = new node(CH_IN);
    nodep chls = new node(CH_IN);
    nodep ch_ac = new node(CHARACTER_CONSTANTS);

    for (auto c:string("LuU")) {
        if (head->nex.count(c) == 0) {
            head->nex[c] = new node(CH_IN);
        }
        head->nex[c]->set("'", chsq);
    }
    head->set("'", chsq);
    chsq->default_next = chsq;
    chsq->set("\\", esca);
    chsq->set("'", chls);
    chsq->set("\n", fail);

    esca->default_next = chsq;
    esca->set("\n", fail);

    chls->default_next = ch_ac;
}

void build_string() {
    nodep stsq = new node(ST_IN);
    nodep esca = new node(ST_IN);
    nodep stls = new node(ST_IN);
    nodep st_ac = new node(STRING_LITERALS);

    vector<nodep> former_chsq = {head};
    for (auto c:string("LuU")) {
        if (head->nex.count(c) == 0) {
            head->nex[c] = new node(ST_IN);
        }
        former_chsq.push_back(head->nex[c]);
    }
    head->nex['u']->nex['8'] = new node(ST_IN);
    former_chsq.push_back(head->nex['u']->nex['8']);

    for (auto x:former_chsq) {
        x->set("\"", stsq);
    }

    stsq->default_next = stsq;
    stsq->set("\\", esca);
    stsq->set("\"", stls);
    stsq->set("\n", fail);

    esca->default_next = stsq;
    esca->set("\n", fail);

    stls->default_next = st_ac;
}

void init() {
    for (auto c:string("0123456789")) {
        digits.insert(c);
        letters.insert(c);
        symbols.insert(c);
    }
    for (auto c:string("_abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ")) {
        nondigits.insert(c);
        letters.insert(c);
        symbols.insert(c);
    }

    for (auto s:punctuators) {
        for (auto c:s) {
            pchars.insert(c);
            symbols.insert(c);
        }
    }

    for (auto c:string("01234567")) {
        oct_digits.insert(c);
    }
    for (auto c:string("0123456789abcdefABCDEF")) {
        hex_digits.insert(c);
    }
    for (auto c:string(" \n\t\r")) {
        blank.insert(c);
    }

    fail = new node(FAIL);
    head = new node(HEAD);
    build_keywords_identifier();
    build_punctuators();
    build_integer();
    build_floating();
    build_char();
    build_string();
    head->nex[EOF] = new node(EOF_AC);
}

int linenum = 1, colnum = 0, wordnum = 0;
//string result = "";

// DEBUG

string token_type(NodeType x, string result) {
    switch (x) {
        case KEYWORDS:
            return "" + result + "";
        case IDENTIFIER: {
            for (const auto &k:keywords) {
                if (result == k) {
                    return result;
                }
            }
            return "identifier";
        }
        case PUNCTUATORS:
            return "" + result + "";
        case INTEGER_CONSTANTS:
            return "integer-constant";
        case FLOATING_CONSTANTS:
            return "floating-constant";
        case CHARACTER_CONSTANTS:
            return "character-constant";
        case STRING_LITERALS:
            return "string-literal";
        case EOF_AC:
            return "EOF";
    }
}

// VISUALIZE
map<char, string> pp;
set<int> vis;

string diff(string t, set<char> s, string setname) {
    set<char> in, out;
    string plus = "";
    for (int i = 0; i < t.length(); i += 2) {
        char x = t[i];
        if (s.count(x)) {
            in.insert(x);
        } else {
            plus += pp[x] + ",";
        }
    }
    string sub = "";
    for (auto x:s) {
        if (in.count(x) == 0)
            sub += pp[x] + ",";
    }
    if (plus.length())
        setname += "+" + plus;
    if (sub.length())
        setname += "-" + sub;
    return setname;
}

string choose_diff(string t) {
    string re = t;

    string x;
    x = diff(t, digits, "digits");
    if (x.length() <= re.length())
        re = x;

    x = diff(t, letters, "letters");
    if (x.length() <= re.length())
        re = x;

    x = diff(t, nondigits, "nondigits");
    if (x.length() <= re.length())
        re = x;

    return re;
}

void dfs(nodep now) {
    vis.insert(now->cnt);
    printf("    \"#%d\\n%s\" [shape=%s]\n",
           now->cnt,
           NodeTypeString[now->type].data(),
           now->type > AC ? "doublecircle" : (now->type == FAIL ? "doubleoctagon" : "circle"));

    map<nodep, string> edges;
    for (auto p:now->nex) {
        nodep np = p.second;
        if (edges.count(np) == 0) {
            edges[np] = pp[p.first];
        } else {
            edges[np] += "," + pp[p.first];
        }
    }
    for (auto p:edges) {
        printf("    \"#%d\\n%s\" -> \"#%d\\n%s\" [label = \"%s\"]\n",
               now->cnt,
               NodeTypeString[now->type].data(),
               p.first->cnt,
               NodeTypeString[p.first->type].data(),
               choose_diff(p.second).data()
//               p.second.data()
        );
    }
    if (now->default_next != nullptr) {
        printf("    \"#%d\\n%s\" -> \"#%d\\n%s\" [label = \"%s\"]\n",
               now->cnt,
               NodeTypeString[now->type].data(),
               now->default_next->cnt,
               NodeTypeString[now->default_next->type].data(),
               "others");

    }

    for (auto p:now->nex) {
        if (vis.count(p.second->cnt) == 0)
            dfs(p.second);
    }
    if (now->default_next != nullptr && vis.count(now->default_next->cnt) == 0) {
        dfs(now->default_next);
    }

}

void to_dot() {
    for (auto c:symbols) {
        pp[c] = c;
    }

    pp['\n'] = "NEW LINE";
    pp[EOF] = "EOF";
    pp[' '] = "SPACE";
    pp['\r'] = "";
    pp['\t'] = "TAB";
    pp['\\'] = "SLASH";

    cout << "digraph finite_state_machine {\n"
            "    graph [ dpi = 200 ];\n"
            "    rankdir = LR;\n";

    dfs(head);

    cout << "}\n";
}


int main(int argc, char *argv[]) {
    string in_dir;
    string out_dir;
    for (int i = 0; i < argc; i++) {
        if (string(argv[i]) == "-o") {
            out_dir = argv[i + 1];
            i++;
        } else {
            in_dir = argv[i];
        }
    }
    if (out_dir.empty()) {
        for (int i = (int) in_dir.length() - 1; i >= 0; i--) {
            if (in_dir[i] == '.') {
                out_dir = in_dir.substr(0, i) + ".tokens";
                break;
            }
        }
    }
    cout << in_dir << endl;
    cout << out_dir << endl;
    ifstream in(in_dir);
    ofstream out(out_dir);
    init();
//    to_dot();
//    return 0;
    nodep now = head;
    char a = 0;
    string result;
    char t[1000];
    do {
        if (a != '\r')
            colnum++;
        a = in.get();
//        printf("line:%d col:%d %c\n", linenum, colnum, a);
        if (now->type == FAIL) {
            if (blank.count(result[0]) == 0) {
                sprintf(t, "line: %d colunm: %d %s Wrong!\n", linenum, colnum, result.data());
                cerr << t << endl;
                return 1;
            }
            now = head;
            result = "";
        }

        now = now->next(a);

        if (now->type > AC) {
            int startcol = colnum - (int) result.length();
            if (a == EOF)
                result = "<EOF>";

            sprintf(t, "[@%d,%d:%d='%s',<%s>,%d:%d]\n", wordnum, startcol, colnum, result.data(),
                    token_type(now->type, result).data(), linenum, startcol);
            out << string(t);
//            cout << result << " " << NodeTypeString[now->type] << endl;
            wordnum++;
            now = head->next(a);
            result = "";

        }
        result.push_back(a);
//        cout << NodeTypeString[now->type] << endl;
        if (a == '\n') {
            linenum++;
            colnum = 0;
        }
    } while (a != EOF);
    cout<<"Scanner OK"<<endl;
    in.close();
    out.close();
    return 0;
}
