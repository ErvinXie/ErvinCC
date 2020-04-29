//
// Created by 谢威宇 on 2020/4/29.
//

#include "types.h"

#include <utility>
#include <iostream>

int anoy_cnt = 0;

type::type(string t, const string &x) : ttype(t) {
    if (x.empty()) {
        name = "__" + t + "__" + to_string(++anoy_cnt);
    } else {
        name = x;
    }
    defined = false;
}

string type::debug() {
    string re;
    if(ttype!="basic"){
        re+=ttype+" ";
    }
    re += name ;
    if (!fields.empty()) {
        re += "{";
        for (auto p :fields) {
            re += p.first.debug() + ":" + p.second + ",";
        }
        re += "}";
    }
    return re;
}

types::types() {
    for (auto s:vector({"void", "char", "short", "int", "long", "float", "double", "signed", "unsigned"})) {
        m[s] = new type("basic", s);
    }
}

type *types::get(string x) {
    return m[x];
}

type *types::new_type(string t, string x) {
    if (m.count(x)) {
        return m[x];
    } else {
        auto te = new type(std::move(t), x);
        m[te->name] = te;
        return te;
    }
}

string types::debug() {

    string re = "---------Type Table---------\n";
    for (auto p:m) {
        re += p.first + " : " + p.second->debug() + "\n";
    }
    return re;
}

rtype::rtype(type *t, set<string> qua, int pcnt) :
        f(t), qualifiers(std::move(qua)), pointer(pcnt) {}

string rtype::debug() {
    string re;
    for (auto q:qualifiers)
        re += q + " ";
    re += f->debug();
    for (int i = 0; i < pointer; i++)
        re += "*";
    if (!array_size.empty()) {
        re += "array(";
        for (auto x:array_size) {
            re += to_string(x) + ",";
        }
        re += ")";
    }
    return re;
}
