//
// Created by 谢威宇 on 2020/4/14.
//

#include "cst_node.h"
#include <iostream>
#include <fstream>
#include <set>
#include <regex>

string cst_node::to_string(bool recursive) {
    string re = "{";
    re += R"("type":")" + type + "\",";
    if (!token.empty()) {
        if (type == "integer-constant" || type == "float-constant")
            re += R"("token":)" + token;
        else if (type == "string-literal" || type == "char-literal") {
            string label;
            for (int i = 0; i < token.length(); i++) {
                if (token[i] == '"' || token[i] == '\\')
                    label.push_back('\\');
                label.push_back(token[i]);
            }
            re += R"("token":")" + label + R"(")";
        } else {
            re += R"("token":")" + token + R"(")";
        }
        if (!sons.empty())
            re += ",";
    }

    if (!sons.empty()) {
        if (is_list()) {
            re += string(R"("items":)") + "[";
            for (int i = 0; i < sons.size(); i++) {
                auto p = sons[i];
                if (recursive) {
                    re += p->to_string(recursive);
                }
                if (i != sons.size() - 1)
                    re += ",";
            }
            re += "]";
        } else {
            map<string, int> names;
            for (int i = 0; i < sons.size(); i++) {
                names[sonnames[i]] += 1;
                if (names[sonnames[i]] == 1)
                    re += string("\"" + sonnames[i] + "\":");
                else
                    re += string("\"" + sonnames[i] + ::to_string(names[sonnames[i]]) + "\":");

                auto p = sons[i];
                if (recursive) {
                    re += p->to_string(recursive);
                }
                if (i != sons.size() - 1)
                    re += ",";
            }
        }
    }
    re += "}";
    return re;

}

int cst_node::cst_node_cnt = 0;

cst_node::cst_node(string type, string token) : type(type), token(token) { cnt = ++cst_node_cnt; }


void cst_node::eliminate() {
    for (int i = 0; i < sons.size(); i++) {
        auto x = sons[i];
        sonnames.push_back(x->type);
        while (!x->is_vt() && x->sons.size() == 1) {
            x = x->sons[0];
        }
        sons[i] = x;
        sons[i]->eliminate();
    }
}

bool cst_node::is_list() {
    return (type.length() >= 4 && type.substr(type.size() - 4, 4) == "list")
           || type == "translation-unit"
           || type == "declaration-specifiers"
           || type == "pointer";
}

void cst_node::expand_list() {
//    cout << type << endl;
    if (is_list()) {
        vector<cnp> list;
        vector<cnp> st;
        st.push_back(this);
        while (!st.empty()) {
            auto u = st.back();
            st.pop_back();
            if (u->type == this->type) {
                for (int i = (int) u->sons.size() - 1; i >= 0; i--) {
                    st.push_back(u->sons[i]);
                }
            } else {
                list.push_back(u);
            }
        }

        sons.clear();
        for (auto i:list) {
            sons.push_back(i);
        }
    }
    for (auto son:sons) {
        son->expand_list();
    }
}


bool cst_node::is_vt() {
    return sons.empty();
}

void cst_node::to_ast() {
    expand_list();
    eliminate();
}

string cst_node::to_dot(string outdir) {
    ofstream dot(outdir);

    string re = "graph tree\n"
                "{\n"
                "    ordering=out ;\n"
                "    graph[dpi = 400];\n"
                "    rankdir = UD;\n"
                "\n"
                "    node [shape =rect]\n";
    vector<cnp> st;
    st.push_back(this);
    while (st.size()) {
        auto u = st.back();
        st.pop_back();
        string label = u->token.empty() ? u->type : u->token;
        if (u->type == "string-literal") {
            label = "";
            set<char> s({'"', '\\'});
            for (auto c:u->token) {
                if (s.count(c))
                    label.push_back('\\');
                label.push_back(c);
            }
        }
        re += "    " + ::to_string(u->cnt) + "[label = \"" + label + "\"]\n";
        for (int i = 0; i < u->sons.size(); i++) {

            re += "    " + ::to_string(u->cnt) + " -- " + ::to_string(u->sons[i]->cnt) + "\n";
            st.push_back(u->sons[(int) u->sons.size() - i - 1]);
        }
    }
    re += "}";
    dot << re;
    return re;
}

vector<cnp> cst_node::all_vt() {

    if (is_vt()) {
        return vector({this});
    } else {
        vector<cnp> re;
        for (auto s:sons) {
            for (auto vt:s->all_vt()) {
                re.push_back(vt);
            }
        }
        return re;
    }
}

string cst_node::content() {
    if (is_vt())
        return token;
    else {
        string re;
        for (auto s:sons) {
            re += s->content() + " ";
        }
        return re;
    }
}

cnp from_string(string str, bool file = false) {
    cnp re;
    string x;
    bool instr = false;
    bool esc = false;
    if (file == true) {
        ifstream in(str);
        char c;
        in >> noskipws;
        while (in >> c) {
            if (esc) {
                esc = false;
                x.push_back(c);
                continue;
            }
            if (c == '\\') {
                esc = true;
                continue;
            }
            if (c == '"')
                instr = !instr;
            if (!instr && (c == ' ' || c == '\n'))
                continue;
            x.push_back(c);
        }
    } else {
        x = str;
    }
//    cout << x << endl;
    int level = 0;
    int last_comma = 0;
    int last_conn = 0;
    vector<pair<string, string>> kv;
    for (int i = 0; i < x.length(); i++) {
        if (esc) {
            esc = false;
            continue;
        }
        if (x[i] == '\\') {
            esc = true;
        }
        if (x[i] == '"')
            instr = !instr;
        if (!instr) {
            if (x[i] == '{' || x[i] == '[')
                level++;
            if (x[i] == '}' || x[i] == ']')
                level--;
            if (level <= 1) {
                if (x[i] == ':') {
                    last_conn = i;
                }
                if (x[i] == ',' || level == 0) {
                    kv.push_back({x.substr(last_comma + 2, last_conn - 2 - (last_comma + 2) + 1),
                                  x.substr(last_conn + 1, i - 1 - (last_conn + 1) + 1)});
                    last_comma = i;
                }
            }
        }
    }
    re = new cst_node(kv.begin()->second.substr(1, kv.begin()->second.length() - 2));
    for (auto p:kv) {
//        cout << p.first << " " << p.second << endl;
        if (p.first == "items") {
            auto &s = p.second;
            last_comma = 0;
            for (int i = 0; i < s.length(); i++) {
                if (esc) {
                    esc = false;
                    continue;
                }
                if (s[i] == '\\') {
                    esc = true;
                }
                if (s[i] == '"')
                    instr = !instr;
                if (instr == false) {
                    if (s[i] == '{' || s[i] == '[')
                        level++;
                    if (s[i] == '}' || s[i] == ']')
                        level--;
                    if (level == 0 || (level == 1 && s[i] == ',')) {
                        auto sub = s.substr(last_comma + 1, i - 1 - (last_comma + 1) + 1);
//                        cout << sub << endl;
                        re->sons.push_back(from_string(sub));
                        last_comma = i;
                    }
                }
            }
        } else if (p.first == "token") {
            if (p.second[0] != '"') {
                re->token = p.second;
            } else {
                re->token = p.second.substr(1, p.second.length() - 2);
            }
        } else {
            if (p.first != "type") {
                re->sonnames.push_back(p.first);
                re->sons.push_back(from_string(p.second));
            }
        }
    }
    return re;
}




