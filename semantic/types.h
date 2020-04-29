//
// Created by 谢威宇 on 2020/4/29.
//

#ifndef ERVINCC_TYPES_H
#define ERVINCC_TYPES_H

#include <string>
#include <vector>
#include <map>
#include <set>
#include "exceptions.h"

using namespace std;

class type;

class rtype {
public:
    type *f;
    int pointer;
    set<string> qualifiers;
    vector<int> array_size;

    rtype(type *t, set<string> qua = set<string>({}), int pcnt = 0);

    string debug();
};


class type {
public:
    string name;
    string ttype;
    bool defined;

    explicit type(string t, const string &x);

    vector<pair<rtype, string>> fields;

    string debug();

};

class types {
public:
    types();

    map<string, type *> m;

    type *new_type(string t, string x = "");

    type *get(string x);

    string debug();
};

#endif //ERVINCC_TYPES_H
