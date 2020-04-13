//
// Created by 谢威宇 on 2020/4/13.
//

#include "LRAnalyzer.h"
#include <fstream>
#include <iostream>
#include "utils.h"

const int error = -1;
const int acc = 0;
const int shift = 1;
const int reduce = 2;

struct x {
    int x:10;
};


void LRAnalyzer::read_lr1_form(string rule_dir) {
    ifstream rules;
    rules.open(rule_dir);
    char in[1000];
    int laststate;
    string lastword;
    while (rules.getline(in, 1000)) {
        auto input = split_space(in);
        if (input.size() < 2)
            continue;
        int state = atoi(input[0].data());
        string word = input[1];
        if (input[3] == "shift") {
            auto nxt_state = atoi(input[4].data());
            action[{state, word}] = [nxt_state, this, word]() -> int {
                state_st.push_back(nxt_state);
                word_st.push_back(word);
                return shift;
            };
        } else if (input[3] == "reduce") {
            int r = 0;
            for (int i = 6; i < input.size(); i++) {
                if (input[i] != "epsilon") {
                    r++;
                }
            }
            string reduced = input[4];
            if (action.count({state, word}) == 0) {
                action[{state, word}] = [this, r, reduced]() -> int {
                    for (int i = 0; i < r; i++) {
                        this->word_st.pop_back();
                        this->state_st.pop_back();
                    }
                    word_st.push_back(reduced);
                    if (this->go_to.count({state_st.back(), reduced}) == 0) {
                        cout << "goto: " << state_st.back() << " " << reduced << " not found" << endl;

                        return error;
                    }
                    state_st.push_back(this->go_to[{state_st.back(), reduced}]);
                    return reduce;
                };
            }
        } else if (input[3] == "acc") {
            action[{state, word}] = []() -> int {
                return 0;
            };
        } else if (input[3] == "goto") {
            go_to[{state, word}] = atoi(input[4].data());
        } else {
            cerr << "Wrong Rules!" << endl;
        }

        laststate = state;
        lastword = word;
    }

    rules.close();
}

int LRAnalyzer::parse(vector<string> input) {
    input.push_back("#");
    state_st.clear();
    word_st.clear();
    state_st.push_back(1);
    word_st.emplace_back("#");
    debug();
    int mathing = 1;
    int now = 0;
    while (mathing > acc && now < input.size()) {
        auto word = input[now];

        cout<<now<<": "<<word<<endl;
        if (action.count({state_st.back(), word}) == 0) {
            cout << "action: " << state_st.back() << " " << word << " not found" << endl;
            mathing = error;
        } else {
            mathing = action[{state_st.back(), word}]();
            if (mathing == shift) {
                now++;
            }
        }
        debug();

    }
    return mathing;
}

void LRAnalyzer::debug() {
    cout << "state stack: ";
    for (auto i:state_st) {
        cout << i << " ";
    }
    cout << endl;

    cout << "word stack: ";
    for (const auto &i:word_st) {
        cout << i << " ";
    }
    cout << endl;
}

int main() {
    LRAnalyzer *a = new LRAnalyzer();
    a->read_lr1_form("../testfiles/slr1_form.txt");
    char input[1000];
    while (cin.getline(input, 1000)) {
        cout << a->parse(split_space(input)) << endl;
    }
    return 0;
}