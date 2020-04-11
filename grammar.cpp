//
// Created by 谢威宇 on 2020/3/30.
//

#include "grammar.h"

grammer_node::grammer_node(string name, class grammar *g) : name(name), grammar(g) {
//    cout << name << endl;
}

void grammer_node::add(vector<string> x) {
    vector<nodep> t;
    for (auto s:x) {
        t.push_back(grammar->next_node(s));
    }
    rules.push_back(t);
}

string grammer_node::to_string() {
    string re;
    string indent = "    ";
    re += name + ": \n";
    for (auto r:rules) {
        re += indent;
        for (int i = 0; i < r.size(); i++) {
            re += r[i]->name + " ";
        }
        re += "\n";
    }
    return re;
}

string grammer_node::to_text() {
    string re = "";
    string indent = "    ";
//        re += "\\begin{equation}\n";
//        re += indent + "\\begin{array}{ll}\n";
    re += indent + indent + texlize(name) + " \\rightarrow ";
    for (int i = 0; i < rules.size(); i++) {
        auto r = rules[i];
        if (i != 0)
            re += indent + indent;
        re += "& ";
        for (int i = 0; i < r.size(); i++) {
            re += texlize(r[i]->name) + " \\,\\, ";
        }
        if (i != rules.size() - 1)
            re += "\\mid";
        re += "\\\\ \n";
    }
//        re += indent + "\\end{array}\n";
//        re += "\\end{equation}\n";
    return re;
}

void grammer_node::substitute(nodep v) {
//    cerr<<this->name<<" substitute "<<v->name<<endl;
    for (auto r:v->rules) {
        for (auto n:r) {
            if (n == v) {
//                cerr << v->name<<" is recursive" << endl;
                return;
            }
        }
    }

    for (int i = 0; i < rules.size(); i++) {
//        cerr << this->to_string() << endl;
        auto r = rules[i];
        for (int j = 0; j < r.size(); j++) {
            auto n = r[j];
            if (n == v) {
                rules.erase(rules.begin() + i);
                for (auto vr:v->rules) {
                    vector nr = r;
                    nr.erase(nr.begin() + j);
                    nr.insert(nr.begin() + j, vr.begin(), vr.end());
                    rules.insert(rules.begin() + i, nr);
                }
            }
        }
    }
}

void grammer_node::substitute_first(nodep v) {
//    cerr<<this->name<<" first substitute "<<v->name<<endl;
    for (auto r:v->rules) {
        if (r.front() == v) {
            cerr << v->name << " is left recursive" << endl;
            return;
        }
    }

    for (int i = 0; i < rules.size(); i++) {
//        cerr << this->to_string() << endl;
        auto r = rules[i];
        auto n = r[0];
        if (n == v) {
            rules.erase(rules.begin() + i);
            for (auto vr:v->rules) {
                vector nr = r;
                nr.erase(nr.begin());
                nr.insert(nr.begin(), vr.begin(), vr.end());
                rules.insert(rules.begin() + i, nr);
            }
        }
    }
}

set<nodep> grammer_node::get_first() {
    if (first.empty()) {
        set<nodep> re;
        if (grammar->isvt(this)) {
            re.insert(this);
        } else {
            get_first_by_rule();
            for (auto x:first_by_rule)
                re.insert(x.begin(), x.end());
        }
        first = re;
    }
    return first;
}

vector<set<nodep>> grammer_node::get_first_by_rule() {
    if (first_by_rule.empty()) {
        vector<set<nodep>> re;
        for (auto r:rules) {
            set<nodep> s;
            for (int i = 0; i < r.size(); i++) {
                if (r[i]->grammar->isvt(r[i]))
                    s.insert(r[i]);
                else {
                    if (r[i] != this) {
                        auto x = r[i]->get_first();
                        for (auto xx:x) {
                            if (xx != grammar->next_node("epsilon"))
                                s.insert(xx);
                        }
                    }
                }
                if (r[i]->de_epsilon() == false)
                    break;
                if (i == r.size() - 1) {
                    s.insert(grammar->next_node("epsilon"));
                }
            }
            re.push_back(s);
        }
        first_by_rule = re;
    }
    return first_by_rule;
}

grammer_node::~grammer_node() {}

bool grammer_node::de_epsilon() {
    if (name == "epsilon")
        return true;
    for (auto r:rules) {
        bool ok = true;
        for (auto n:r) {
            if (n != this) {
                if (n->de_epsilon() == false) {
                    ok = false;
                    break;
                }
            }
        }
        if (ok)
            return true;
    }
    return false;
}

set<itemp> grammer_node::get_items(function<bool(itemp)> filter) {
    set<itemp> re;
    for (auto r:rules) {
        for (int i = 0; i <= r.size(); i++) {
            if (filter == nullptr || filter(get_item(r, i)))
                re.insert(get_item(r, i));
        }
    }
    return re;
}

itemp grammer_node::get_item(vector<nodep> rule, int pos) {

    if (itemps.count({rule, pos}) == 0) {
        auto x = new item(this, rule, pos);
        if (x->dot_position != pos) {
            itemps[{rule, pos}] = get_item(rule, x->dot_position);
        } else {
            itemps[{rule, pos}] = x;
        }
    }
    return itemps[{rule, pos}];
}

bool grammer_node::is_vt() {
    return rules.empty();
}


nodep grammar::next_node(string name) {
    if (all_vertices.count(name) == 0) {
        all_vertices[name] = new grammer_node(name, this);
    }
    return all_vertices[name];
}

void grammar::read_rules() {
    char line[1000];
    nodep now = nullptr;
    bool oneof = false;
    while (cin.getline(line, 1000)) {
        vector<string> xs = split(line);
        if (xs.empty())
            continue;
        if (xs[0] == "fuck")
            break;

        if (xs[0].back() == ':' && xs[0].length() > 1) {
//            cout<<xs[0].substr(0,xs[0].length()-1)<<endl;
            string newname = xs[0].substr(0, xs[0].length() - 1);
            now = next_node(newname);
            nodes.push_back(now);
            if (xs.size() >= 3 && xs[xs.size() - 2] == "one" && xs[xs.size() - 1] == "of") {
                oneof = true;
            } else {
                oneof = false;
            }
        } else {
            if (oneof) {
                for (auto s:xs) {
                    now->add(vector<string>({s}));
                }
            } else {

                map<int, int> opts;
                for (int i = 0; i < xs.size(); i++) {
                    string s = xs[i];
                    if (s.length() >= 3 && s.substr(s.length() - 3, 3) == "opt") {
                        opts[i] = opts.size();
                    }
                }
                for (int k = 0; k < (1 << opts.size()); k++) {
                    bitset<32> w = k;
                    vector<string> ad;
                    for (int i = 0; i < xs.size(); i++) {
                        if (opts.count(i) == 0) {
                            ad.push_back(xs[i]);
                        } else {
                            if (w[opts[i]] == 1) {
                                ad.push_back(xs[i].substr(0, xs[i].length() - 3));
                            }
                        }
                    }
//                for(auto s:ad){
//                    cout<<s<<" ";
//                }
//                cout<<endl;
                    if (ad.size())
                        now->add(ad);
                }
            }
        }
    }
}

vector<nodep> grammar::get_all_Vn() {
    vector<nodep> re;
    for (auto x:all_vertices) {
        if (!x.second->is_vt()) {
            re.push_back(x.second);
        }
    }
    return re;
}

vector<nodep> grammar::get_all_Vt() {
    vector<nodep> re;
    for (auto x:all_vertices) {
        if (x.second->is_vt()) {
            re.push_back(x.second);
        }
    }
    return re;
}

vector<nodep> grammar::get_root() {
//    map<nodep, bool> m;
//    for (auto x:all_vertices) {
//        if (m.count(x.second) == 0) {
//            m[x.second] = true;
//        }
//        for (auto r:x.second->rules) {
//            for (auto p:r) {
//                if (p != x.second)
//                    m[p] = false;
//            }
//        }
//    }
//    vector<nodep> re;
//    for (auto p:m) {
//        if (p.second) {
//            re.push_back(p.first);
//        }
//    }
//    if (re.empty()) {
//        return vector<nodep>({nodes[0]});
//    }
//    return re;

    return vector<nodep>({nodes[0]});
}

void grammar::print_rules(bool tex) {
    printf("All Vn:\n");
    for (auto p : get_all_Vn()) {
        if (tex) {
            printf("        & %s, \\\\\n", texlize(p->name).data());
        } else {
            printf("        %s\n", p->name.data());
        }

    }
    printf("\n");
    printf("All Vt:\n");
    for (auto p : get_all_Vt()) {
        if (tex) {
            printf("        & %s, \\\\\n", texlize(p->name).data());
        } else {
            printf("        %s\n", p->name.data());
        }
    }
    printf("\n");
    printf("All S:\n");
    for (auto p : get_root()) {
        if (tex) {
            printf("        & %s, \\\\\n", texlize(p->name).data());
        } else {
            printf("        %s\n", p->name.data());
        }
    }

    printf("All rules:\n");
    for (auto x:nodes) {
        nodep p = x;
        if (p->rules.size()) {
            if (tex)
                printf("%s\n", p->to_text().data());
            else
                printf("%s\n", p->to_string().data());
        }
    }

}

grammar::grammar(const grammar &g) {
    for (auto n:g.nodes) {
        auto newn = next_node(n->name);
        nodes.push_back(newn);
        for (const auto &r:n->rules) {
            vector<nodep> x;
            x.reserve(r.size());
            for (auto nn:r) {
                x.push_back(next_node(nn->name));
            }
            newn->rules.push_back(x);
        }
    }
}

grammar *grammar::eliminate_left_recursion() {
    grammar *re = new grammar(*this);
    for (int i = 0; i < re->nodes.size(); i++) {
//        re->print_rules();
        nodep u = re->nodes[i];
        //eliminate left recursion for node i
        for (int j = 0; j < i; j++) {
            nodep v = re->nodes[j];
            u->substitute_first(v);
        }

        bool leftr = false;
        for (int j = 0; j < u->rules.size(); j++) {
            auto r = u->rules[j];
            if (r[0] == u) {
                leftr = true;
                break;
            }
        }
        if (leftr) {
            nodep uu = re->next_node(u->name + "'");
            re->nodes.insert(re->nodes.begin() + i + 1, uu);
            vector<vector<nodep>> unew_compose;
            for (auto r:u->rules) {
                if (r[0] == u) {
                    vector<nodep> x;
                    x.insert(x.begin(), r.begin() + 1, r.end());
                    x.push_back(uu);
                    uu->rules.push_back(x);
                } else {
                    r.push_back(uu);
                    unew_compose.push_back(r);
                }
            }
            uu->rules.push_back(vector<nodep>({re->next_node("epsilon")}));
            u->rules = unew_compose;
        }
    }
    stack<nodep> st;
    for (auto s:re->get_root()) {
        st.push(s);
    }
    set<nodep> vis;
    while (st.size()) {
        auto u = st.top();
//        cout<<"dfs: "<<u->name<<endl;
        st.pop();
        vis.insert(u);
        for (const auto &r:u->rules) {
            for (auto n:r) {
                if (vis.count(n) == 0) {
                    st.push(n);
                }
            }
        }
    }
    for (auto p = re->all_vertices.begin(); p != re->all_vertices.end(); p++) {
        if (vis.count(p->second) == 0) {
            cout << "delete " << p->second->name << endl;

            for (int i = 0; i < re->nodes.size(); i++) {
                cout << re->nodes[i]->name << endl;
                if (re->nodes[i] == p->second) {
                    re->nodes.erase(re->nodes.begin() + i);
                    break;
                }
            }

            delete p->second;
            all_vertices.erase(p);

        }
    }
    return re;
}

grammar::~grammar() {
    for (auto p:all_vertices) {
        delete p.second;
    }
}

void grammar::print_first() {
    cout << "----First:----" << endl;
    set<nodep> Vt;
    for (auto n:get_all_Vt())
        Vt.insert(n);
    for (auto p:all_vertices) {
        auto u = p.second;
        if (isvt(u))
            continue;
        auto f = u->get_first_by_rule();
        cout << u->name << ":" << endl;
        for (auto x:u->get_first()) {
            cout << "    " << x->name << endl;
        }
        cout << endl;
        for (int i = 0; i < u->rules.size(); i++) {
            auto r = u->rules[i];
            cout << "    ";
            for (auto n:r) {
                cout << n->name << " ";
            }
            cout << ": ";
            for (auto x:f[i]) {
                cout << x->name << " ";
            }
            cout << endl;
        }
    }
}


bool grammar::isvt(nodep x) {
    return x->is_vt();
}

map<nodep, set<nodep>> grammar::get_follow() {
    if (follow.size())
        return follow;

    map<nodep, set<nodep> > last;
    map<nodep, set<nodep> > re;
    re[get_root()[0]].insert(next_node("#"));
    while (last != re) {
        last = re;
        for (auto p:all_vertices) {
            auto u = p.second;
            if (isvt(u))
                continue;
            for (auto r:u->rules) {
                for (int i = 0; i < r.size() - 1; i++) {
                    if (isvt(r[i]))
                        continue;
//                    if (r[i + 1]->name == "epsilon")
//                        continue;
                    for (int j = i + 1; j < r.size(); j++) {
                        for (auto x:r[j]->get_first()) {
                            if (x->name != "epsilon") {
                                re[r[i]].insert(x);
                            }
                        }
                        if (r[j]->de_epsilon() == false)
                            break;
                    }
                }
            }
            for (auto r:u->rules) {
                if (!isvt(r.back())) {
                    re[r.back()].insert(re[u].begin(), re[u].end());
                }

                for (int i = r.size() - 1; i > 0; i--) {
                    if (r[i]->de_epsilon()) {
                        if (isvt(r[i - 1]))
                            continue;
                        re[r[i - 1]].insert(re[u].begin(), re[u].end());
                    } else {
                        break;
                    }
                }
            }
        }
    }
    follow = re;
    return re;
}

void grammar::print_follow() {
    cout << "----Follow:----" << endl;
    for (auto p:get_follow()) {
        cout << p.first->name << ": " << endl;
        for (auto x:p.second) {
            cout << "    " << x->name << endl;
        }
    }
}

map<nodep, set<nodep>> grammar::get_first() {
    if (first.size())
        return first;
    map<nodep, set<nodep>> re;
    for (auto x:get_all_Vn()) {
        re[x] = x->get_first();
    }
    first = re;
    return re;
}

map<pair<nodep, nodep>, vector<vector<nodep>>> grammar::get_LL1form() {
    if (LL1form.size()) {
        return LL1form;
    }

    map<pair<nodep, nodep>, vector<vector<nodep>>> re;
    get_follow();
    get_first();

    for (auto p:all_vertices) {
        auto u = p.second;

        auto ufi = u->get_first_by_rule();
        auto ufo = follow[u];
        for (int i = 0; i < u->rules.size(); i++) {
            auto r = u->rules[i];
            if (ufi[i].count(next_node("epsilon"))) {
                for (auto a:ufo) {
                    re[{u, a}].push_back(r);
                }
            } else {
                for (auto a:ufi[i]) {
                    re[{u, a}].push_back(r);
                }
            }
        }
    }
    LL1form = re;
    return re;
}

void grammar::print_LL1form() {
    cout << "----LL1 form----" << endl;
    get_LL1form();
    for (auto p: LL1form) {
        printf("%s %s\n", p.first.first->name.data(), p.first.second->name.data());
        for (auto r:p.second) {
            printf("    ");
            for (auto n:r) {
                printf("%s ", n->name.data());
            }
            printf("\n");
        }
    }
}

closurep grammar::get_closure(set<itemp> items) {
    set<itemp> last;
    while (items != last) {
        last = items;
        for (auto x:items) {
            auto y = x->get_closure_items();
            items.insert(y.begin(), y.end());
        }
    }
    if (!closures.count(items))
        closures[items] = new closure(items);
    return closures[items];
}

void grammar::build_closures() {
    auto c = get_closure(get_root()[0]->get_items([](itemp p) -> bool { return p->dot_position == 0; }));
    c->expand();
    head_closure = c;
}


void grammar::print_closures() {
    vector<closurep> c;
    for (auto p:closures) {
        c.push_back(p.second);
    }
    sort(c.begin(), c.end(), [](closurep a, closurep b) -> bool { return a->mycnt < b->mycnt; });
    for (auto p:c) {
        cout << p->to_string();
    }
}


item::item(nodep left, vector<nodep> rule, int dot_position) : left(left), rule(std::move(rule)),
                                                               dot_position(dot_position) {
    if (this->rule.size() == 1 && this->rule[0]->name == "epsilon") {
        this->rule.clear();
        this->dot_position = 0;
    }
}

set<itemp> item::get_closure_items() {
    if (!closure_items.empty())
        return closure_items;
    grammerp g = left->grammar;
    set<itemp> re;
    set<itemp> last;
    re.insert(this);
    while (last != re) {
        last = re;
        for (auto p:re) {
            if (p->dot_position < p->rule.size()) {
                nodep nxt = p->rule[p->dot_position];
                if (g->isvt(nxt) == false) {
                    for (auto i:nxt->get_items()) {
                        if (i->dot_position == 0) {
                            re.insert(i);
                        }
                    }
                }
            }
        }
    }
    return re;
}

string item::to_string(string prefix) {
    prefix += left->name + " ->";
    for (int i = 0; i < rule.size(); i++) {
        if (i == dot_position)
            prefix += " ·";
        prefix += " " + rule[i]->name;
    }
    if (dot_position == rule.size())
        prefix += " ·";

    prefix += "  " + get_kind_str();


    prefix += "\n";
    return prefix;
}

item::kind item::get_kind() {
    if (dot_position == rule.size()) {
        if (left == left->grammar->get_root()[0])
            return ACC;
        else
            return RDC;
    } else {
        auto nxt = rule[dot_position];
        if (nxt->is_vt())
            return SFT;
        else
            return TRD;
    }
}

std::string item::get_kind_str() {
    return std::map<kind, std::string>({
                                               {ACC, "accept"},
                                               {RDC, "reduce"},
                                               {SFT, "shift"},
                                               {TRD, "to be reduce"}
                                       })[get_kind()];
}

int closure::cnt = 0;

closure::closure(set<itemp> s) : items(std::move(s)) {
    cnt++;
    mycnt = cnt;
}

void closure::expand() {
    if (items.empty())
        return;
    expanded = true;
    grammerp g = (*items.begin())->left->grammar;

    map<nodep, set<itemp> > nxt;
    for (auto i:items) {
        if (i->dot_position < i->rule.size()) {
            auto nxt_node = i->rule[i->dot_position];
            nxt[nxt_node].insert(i->left->get_item(i->rule, i->dot_position + 1));
        }
    }
    for (const auto &p:nxt) {
        go[p.first] = g->get_closure(p.second);
    }
    for (const auto &p:go) {
        if (p.second->expanded == false)
            p.second->expand();
    }
}

string closure::to_string() {
    string re = "Closure " + ::to_string(mycnt) + ": \n";

    for (auto i:items) {
        re += i->to_string("    ");
    }
    re += "\n";
    for (auto p:go) {
        re += "through ";
        re += p.first->name;
        re += " to Closure ";
        re += ::to_string(p.second->mycnt);
        re += "\n";
    }
    re += "\n";
    return re;
}
