//
// Created by 谢威宇 on 2020/3/30.
//

#include "utils.h"
#include <set>

map<char, string> esc({
                              {'{',  "\\{"},
                              {'}',  "\\}"},
                              {'^',  "\\widehat{}"},
                              {'_',  "\\_"},
                              {'&',  "\\&"},
                              {'\\', "\\\\"},
                              {'%',  "\\%"},
                              {'~',  "\\sim"}
                      });

vector<string> split_space(string s) {
    return split(s, " \t");
}

vector<string> split(string s, string se) {
    vector<string> re;
    set<char> x;
    string t;
    x.insert(se.begin(), se.end());
    for (auto c:s) {
        if (x.count(c)) {
            if (t.length()) {
                re.push_back(t);
                t = "";
            }
        } else {
            t.push_back(c);
        }
    }
    if (t.size())
        re.push_back(t);
    return re;
}

vector<string> split_q(string s, string se) {
    vector<string> re;
    set<char> x;
    string t;
    x.insert(se.begin(), se.end());
    bool instr = false;
    bool esc = false;
    for (auto c:s) {
        if (x.count(c) && !instr) {
            if (t.length()) {
                re.push_back(t);
                t = "";
            }
        } else {
            t.push_back(c);
            if (instr) {
                if (esc) {
                    esc = false;
                } else {
                    if (c == '\\')
                        esc = true;
                    if (c == '"')
                        instr = false;
                }
            } else {
                if (c == '"')
                    instr = true;
            }
        }
    }
    if (t.size())
        re.push_back(t);
    return re;
}

string texlize(string s) {
    string re;
    char last = ' ';
    for (auto c:s) {
        if (esc.count(c)) {
            re += esc[c] + " ";
        } else {
            if (c == '-' && ((last <= 'z' && last >= 'a') || (last <= 'Z' && last >= 'A'))) {
                re += "\\text{-}";
            } else {
                re.push_back(c);
            }
        }
        last = c;
    }
    return re;
}




