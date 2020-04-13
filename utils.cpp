//
// Created by 谢威宇 on 2020/3/30.
//

#include "utils.h"
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
    vector<string> re;
    string x;
    for (auto c:s) {
        if (c == ' ' || c == '\t') {
            if (x.length()) {
                re.push_back(x);
                x = "";
            }
        } else {
            x.push_back(c);
        }
    }
    if (x.size())
        re.push_back(x);
    return re;
}


string texlize(string s) {
    string re;
    char last = ' ';
    for (auto c:s) {
        if (esc.count(c)) {
            re += esc[c] + " ";
        } else {
            if (c == '-' && (last <= 'z' && last >= 'a' || last <= 'Z' && last >= 'A')) {
                re += "\\text{-}";
            } else {
                re.push_back(c);
            }
        }
        last = c;
    }
    return re;
}