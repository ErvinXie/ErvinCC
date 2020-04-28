//
// Created by 谢威宇 on 2020/4/14.
//

#include "cst_node.h"
#include <iostream>
#include <fstream>
#include <set>

string cst_node::to_string(bool recursive) {
    string re = "{";
    re += R"("type" : ")" + type + "\",";
    if (!token.empty()) {
        if (type == "integer-constant" || type == "float-constant")
            re += R"("token":)" + token;
        else if (type == "string-literal" || type == "char-literal") {
            re += R"("token":"\)" + token.substr(0, token.length() - 1) + R"(\"")";
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
            map<string,int> names;
            for (int i = 0; i < sons.size(); i++) {
                names[sonnames[i]]+=1;
                if(names[sonnames[i]]==1)
                    re += string("\"" + sonnames[i] + "\":");
                else
                    re += string("\"" + sonnames[i]+::to_string(names[sonnames[i]]) + "\":");

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
    return (type.length() >= 4 && type.substr(type.size() - 4, 4) == "list") || type == "translation-unit";
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


cst_value::cst_value(int i) : i(i) { type = INT; }

cst_value::cst_value(char c) : c(c) { type = CHA; }

cst_value::cst_value(double f) : f(f) { type = FLT; }

cst_value::cst_value(string s) : s(std::move(s)) { type = STR; }

cst_value::cst_value(cst_node *node) : node(node) { type = NOD; }

cst_value::cst_value(vector<cnp> nodes) : nodes(nodes) { type = NOA; }

string cst_value::to_string() {
    switch (type) {
        case INT:
            return ::to_string(i);
            break;
        case CHA:
            return ::to_string(c);
            break;
        case FLT:
            return ::to_string(f);
            break;
        case STR:
            return s;
            break;
        case NOD:
            return node->to_string(true);
            break;
        case NOA:
            string re = "[";
            for (int x = 0; x < nodes.size(); x++) {
                re += nodes[x]->to_string();
                if (x != nodes.size() - 1)
                    re += ",";
            }
            re += "]";
            return re;
            break;
    }
}
