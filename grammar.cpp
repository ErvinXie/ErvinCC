//
// Created by 谢威宇 on 2020/3/30.
//

#include "grammar.h"

grammar_node::grammar_node(string name, class grammar *g) : name(name), grammar(g) {
//    cout << name << endl;
}

void grammar_node::add_rule(vector<string> x) {
    vector<nodep> t;
    for (const auto &s:x) {
        if (s != "epsilon")
            t.push_back(grammar->get_node(s));
    }
    rules.push_back(t);
}

string grammar_node::to_string() {
    string re;
    string indent = "    ";
    re += name + ": \n";
    for (const auto &r:rules) {
        re += indent;

        re += rule_to_string(r);
        re += "\n";
    }
    return re;
}

string grammar_node::to_text() {
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

void grammar_node::substitute(nodep v) {
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

void grammar_node::substitute_first(nodep v) {
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
        if (r.empty())
            continue;
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

set<nodep> grammar_node::get_first(set<nodep> fathers) {
    fathers.insert(this);
    if (first.empty()) {
        set<nodep> re;
        if (is_vt()) {
            re.insert(this);
        } else {
            for (auto x:get_first_by_rule(fathers))
                re.insert(x.begin(), x.end());
        }
        first = re;
    }
    return first;
}

vector<set<nodep>> grammar_node::get_first_by_rule(set<nodep> fathers) {
    fathers.insert(this);
    if (first_by_rule.empty()) {
        vector<set<nodep>> re;
        for (auto r:rules) {
            set<nodep> s;
            for (int i = 0; i < r.size(); i++) {
                auto &word_i = r[i];
                if (word_i->is_vt()) {
                    s.insert(word_i);
                } else {
                    if (fathers.count(word_i) == 0) {
                        auto word_i_first = word_i->get_first(fathers);
                        for (auto x:word_i_first) {
                            if (x != grammar->get_node("epsilon"))
                                s.insert(x);
                        }
                    }
                }
                if (!word_i->de_epsilon(set<nodep>()))
                    break;
                if (i == r.size() - 1) {
                    s.insert(grammar->get_node("epsilon"));
                }
            }
            if (r.empty())
                s.insert(grammar->get_node("epsilon"));
            re.push_back(s);
        }
        first_by_rule = re;
    }
    return first_by_rule;
}

grammar_node::~grammar_node() {}

bool grammar_node::de_epsilon(set<nodep> fathers) {
    fathers.insert(this);
    for (const auto &r:rules) {
        bool might_epsilon = true;
        for (auto n:r) {
            if (fathers.count(this) == 0 && n->de_epsilon(fathers) == false) {
                might_epsilon = false;
                break;
            }
        }
        if (might_epsilon)
            return true;
    }
    return false;
}

set<lr0_itemp> grammar_node::get_lr0_items(function<bool(lr0_itemp)> filter) {
    set<lr0_itemp> re;
    for (const auto &r:rules) {
        for (int i = 0; i <= r.size(); i++) {
            if (filter == nullptr || filter(get_lr0_item(r, i)))
                re.insert(get_lr0_item(r, i));
        }
    }
    return re;
}


lr0_itemp grammar_node::get_lr0_item(vector<nodep> rule, int pos) {
    if (all_lr0_items.count({rule, pos}) == 0) {
        auto x = new lr0_item(this, rule, pos);
        all_lr0_items[{rule, pos}] = x;
    }
    return all_lr0_items[{rule, pos}];
}

lr1_itemp grammar_node::get_lr1_item(vector<nodep> rule, int pos, nodep search_char) {
    if (all_lr1_items.count({rule, {pos, search_char}}) == 0) {
        auto x = new lr1_item(this, rule, pos, search_char);
        all_lr1_items[{rule, {pos, search_char}}] = x;
    }
    return all_lr1_items[{rule, {pos, search_char}}];
}

bool grammar_node::is_vt() {
    return rules.empty();
}

set<nodep> grammar_node::get_follow() {
    return grammar->get_follow()[this];
}

string grammar_node::rule_to_string(vector<nodep> rule) {
    string re = "";
    if (rule.empty())
        re += "epsilon ";
    for (auto n:rule) {
        re += n->name + " ";
    }
    return re;
}


nodep grammar::get_node(string s) {
    if (all_nodes.count(s) == 0) {
        all_nodes[s] = new grammar_node(s, this);
    }
    return all_nodes[s];
}

void grammar::read_rules() {
    char line[1000];
    nodep now_left = nullptr;
    bool oneof = false;
    while (cin.getline(line, 1000)) {
        vector<string> words = split_space(line);
        if (words.empty())
            continue;
        if (words[0] == "fuck")
            break;

        if (words[0].back() == ':' && words[0].length() > 1) {
//            cout<<words[0].substr(0,words[0].length()-1)<<endl;
            string left_name = words[0].substr(0, words[0].length() - 1);
            now_left = get_node(left_name);
            nodes.push_back(now_left);
            if (words.size() >= 3 && words[words.size() - 2] == "one" && words[words.size() - 1] == "of") {
                oneof = true;
            } else {
                oneof = false;
            }
        } else {
            if (oneof) {
                for (auto word:words) {
                    now_left->add_rule(vector<string>({word}));
                }
            } else {
                map<int, int> opts;
                for (int i = 0; i < words.size(); i++) {
                    auto word_i = words[i];
                    if (word_i.length() >= 3 && word_i.substr(word_i.length() - 3, 3) == "opt") {
                        opts[i] = opts.size();
                    }
                }
                for (int k = 0; k < (1 << opts.size()); k++) {
                    bitset<32> w = k;
                    vector<string> ad;
                    for (int i = 0; i < words.size(); i++) {
                        if (opts.count(i) == 0) {
                            ad.push_back(words[i]);
                        } else {
                            if (w[opts[i]] == 1) {
                                ad.push_back(words[i].substr(0, words[i].length() - 3));
                            }
                        }
                    }
//                for(auto s:ad){
//                    cout<<s<<" ";
//                }
//                cout<<endl;
                    if (ad.size())
                        now_left->add_rule(ad);
                }
            }
        }
    }
}

vector<nodep> grammar::get_all_Vn() {
    vector<nodep> re;
    for (auto x:all_nodes) {
        if (!x.second->is_vt()) {
            re.push_back(x.second);
        }
    }
    return re;
}

vector<nodep> grammar::get_all_Vt() {
    vector<nodep> re;
    for (auto x:all_nodes) {
        if (x.second->is_vt()) {
            re.push_back(x.second);
        }
    }
    return re;
}

vector<nodep> grammar::get_root() {
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
        if (!p->rules.empty()) {
            if (tex)
                printf("%s\n", p->to_text().data());
            else
                printf("%s\n", p->to_string().data());
        }
    }

}

grammar::grammar(const grammar &g) {
    for (auto n:g.nodes) {
        auto newn = get_node(n->name);
        nodes.push_back(newn);
        for (const auto &r:n->rules) {
            vector<nodep> x;
            x.reserve(r.size());
            for (auto nn:r) {
                x.push_back(get_node(nn->name));
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
            if (!r.empty() && r[0] == u) {
                leftr = true;
                break;
            }
        }
        if (leftr) {
            nodep uu = re->get_node(u->name + "'");
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
            uu->rules.push_back(vector<nodep>({re->get_node("epsilon")}));
            u->rules = unew_compose;
        }
    }
    stack<nodep> st;
    for (auto s:re->get_root()) {
        st.push(s);
    }
    set<nodep> vis;
    while (!st.empty()) {
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
    for (auto p = re->all_nodes.begin(); p != re->all_nodes.end(); p++) {
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
            all_nodes.erase(p);

        }
    }
    return re;
}

void grammar::elimiante_common_left() {

}

grammar::~grammar() {
    for (auto p:all_nodes) {
        delete p.second;
    }
}

void grammar::print_first() {
    cout << "----First:----" << endl;
    set<nodep> Vt;
    for (auto n:get_all_Vt())
        Vt.insert(n);
    for (auto p:all_nodes) {
        auto u = p.second;
        if (isvt(u))
            continue;
        auto f = u->get_first_by_rule(set<nodep>());
        cout << u->name << ":" << endl;
        for (auto x:u->get_first(set<nodep>())) {
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
    if (!follow.empty())
        return follow;

    map<nodep, set<nodep> > last;
    map<nodep, set<nodep> > re;
    re[get_root()[0]].insert(get_node("#"));
    while (last != re) {
        last = re;
        for (const auto &p:all_nodes) {
            auto u = p.second;
            if (u->is_vt())
                continue;
            for (auto r:u->rules) {
                for (int i = 0; i < (int) r.size() - 1; i++) {
                    if (r[i]->is_vt())
                        continue;
                    for (int j = i + 1; j < r.size(); j++) {
                        for (auto x:r[j]->get_first(set<nodep>())) {
                            if (x->name != "epsilon") {
                                re[r[i]].insert(x);
                            }
                        }
                        if (!r[j]->de_epsilon(set<nodep>()))
                            break;
                    }
                }
            }
            for (auto r:u->rules) {
                if (!r.empty() && !r.back()->is_vt()) {
                    re[r.back()].insert(re[u].begin(), re[u].end());
                }
                for (int i = (int) r.size() - 1; i > 0; i--) {
                    if (r[i]->de_epsilon(set<nodep>())) {
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
    return follow;
}

void grammar::print_follow() {
    cout << "----Follow:----" << endl;
    for (const auto &p:get_follow()) {
        cout << p.first->name << ": " << endl;
        for (auto x:p.second) {
            cout << "    " << x->name << endl;
        }
    }
}

map<nodep, set<nodep>> grammar::get_first() {
    if (!first.empty())
        return first;
    map<nodep, set<nodep>> re;
    for (auto x:get_all_Vn()) {
        re[x] = x->get_first(set<nodep>());
    }
    first = re;
    return re;
}

map<pair<nodep, nodep>, vector<vector<nodep>>> grammar::get_LL1form() {
    if (!LL1form.empty()) {
        return LL1form;
    }

    map<pair<nodep, nodep>, vector<vector<nodep>>> re;
    get_follow();
    get_first();

    for (const auto &p:all_nodes) {
        auto u = p.second;

        auto first_set = u->get_first_by_rule(set<nodep>());
        auto ufollow = follow[u];
        for (int i = 0; i < u->rules.size(); i++) {
            auto r = u->rules[i];
            if (first_set[i].count(get_node("epsilon"))) {
                for (auto a:ufollow) {
                    re[{u, a}].push_back(r);
                }
            } else {
                for (auto a:first_set[i]) {
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
    for (const auto &p: LL1form) {
        cout << p.first.first->name << " " << p.first.second->name << endl;
        for (const auto &r:p.second)
            cout << "    " << grammar_node::rule_to_string(r) << endl;
    }
}

lr0_closurep grammar::get_lr0_closure(set<lr0_itemp> items) {
    set<lr0_itemp> last;
    while (items != last) {
        last = items;
        for (auto x:items) {
            auto y = x->expand_closure_items();
            items.insert(y.begin(), y.end());
        }
    }
    if (!lr0_closures.count(items))
        lr0_closures[items] = new lr0_closure(items);
    return lr0_closures[items];
}

lr1_closurep grammar::get_lr1_closure(set<lr1_itemp> items) {
    set<lr1_itemp> last;
    while (items != last) {
        last = items;
        for (auto x:items) {
            auto y = x->expand_closure_items();
            items.insert(y.begin(), y.end());
        }
    }
    if (!lr1_closures.count(items))
        lr1_closures[items] = new lr1_closure(items);
    return lr1_closures[items];
}

void grammar::build_lr0_closures() {
    auto c = get_lr0_closure(get_root()[0]->get_lr0_items([](lr0_itemp p) -> bool { return p->dot_position == 0; }));
    c->expand();
    head_lr0_closure = c;
}

void grammar::build_lr1_closures() {
    auto x = get_root()[0]->get_lr1_item(get_root()[0]->rules[0], 0, get_node("#"));
    auto c = get_lr1_closure(set({x}));
    c->expand();
    head_lr1_closure = c;
}


void grammar::print_lr0_closures(bool dot) {
    cout << "----LR0 Closures----" << endl;
    if (lr0_closures.empty())
        build_lr0_closures();


    if (dot) {
        string indent = "    ";
        set<lr0_closurep> vis;
        stack<lr0_closurep> st;
        st.push(head_lr0_closure);
        vis.insert(head_lr0_closure);
        while (!st.empty()) {
            auto c = st.top();
            st.pop();
            string label = "I-";
            label += ::to_string(c->lr0_closure_cnt) + "\n";
            for (auto i:c->itemps) {
                label += i->to_string("");
            }
            cout << indent << c->lr0_closure_cnt << " [label=\"" << label << "\"]" << endl;
            for (auto g:c->go) {
                cout << indent << c->lr0_closure_cnt << "->" << g.second->lr0_closure_cnt
                     << " [label=\"" << g.first->name << "\"]" << endl;
                if (vis.count(g.second) == 0) {
                    st.push(g.second);
                    vis.insert(g.second);
                }
            }
        }
    } else {
        vector<lr0_closurep> c;
        for (const auto &p:lr0_closures) {
            c.push_back(p.second);
        }
        sort(c.begin(), c.end(),
             [](lr0_closurep a, lr0_closurep b) { return a->lr0_closure_cnt < b->lr0_closure_cnt; });

        for (auto p:c) {
            cout << p->to_string();
        }
    }
}

void grammar::print_lr1_closures(bool dot) {

    cout << "----LR1 Closures----" << endl;
    if (lr1_closures.empty())
        build_lr1_closures();

    if (dot) {
        string indent = "    ";
        set<lr1_closurep> vis;
        stack<lr1_closurep> st;
        st.push(head_lr1_closure);
        vis.insert(head_lr1_closure);
        while (!st.empty()) {
            auto c = st.top();
            st.pop();
            string label = "I-";
            label += ::to_string(c->lr1_closure_cnt) + "\n";
            for (auto i:c->itemps) {
                label += i->to_string("");
            }
            cout << indent << c->lr1_closure_cnt << " [label=\"" << label << "\"]" << endl;
            for (auto g:c->go) {
                cout << indent << c->lr1_closure_cnt << "->" << g.second->lr1_closure_cnt
                     << " [label=\"" << g.first->name << "\"]" << endl;
                if (vis.count(g.second) == 0) {
                    st.push(g.second);
                    vis.insert(g.second);
                }
            }
        }
    } else {
        vector<lr1_closurep> c;
        for (const auto &p:lr1_closures) {
            c.push_back(p.second);
        }
        sort(c.begin(), c.end(),
             [](lr1_closurep a, lr1_closurep b) { return a->lr1_closure_cnt < b->lr1_closure_cnt; });
        for (auto p:c) {
            cout << p->to_string();
        }
    }
}

map<pair<lr0_closurep, nodep>, vector<string>> grammar::get_SLR1form() {
    if (!SLR1form.empty())
        return SLR1form;
    decltype(SLR1form) re;
    get_follow();
    if (lr0_closures.empty())
        build_lr0_closures();
    for (const auto &c:lr0_closures) {
        auto cp = c.second;
        for (auto g:cp->go) {
            if (g.first->is_vt()) {
                re[{cp, g.first}].push_back("shift " + to_string(g.second->lr0_closure_cnt));
            } else {
                re[{cp, g.first}].push_back("goto " + to_string(g.second->lr0_closure_cnt));
            }
        }
        for (auto i:cp->itemps) {
            if (i->get_kind() == lr0_item::RDC) {
                for (auto fn:follow[i->left]) {
                    re[{cp, fn}].push_back("reduce " + i->left->name + " -> " + grammar_node::rule_to_string(i->rule));
                }
            }
            if (i->get_kind() == lr0_item::ACC) {
                for (auto fn:follow[i->left]) {
                    re[{cp, fn}].push_back("acc");
                }
            }
        }
    }
    SLR1form = re;
    return re;
}

map<pair<lr1_closurep, nodep>, vector<string>> grammar::get_LR1form() {
    if (!LR1form.empty())
        return LR1form;
    decltype(LR1form) re;
    get_follow();
    if (lr1_closures.empty()) {
        build_lr1_closures();
    }
    for (const auto &c:lr1_closures) {
        auto cp = c.second;
        for (auto g:cp->go) {
            if (g.first->is_vt()) {
                re[{cp, g.first}].push_back("shift " + to_string(g.second->lr1_closure_cnt));
            } else {
                re[{cp, g.first}].push_back("goto " + to_string(g.second->lr1_closure_cnt));
            }
        }
        for (auto i:cp->itemps) {
            if (i->get_kind() == lr0_item::RDC) {
                re[{cp, i->search_char}].push_back(
                        "reduce " + i->left->name + " -> " + grammar_node::rule_to_string(i->rule));
            }
            if (i->get_kind() == lr0_item::ACC) {
                for (auto fn:follow[i->left]) {
                    re[{cp, fn}].push_back("acc");
                }
            }
        }
    }
    LR1form = re;
    return re;
}


void grammar::print_SLR1form() {
    cout << "----SLR1 form----" << endl;
    get_SLR1form();
    for (const auto &p:SLR1form) {
        for (const auto &x:p.second) {
            cout << p.first.first->lr0_closure_cnt << " " << p.first.second->name << " : ";
            cout << " " << x << endl;
        }
    }
}

void grammar::print_LR1form() {
    cout << "----LR1 form----" << endl;
    get_LR1form();
    for (const auto &p:LR1form) {
        for (const auto &x:p.second) {
            cout << p.first.first->lr1_closure_cnt << " " << p.first.second->name << " : ";
            cout << " " << x << endl;
        }
    }
}


lr0_item::lr0_item(nodep left, vector<nodep> rule, int dot_position)
        : left(left), rule(std::move(rule)), dot_position(dot_position) {}

set<lr0_itemp> lr0_item::expand_closure_items() {
    if (!closure_items.empty())
        return closure_items;
    grammerp g = left->grammar;
    set<lr0_itemp> re;
    set<lr0_itemp> last;
    re.insert(this);
    while (last != re) {
        last = re;
        for (auto p:re) {
            if (p->dot_position < p->rule.size()) {
                nodep nxt = p->rule[p->dot_position];
                if (!g->isvt(nxt)) {
                    auto s = nxt->get_lr0_items([](lr0_itemp x) { return x->dot_position == 0; });
                    re.insert(s.begin(), s.end());
                }
            }
        }
    }
    return re;
}

string lr0_item::to_string(string prefix) {
    prefix += left->name + " ->";
    for (int i = 0; i < rule.size(); i++) {
        if (i == dot_position)
            prefix += " ·";
        prefix += " " + rule[i]->name;
    }
    if (dot_position == rule.size())
        prefix += " ·";

//    prefix += "  " + get_kind_str();

    prefix += "\n";
    return prefix;
}

lr0_item::kind lr0_item::get_kind() {
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

std::string lr0_item::get_kind_str() {
    return std::map<kind, std::string>({
                                               {ACC, "accept"},
                                               {RDC, "reduce"},
                                               {SFT, "shift"},
                                               {TRD, "to be reduce"}
                                       })[get_kind()];
}


lr0_closure::lr0_closure(set<lr0_itemp> s) : itemps(std::move(s)) {
    g = (*itemps.begin())->left->grammar;
    lr0_closure_cnt = ++g->lr0_closure_cnt;
    cerr << "creating lr0 closure " << lr0_closure_cnt << endl;
}

void lr0_closure::expand() {
    cerr << "expanding lr0 closure " << lr0_closure_cnt << endl;
    if (itemps.empty())
        return;
    expanded = true;
    grammerp g = (*itemps.begin())->left->grammar;

    map<nodep, set<lr0_itemp> > nxt;
    for (auto i:itemps) {
        if (i->dot_position < i->rule.size()) {
            auto nxt_node = i->rule[i->dot_position];
            nxt[nxt_node].insert(i->left->get_lr0_item(i->rule, i->dot_position + 1));
        }
    }
    for (const auto &p:nxt) {
        go[p.first] = g->get_lr0_closure(p.second);
    }
    for (const auto &p:go) {
        if (p.second->expanded == false)
            p.second->expand();
    }
}

string lr0_closure::to_string() {
    string re = "Closure " + ::to_string(lr0_closure_cnt) + ": \n";

    for (auto i:itemps) {
        re += i->to_string("    ");
    }
    re += "\n";
    for (auto p:go) {
        re += "through ";
        re += p.first->name;
        re += " to Closure ";
        re += ::to_string(p.second->lr0_closure_cnt);
        re += "\n";
    }
    re += "\n";
    return re;
}

lr1_item::lr1_item(nodep left1, vector<nodep> rule1, int dotPosition, nodep search)
        : lr0_item(left1, rule1, dotPosition), search_char(search) {}

string lr1_item::to_string(string prefix) {
    prefix += left->name + " ->";
    for (int i = 0; i < rule.size(); i++) {
        if (i == dot_position)
            prefix += " ·";
        prefix += " " + rule[i]->name;
    }
    if (dot_position == rule.size())
        prefix += " ·";

    prefix += "  with " + search_char->name;

//    prefix += "  " + get_kind_str();

    prefix += "\n";
    return prefix;
}

set<lr1_itemp> lr1_item::expand_closure_items() {
    if (!closure_items.empty())
        return closure_items;
    grammerp g = left->grammar;
    set<lr1_itemp> re;
    set<lr1_itemp> last;
    re.insert(this);
    while (last != re) {
        last = re;
        for (auto p:re) {
            if (p->dot_position < p->rule.size()) {
                nodep nxt = p->rule[p->dot_position];
                if (!nxt->is_vt()) {
                    set<nodep> search_chars;
                    for (int i = p->dot_position + 1; i < p->rule.size(); i++) {
                        auto ri = p->rule[i];
                        auto first_of_ri = ri->get_first(set<nodep>());
                        search_chars.insert(first_of_ri.begin(), first_of_ri.end());
                        if (!ri->de_epsilon(set<nodep>())) {
                            break;
                        }
                        if (i == (int) p->rule.size() - 1) {
                            search_chars.insert(g->get_node("#"));
                        }
                    }
                    if (search_chars.empty()) {
                        search_chars.insert(g->get_node("#"));
                    }

//                    for (auto x:search_chars) {
//                        cout << x->name << " ";
//                    }
//                    cout << endl;

                    for (const auto &r:nxt->rules) {
//                        cout<<nxt->rule_to_string(r)<<endl;
                        for (auto s:search_chars) {
//                            cout << nxt->get_lr1_item(r, 0, s)->to_string() << endl;
                            re.insert(nxt->get_lr1_item(r, 0, s));
                        }
                    }
                }
            }
        }
    }
    return re;
}

lr1_closure::lr1_closure(set<lr1_itemp> s) : itemps(s) {
    g = (*s.begin())->left->grammar;
    lr1_closure_cnt = ++g->lr1_closure_cnt;
    cerr << "creating lr1 closure " << lr1_closure_cnt << endl;
}

string lr1_closure::to_string() {
    string re = "Closure " + ::to_string(lr1_closure_cnt) + ": \n";

    for (auto i:itemps) {
        re += i->to_string("    ");
    }
    re += "\n";
    for (auto p:go) {
        re += "through ";
        re += p.first->name;
        re += " to Closure ";
        re += ::to_string(p.second->lr1_closure_cnt);
        re += "\n";
    }
    re += "\n";
    return re;
}

void lr1_closure::expand() {
    cerr << "expanding lr1 closure " << lr1_closure_cnt << endl;
    if (itemps.empty())
        return;
    expanded = true;
    grammerp g = (*itemps.begin())->left->grammar;

    map<nodep, set<lr1_itemp> > nxt;
    for (auto i:itemps) {
        if (i->dot_position < i->rule.size()) {
            auto nxt_node = i->rule[i->dot_position];
            nxt[nxt_node].insert(i->left->get_lr1_item(i->rule, i->dot_position + 1, i->search_char));
        }
    }
    for (const auto &p:nxt) {
        go[p.first] = g->get_lr1_closure(p.second);
    }
    for (const auto &p:go) {
        if (p.second->expanded == false)
            p.second->expand();
    }
}