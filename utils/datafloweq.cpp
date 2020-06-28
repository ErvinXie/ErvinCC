//
// Created by 谢威宇 on 2020/6/27.
//

#include "datafloweq.h"
#include <iostream>
#include <fstream>

void code::readfromfile() {
    while (fin.get() != EOF) {
        build_block();
    }
}

code::code(const string &dir) : dir(dir) {
    fin = ifstream(dir);
    lineid = 0;
    for (int i = (int) dir.length() - 1; i >= 0; i--) {
        if (dir[i] == '.') {
            dirh = dir.substr(0, i);
            break;
        }
    }
}

codeblock *code::getblock(int id) {
    if (blocks.count(id) == 0)
        blocks[id] = new codeblock(id);
    return blocks[id];
}

var *code::getvar(string name) {
    if (vars.count(name) == 0)
        vars[name] = new var(name);
    return vars[name];
}

codeline *code::getline(int id) {
    if (lines.count(id) == 0)
        lines[id] = new codeline(id);
    return lines[id];
}


void code::build_block() {
    char temp[1000];
    fin.getline(temp, 1000);
    string t(temp);

    auto id = ::atoi(split_space(t)[1].data());

    auto b = getblock(id);


    fin.getline(temp, 1000);
    t = string(temp);
    for (auto s:split_space(t)) {
        if (s.back() == ':')
            continue;
        auto to = ::atoi(s.data());
        b->to.push_back(getblock(to));
        getblock(to)->from.push_back(b);
    }

    while (true) {
        fin.getline(temp, 1000);
        t = string(temp);
        if (t == "")
            break;
        build_line(b, t);
    }
}

codeline *code::build_line(codeblock *block, string content) {
    lineid++;
    auto l = getline(lineid);
    l->b = block;
    block->lines.push_back(l);

    bool ifuse = false;
    for (auto name:split_space(content)) {
        if (name == "=") {
            ifuse = true;
            continue;
        }
        l->addvar(getvar(name), ifuse);
    }
    return l;
}


void codeline::addvar(var *v, bool isuse) {
    if (isuse) {
        use.push_back(v);
        v->use.insert(this);
    } else {
        def.push_back(v);
        v->def.insert(this);
    }
}

void codeline::debug() {
    cout << "d" << id << ": DEF ";
    for (auto x:def) {
        cout << x->name << " ";
    }
    cout << " USE ";
    for (auto x:use) {
        cout << x->name << " ";
    }
    cout << endl;
}

codeline::codeline(int id) : id(id) {}


void codeblock::debug() {
    cout << "B" << id << ": " << endl;
    cout << "From:" << endl;
    for (auto x:from) {
        cout << "B" << x->id << " ";
    }
    cout << endl;

    cout << "To:" << endl;
    for (auto x:to) {
        cout << "B" << x->id << " ";
    }
    cout << endl;

    for (auto l :lines) {
        l->debug();
    }

}

codeblock::codeblock(int id) : id(id) {}

void code::debug() {
    for (auto b:blocks) {
        b.second->debug();
        cout << endl;
    }
    for (auto v:vars) {
        v.second->debug();
        cout << endl;
    }
}

void code::compute_in_out() {
    for (auto bp:blocks) {
        set<codeline *> all;
        auto b = bp.second;
        set<var *> genv;
        for (int i = (int) b->lines.size() - 1; i >= 0; i--) {
            auto l = b->lines[i];
            if (!l->def.empty()) {
                bool ins = false;
                for (auto v:l->def) {
                    if (genv.count(v) == 0) {
                        ins = true;
                        genv.insert(v);
                    }
                }
                if (ins) {
                    gen[b].insert(l);
                    for (auto v:l->def) {
                        all.insert(v->def.begin(), v->def.end());
                    }
                }
            }
        }
        set_difference(all.begin(), all.end(), gen[b].begin(), gen[b].end(), inserter(kill[b], kill[b].begin()));
        out[b] = gen[b];
    }
    bool go = true;
    while (go) {
        go = false;
        for (auto bp: blocks) {
            auto b = bp.second;
            set<codeline *> newin, newout;

            for (auto from:b->from) {
                newin.insert(out[from].begin(), out[from].end());
            }

            if (newin != in[b]) {
                go = true;
                in[b] = newin;
                set_difference(newin.begin(), newin.end(), kill[b].begin(), kill[b].end(),
                               inserter(newout, newout.begin()));
                newout.insert(gen[b].begin(), gen[b].end());
                out[b] = newout;
            }
        }

    }
}

void code::show_form() {
    ofstream fout(dirh + "forms.md");
    fout << "|BLOCK|GEN|KILL|IN|OUT|" << endl;
    fout << "|--|--|--|--|--|" << endl;

    for (auto bp:blocks) {
        fout << "|";
        auto b = bp.second;
        fout << "B" << b->id;
        for (auto s:vector<decltype(gen) *>({&gen, &kill, &in, &out})) {
            fout << "|";
            for (auto l:(*s)[b]) {
                fout << "d" << l->id << ",";
            }
        }
        fout << "|" << endl;
    }
    fout << endl;

    fout << "ud:" << endl;
    for (auto bp:blocks) {
        auto b = bp.second;
        fout << "Block " << b->id << ":" << endl;
        map<var *, codeline *> dvl;
        for (auto l:b->lines) {
            for (auto uv:l->use) {
                vector<codeline *> ud;
                if (dvl.count(uv)) {
                    ud.push_back(dvl[uv]);
                } else {
                    for (auto dl:in[b]) {
                        for (auto dv:dl->def) {
                            if (dv == uv) {
                                ud.push_back(dl);
                            }
                        }
                    }
                }
                fout << uv->name << "(" << l->id << "): {";
                for (auto dl:ud) {
                    fout << "d" << dl->id << ",";
                }
                fout << "}" << endl;
            }
        }
    }

}

void code::computeL() {
    for (auto bp:blocks) {
        auto b = bp.second;
        set<var *> defv, usev;
        for (auto l : b->lines) {
            set<var *> ldefv, lusev;
            for (auto uv:l->use) {
                if (defv.count(uv) == 0) {
                    lusev.insert(uv);
                    usel[uv][b].insert(l);
                }
            }
            for (auto dv:l->def) {
                if (usev.count(dv) == 0) {
                    ldefv.insert(dv);
                    defl[dv][b].insert(l);
                }
            }
            defv.insert(ldefv.begin(), ldefv.end());
            usev.insert(lusev.begin(), lusev.end());
        }
    }
    for (auto vp:vars) {
        auto v = vp.second;

        vector<codeblock *> rbs;
        for (auto bp:blocks) {
            rbs.push_back(bp.second);
        }
        ::reverse(rbs.begin(), rbs.end());

        bool go = true;

        while (go) {

            go = false;
            for (auto b:rbs) {
                set<codeline *> newout, newin;
                for (auto son:b->to) {
                    newout.insert(inl[v][son].begin(), inl[v][son].end());
                }
                outl[v][b] = newout;

                if (defl[v][b].empty())
                    newin = newout;

                newin.insert(usel[v][b].begin(), usel[v][b].end());
                if (inl[v][b] != newin) {
                    go = true;
                    inl[v][b] = newin;
                }
            }
        }

    }
}

void code::show_formL() {
    ofstream fout(dirh + "formsL.md", ios::app);
    fout << "|BLOCK|DEF|USE|IN|OUT|" << endl;
    fout << "|--|--|--|--|--|" << endl;

    for (auto bp:blocks) {
        fout << "|";
        auto b = bp.second;
        fout << "B" << b->id;
        for (auto s:vector<decltype(defl) *>({&defl, &usel, &inl, &outl})) {
            fout << "|";
            for (auto x:(*s)) {
                if (!x.second[b].empty()) {
                    fout << x.first->name << "(";
                    bool first = true;
                    for (auto l:x.second[b]) {
                        if (first)
                            first = false;
                        else
                            fout << ",";
                        fout << l->id;
                    }
                    fout << "),";
                }
            }
        }
        fout << "|" << endl;
    }
    fout << endl;
}

var::var(const string &name) : name(name) {}

void var::debug() {
    cout << "VAR " << name << " :" << endl;
    cout << "DEF: ";
    for (auto l:def) {
        cout << "d" << l->id << " ";
    }
    cout << endl;
    cout << "USE: ";
    for (auto l:use) {
        cout << "d" << l->id << " ";
    }
    cout << endl;
}

int main() {
    code c("/Users/ervinxie/ClionProjects/ErvinCC/utils/test.txt");
    c.readfromfile();
    c.debug();
    c.compute_in_out();
    c.show_form();
    c.computeL();
    c.show_formL();

    return 0;
}

