//
// Created by 谢威宇 on 2020/4/29.
//

#include "table.h"
#include <utility>
#include <iostream>

int anoy_cnt = 0;
set<pair<string, string>> con;

bool castable(string a, string b) {
    return a == b || con.count({a, b});
}

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
    if (ttype != "basic") {
        re += ttype + " ";
    }
    re += name;
    if (!fields.empty()) {
        re += "{";
        for (auto p :fields) {
            re += p.first.debug() + ":" + p.second + ",";
        }
        re += "}";
    }
    return re;
}

rtype type::get_sub(string name) {
    for (auto p:fields) {
        if (p.second == name) {
            return p.first;
        }
    }
    cout << "Around: " << name << endl;
    throw var_not_defined();
}

string type::llvm_type() {
    if (ttype == "basic") {
        if (name == "void") {
            return "void";
        } else if (name == "char") {
            return "i8";
        } else if (name == "short") {
            return "i16";
        } else if (name == "int" || name == "long" || name == "signed" || name == "unsigned") {
            return "i32";
        } else if (name == "float") {
            return "float";
        } else if (name == "double") {
            return "double";
        }
    } else {
        return name;
    }
}

string type::llvm_type_define() {
    string re = "{";
    for (auto p:fields) {
        re += p.first.llvm_type() + ",";
    }
    re.pop_back();
    re.push_back('}');
    return re;
}

types::types() {
    for (auto s:vector({"void", "char", "short", "int", "long", "float", "double", "signed", "unsigned"})) {
        m[s] = new type("basic", s);
    }
    for (auto s:vector({"char", "short", "int", "long", "float", "double", "signed", "unsigned"})) {
        for (auto b:vector({"char", "short", "int", "long", "float", "double", "signed", "unsigned"})) {
            con.insert({s, b});
        }
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

bool rtype::congruent(const rtype &b) {
    if (castable(b.f->name, f->name) == false) {
        return false;
    }
    int ap = pointer, bp = b.pointer;
    if (array_size.empty() == false)
        ap++;
    if (b.array_size.empty() == false)
        bp++;
    return ap == bp;
}

rtype rtype::get_tar() {
    rtype r = *this;
    if (r.pointer > 0)
        r.pointer--;
    else if (r.array_size.empty() == false) {
        r.array_size.pop_back();
    }
    return r;
}

rtype rtype::get_add() {
    rtype r = *this;
    r.pointer++;
    return r;
}

rtype::rtype() {}

string rtype::llvm_type() {
    string re = f->llvm_type();
    for (int i = 0; i < pointer; i++) {
        re.push_back('*');
    }
    for (auto x:array_size) {
        if (x == -1) {
            re.push_back('*');
        } else {
            re = string("[ ") + to_string(x) + " x " + re + " ]";
        }
    }
    return re;
}


variable::variable(rtype r, const string &name, int scopeLevel) : r(r), name(name), scope_level(scopeLevel) {}


string variable::debug() {
    string re = r.debug();
    re += " " + name;
    re += " scope:" + to_string(scope_level);
    return re;
}

string variable::level_name() {
    if (name[0] <= '9' && name[0] >= '0')
        return name;
    else
        return to_string(scope_level) + "_" + name;
}

variable *variables::get(string x) {
    if (m.count(x) == 0 || m[x].empty()) {
        cout << "Around: " << x << endl;
        throw var_not_defined();
    }
    return m[x].back();
}

vp variables::new_var(rtype r, const string &name, int scopeLevel) {
    if (m.count(name)) {
        if (!m[name].empty() && m[name].back()->scope_level == scopeLevel) {
            cout << r.debug() << " " << name << endl;
            cout << "Around: " << name << endl;
            throw duplicate_definition();
        }
    }
    vp re = new variable(r, name, scopeLevel);
    m[name].push_back(re);
    return re;
}

string variables::debug() {
    string re = "---------Var Table--------\n";
    for (auto p:m) {
        if (p.second.empty() == false) {
            re += p.first + ":\n";
            for (auto v:p.second) {
                re += "    " + v->debug() + "\n";
            }
        }
    }
    return re;
}

fp functions::get(string name) {
    if (m.count(name) == 0) {
        cout << "Around: " << name << endl;
        throw func_not_defined();
    }
    return m[name];
}

fp functions::new_func(rtype r, const string &name) {
    if (m.count(name)) {
        return m[name];
    } else {
        m[name] = new func(r, name);
        return m[name];
    }
}

string functions::debug() {
    string re = "---------Func Table--------\n";
    for (auto p:m) {
        re += p.first + ":";
        re += p.second->debug() + "\n";
    }
    return re;
}

func::func(const rtype &r, const string &name) : r(r), name(name) {
    defined = false;
    temp_varible = 0;
    returned = r.f->name == "void";
}

string func::debug() {
    string re = r.debug();
    re += " " + name + "(";
    for (auto p:parameters) {
        re += p.r.debug() + ":" + p.name + ",";
    }
    re += ")";
    if (!labels.empty()) {
        re += "labels(";
        for (auto l:labels) {
            re += l + ",";
        }
        re += ")";
    }
    return re;
}

void func::new_label(string name) {
    if (labels.count(name)) {
        cout << "Around: " << name << endl;
        throw duplicate_label();
    }
    labels.insert(name);
}

