//
// Created by 谢威宇 on 2020/6/5.
//

#include "codegen.h"

#include "../utils/utils.h"
#include <string>
#include <fstream>
#include <iostream>
#include <vector>

using namespace std;

int main(int argc, char *argv[]) {
    string in_dir = argv[1], outdir;
    ifstream in(in_dir);
    cout << in_dir << endl;
    for (int i = (int) in_dir.length() - 1; i >= 0; i--) {
        if (in_dir[i] == '.') {
            outdir = in_dir.substr(0, i) + ".asm";
            break;
        }
    }
    cout << outdir << endl;

    char b[1000];
    vector<string> ir;
    while (in.getline(b, 1000)) {
        string s(b);
        for (int i = 0; i < s.length(); i++) {
            if (s[i] == ';') {
                s = s.substr(0, i);
                break;
            }
        }
        if (!s.empty()) {
            ir.push_back(s);
        }
    }
    codegen c(ir, outdir);
    c.gen();
    cout << "MIPS Generate OK" << endl;

    return 0;
}

codegen::codegen(const vector<string> &ir, string outdir) : ir(ir) {
    now = 0;
    fcnt = 0;
    out.open(outdir);
    r[0].name = "$zero";
    for (int i = 2; i <= 3; i++) {
        r[i].name = "$v" + to_string(i - 2);
    }
    for (int i = 4; i <= 7; i++) {
        r[i].name = "$a" + to_string(i - 4);
        a[i - 4] = &r[i];
    }
    for (int i = 8; i <= 15; i++) {
        r[i].name = "$t" + to_string(i - 8);
    }
    for (int i = 24; i <= 25; i++) {
        r[i].name = "$t" + to_string(i - 24 + 8);
    }
    for (int i = 16; i <= 23; i++) {
        r[i].name = "$s" + to_string(i - 6);
    }
    r[31].name = "$ra";

}

void codegen::gen() {

    while (now < ir.size()) {
        auto s = ir[now];
//        cout << s << endl;
        auto ts = split(s, " ,");
        if (ts[0] == "define") {
            fcnt++;
            genfunc();
        } else {
            now++;
        }
    }
    out << global.str() << endl << function.str() << endl;
}


void codegen::genfunc() {
    toass.clear();
    ostringstream con, body;
    con << ".data" << endl;
    var_type *ret;
    int pcnt = 1;
    string fname;
    while (now < ir.size()) {
        auto s = ir[now];
        body << "#" << s << endl;
        auto ts = split_q(s, " ,");
        if (ts[0] == "define") {
            ret = new var_type(ts[1]);
            int i;
            for (i = 1;; i++) {
                if (ts[2][i] == '(') {
                    break;
                }
                fname.push_back(ts[2][i]);
            }
            if (ts[2][i + 1] == ')') {
                pcnt = 0;
            } else {
                pcnt = ts.size() - 2;
            }
            for (i = 0; i < pcnt; i++) {
                auto v = new var("", this);
                v->va = a[i];
                a[i]->v = v;
                toass["%" + to_string(i)] = v;
            }
            body << ".text" << endl;
            body << "fun_" << fname << ":" << endl;
        } else if (ts.size() == 1 && ts[0].back() == ':') {
            body << "label_" << ts[0] << endl;
        } else if (ts[2] == "alloca") {
            var_type t(ts[3]);
            string name = "f" + to_string(fcnt) + "_" + ts[0].substr(1, ts[0].length() - 1);
            toass[ts[0]] = new var(name, this);
            toass[ts[0]]->t = t;

            con << name << ": " << t.get_unit_descripter();
            if (ts.size() < 5) {
                con << " 0 : " << t.all_count() << endl;
            } else {
                con << " " << ts[4] << endl;
            }
        } else if (ts[0] == "}") {
            now++;
            break;
        } else if (ts[1] == "store") {
            if (ts[2].length() > 1 && ts[2][0] == '%') {
                if (ts[2].back() != ')') {
                    auto b = toass[ts[2]]->val(body);
                    auto a = toass[ts[0]]->add(body);
                    body << "    sw " << b << " , 0(" << a << ")" << endl;
                    toass[ts[2]]->unbind();
                    toass[ts[0]]->unbind();
                    body << endl;
                } else {
                    body << "array access not supported" << endl;
//                    auto v = split(ts[2], "()");
//                    for (auto x:v) {
//                        body << x << endl;
//                    }
//
//                    auto tr = idle();
//                    tr->v = reinterpret_cast<var *>(1);
//                    body << "    addi " << tr->name << " , " << r[0].name << " , " << " 1 " << endl;
//                    for (int i = 1; i < v.size(); i++) {
//                        auto d = toass[v[i]];
//                        auto dv = d->val(body);
//                        body << "    mul " << tr->name << " , " << tr->name << " , " << dv << endl;
//                        d->unbind();
//                    }
//                    auto b = toass[v[0]];
//                    auto ba = b->add(body);
//                    auto a = toass[ts[0]];
//                    auto aa = a->add(body);

                }
            } else if (ts[2][0] >= '0' && ts[2][0] <= '9') {
                auto a = toass[ts[0]]->add(body);
                auto tr = idler();
                body << "    addi " << tr << " , " << r[0].name << " , " << ts[2] << endl;
                body << "    sw " << tr << " , 0(" << a << ")" << endl;
                toass[ts[0]]->unbind();
                body << endl;
            } else if (ts[2] == "call") {
                int d;
                d = 2;


                string cname;
                for (int i = 1; i < ts[d + 2].length(); i++) {
                    if (ts[d + 2][i] == '(') {
                        break;
                    }
                    cname.push_back(ts[d + 2][i]);
                }
                vector<var *> pars;
                vector<var_type> vt;
                for (int i = 3; i < ts.size(); i += 2) {
                    if (ts[d + i] == ")")
                        break;
                    vt.push_back(var_type(ts[d + i]));
                    pars.push_back(toass[ts[d + i + 1]]);
                }
                for (int i = 0; i < pars.size(); i++) {
                    auto p = pars[i];
                    auto pa = p->add(body);
                    if (vt[i].subtype != nullptr) {
                        body << "    move $a" << i << " , " << pa << endl;
                    } else {
                        body << "    lw $a" << i << " , 0(" << pa << ")" << endl;
                    }
                    p->unbind();
                }
                map<string, string> infun({{"Mars_PrintInt", "1"},
                                           {"Mars_PrintStr", "4"},
                                           {"Mars_GetInt",   "5"}});

                {
                    body << push("$ra") << endl;
                    for (auto p:toass) {
                        if (p.first.back()>'9'||p.first.back()<'0') {
                            auto pv = p.second->val(body);
                            body << push(pv) << endl;
                            p.second->unbind();
                        }
                    }
                }
                if (infun.count(cname)) {
                    body << "    li $v0 , " << infun[cname] << endl;
                    body << "    syscall" << endl << endl;
                } else {
                    body << "    jal fun_" << cname << endl;
                }

                {
                    vector<var *> v;
                    for (auto p:toass) {
                        if (p.first.back()>'9'||p.first.back()<'0') {
                            v.push_back(p.second);
                        }
                    }
                    reverse(v.begin(), v.end());
                    for (auto p:v) {

                        auto pa = p->add(body);
                        auto tr = idler();
                        body << pop(tr) << endl;
                        body << "    sw " << tr << " , " << "0(" << pa << ")" << endl;
                        p->unbind();
                    }
                    body << pop("$ra") << endl;
                }

                auto r = toass[ts[0]];
                auto ra = r->add(body);
                body << "    sw  $v0 " << ", 0(" << ra << ")" << endl;
                r->unbind();

                body << endl;

            } else if (ts[2] == "=") {
                auto a = toass[ts[5]], b = toass[ts[7]], r = toass[ts[0]];
                auto aa = a->add(body), bv = b->val(body);
                body << "    sw " << bv << " , 0(" << aa << ")" << endl;
                a->unbind();
                auto ra = r->add(body);
                body << "    sw " << bv << " , 0(" << ra << ")" << endl;
                r->unbind();
                b->unbind();
            } else {
                map<string, string> ops({{"+",  "add"},
                                         {"-",  "sub"},
                                         {"*",  "mul"},
                                         {"/",  "div"},
                                         {"%",  "div"},
                                         {"==", "seq"},
                                         {">=", "sge"},
                                         {">",  "sgt"},
                                         {"<=", "sle"},
                                         {"<",  "slt"},
                                         {"!=", "sne"}});
                if (ops.count(ts[2])) {
                    string op = ops[ts[2]];
                    auto a = toass[ts[5]], r = toass[ts[0]];
                    auto av = a->val(body);
                    string bv;
                    decltype(a) b;
                    if (ts[7][0] == '%') {
                        b = toass[ts[7]];
                        bv = b->val(body);
                    } else {
                        bv = ts[7];
                    }
                    auto tr = idler();
                    if (ts[2] == "/" || ts[2] == "%") {
                        body << "    " + op + " " << av << " , " << bv << endl;
                        if (ts[2] == "/")
                            body << "    mflo " << tr << endl;
                        if (ts[2] == "%")
                            body << "    mfhi " << tr << endl;

                    } else {
                        if (bv[0] != '$') {
                            op += "i";
                        }
                        body << "    " + op + " " << tr << " , " << av << " , " << bv << endl;
                    }
                    a->unbind();
                    if (ts[7][0] == '%')
                        b->unbind();
                    auto ra = r->add(body);
                    body << "    sw " << tr << " , 0(" << ra << ")" << endl;
                    r->unbind();
                }
                body << endl;
            }
        } else if (ts[0] == "br") {
            if (ts[1] == "label") {
                body << "    b label_" << ts[2].substr(1, ts[2].length() - 1) << endl;
            } else {
                auto c = toass[ts[2]];
                auto cv = c->val(body);
                body << "    bgtz " << cv << " , label_" << ts[4].substr(1, (int) ts[4].length() - 1) << endl;
                body << "    b " << " , label_" << ts[6].substr(1, (int) ts[6].length() - 1) << endl;
            }
        } else if (ts[0] == "call") {
            int d = 0;

            string cname;
            for (int i = 1; i < ts[d + 2].length(); i++) {
                if (ts[d + 2][i] == '(') {
                    break;
                }
                cname.push_back(ts[d + 2][i]);
            }
            vector<var *> pars;
            vector<var_type> vt;
            for (int i = 3; i < ts.size(); i += 2) {
                if (ts[d + i] == ")")
                    break;
                vt.push_back(var_type(ts[d + i]));
                pars.push_back(toass[ts[d + i + 1]]);
            }
            for (int i = 0; i < pars.size(); i++) {
                auto p = pars[i];
                auto pa = p->add(body);
                if (vt[i].subtype != nullptr) {
                    body << "    move $a" << i << " , " << pa << endl;
                } else {
                    body << "    lw $a" << i << " , 0(" << pa << ")" << endl;
                }
                p->unbind();
            }
            map<string, string> infun({{"Mars_PrintInt", "1"},
                                       {"Mars_PrintStr", "4"},
                                       {"Mars_GetInt",   "5"}});

            {
                body << push("$ra") << endl;
                for (auto p:toass) {
                    if (p.first.back()>'9'||p.first.back()<'0') {
                        auto pv = p.second->val(body);
                        body << push(pv) << endl;
                        p.second->unbind();
                    }
                }
            }
            if (infun.count(cname)) {
                body << "    li $v0 , " << infun[cname] << endl;
                body << "    syscall" << endl << endl;
            } else {
                body << "    jal fun_" << cname << endl;
            }

            {
                vector<var *> v;
                for (auto p:toass) {
                    if (p.first.back()>'9'||p.first.back()<'0') {
                        v.push_back(p.second);
                    }
                }
                reverse(v.begin(), v.end());
                for (auto p:v) {
                    auto pa = p->add(body);
                    auto tr = idler();
                    body << pop(tr) << endl;
                    body << "    sw " << tr << " , " << "0(" << pa << ")" << endl;
                    p->unbind();
                }
                body << pop("$ra") << endl;
            }
            body << endl;

        } else if (ts[0] == "ret") {
//            cout << s << endl;
            if (ts.size() >= 3) {
                auto a = toass[ts[2]];
                auto ad = a->add(body);
                body << "    lw   $v0, " << "0(" << ad << ")" << endl;
                a->unbind();
            }

            if (fname == "main") {
                body << "    li   $v0, 10" << endl;
                body << "    syscall" << endl;
            } else {
                body << "    jr $ra" << endl;
            }


        } else {
            cout << "?" << endl;
            body << s << endl;
        }

        now++;

    }
    if (fname == "main") {
        global << con.str() << endl << body.str() << endl;
    } else {
        function << con.str() << endl << body.str() << endl;
    }

}

string codegen::idler() {
    for (int i = 8; i <= 25; i++) {
        if (r[i].v == nullptr) {
            return r[i].name;
        }
    }
    return "";
}

string codegen::push(string r) {
    return string("    sub $sp,$sp,4\n") +
           "    sw " + r + ",($sp)\n";
}

string codegen::pop(string r) {
    return string("    lw " + r + ",($sp)\n") +
           "    addiu $sp,$sp,4";
}

reg *codegen::idle() {
    for (int i = 8; i <= 25; i++) {
        if (r[i].v == nullptr) {
            return &r[i];
        }
    }
    return nullptr;
}


var_type::var_type(string s) {
//    cout << s << endl;
    if (s[0] == 'i') {
        size = atoi(s.data() + 1);
        size = size / 8;
        count = 1;
        subtype = nullptr;
    } else if (s == "float") {
        size = 4;
        count = 1;
        subtype = nullptr;
    } else if (s == "double") {
        size = 8;
        count = 1;
        subtype = nullptr;
    } else if (s == "void ") {
        size = 0;
        count = 1;
        subtype = nullptr;
    } else if (s[0] == '[') {
        string c;
        int i = 1;
        for (; s[i] != 'x'; i++) {
            c.push_back(s[i]);
        }
        count = atoi(c.data());
        subtype = new var_type(s.substr(i + 1, s.length() - i - 2));
        size = count * subtype->size;
    }
}

string var_type::get_unit_descripter() {
    if (subtype == nullptr) {
        if (size == 1) {
            return ".asciiz";
        } else if (size == 4)
            return ".word";
    } else {
        return subtype->get_unit_descripter();
    }
}

int var_type::all_count() {
    if (subtype == nullptr) {
        return count;
    } else {
        return count * subtype->count;
    }
}

int var_type::get_unit_size() {
    if (subtype == nullptr) {
        return size;
    } else {
        return subtype->get_unit_size();
    }
}

var::var(const string &name, codegen *c) : name(name), c(c) {}

string var::add(ostringstream &body) {
    if (ad != nullptr) {
        return ad->name;
    } else {
        for (int i = 8; i <= 25; i++) {
            if (c->r[i].v == nullptr) {
                ad = &c->r[i];
                c->r[i].v = this;
                body << "    la " << ad->name << " , " << name << endl;
                return ad->name;
            }
        }
    }
}

string var::val(ostringstream &body) {
    if (va == nullptr) {
        add(body);
        va = ad;
        ad = nullptr;
        body << "    lw " << va->name << " , 0(" << va->name << ")" << endl;
    }
    return va->name;
}

void var::unbind() {
    if (ad != nullptr) {
        ad->v = nullptr;
        ad = nullptr;
    }
    if (va != nullptr) {
        va->v = nullptr;
        va = nullptr;
    }
}
