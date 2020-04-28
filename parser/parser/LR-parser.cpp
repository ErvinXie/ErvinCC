//
// Created by 谢威宇 on 2020/4/13.
//

#include "LR-parser.h"
#include <fstream>
#include <iostream>
#include <regex>
#include "../../utils/utils.h"

const int error = -1;
const int acc = 0;
const int shift = 1;
const int reduce = 2;

struct x {
    int x:10;
};


void LR::read_lr1_form(string rule_dir) {
    ifstream rules;
    rules.open(rule_dir);
    char in[1000];
    while (rules.getline(in, 1000)) {
//        cout<<in<<endl;
        auto input = split_space(in);
        if (input.size() < 2)
            continue;
        int state = atoi(input[0].data());

        string word = input[1];
        if (input[3] == "shift") {
            auto nxt_state = atoi(input[4].data());
            action[{state, word}] = [nxt_state, this, word](cnp nd) -> int {
                state_st.push_back(nxt_state);
                node_st.push_back(nd);
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
                action[{state, word}] = [this, r, reduced](cnp nd) -> int {
                    auto rn = new cst_node(reduced, std::string());

                    for (int i = 0; i < r; i++) {
                        rn->sons.push_back(node_st.back());
                        this->node_st.pop_back();
                        this->state_st.pop_back();
                    }
                    reverse(rn->sons.begin(), rn->sons.end());
                    //todo: add sons
                    node_st.push_back(rn);
                    if (this->go_to.count({state_st.back(), reduced}) == 0) {
                        cout << "goto: " << state_st.back() << " " << reduced << " not found" << endl;

                        return error;
                    }
                    state_st.push_back(this->go_to[{state_st.back(), reduced}]);
                    return reduce;
                };
            }
        } else if (input[3] == "acc") {
            action[{state, word}] = [](cnp x) -> int {
                return 0;
            };
        } else if (input[3] == "goto") {
            go_to[{state, word}] = atoi(input[4].data());
        } else {
            cerr << "Wrong Rules! : " << in << endl;
        }
    }

    rules.close();
}

int LR::parse(vector<string> type, vector<string> token) {
    type.push_back("#");
    token.push_back("#");
    state_st.clear();
    node_st.clear();
    state_st.push_back(1);
    node_st.push_back(new cst_node("#", ""));
//    debug();
    int matching = 1;
    int now = 0;
    while (matching > acc && now < type.size()) {
        auto word = type[now];

//        cout << now << ": " << word << endl;
        if (action.count({state_st.back(), word}) == 0) {
            cout << "Error at token " << now << " action: " << state_st.back() << " " << word << " not found" << endl;
            matching = error;
        } else {

            matching = action[{state_st.back(), word}](new cst_node(word, token[now]));
            if (matching == shift) {
                now++;
            }
        }
//        debug();

    }
    return matching;
}

void LR::debug() {
    cout << "state stack: ";
    for (auto i:state_st) {
        cout << i << " ";
    }
    cout << endl;

    cout << "word stack: ";
    for (const auto &i:node_st) {
        cout << i->type << " ";
    }
    cout << endl;
}

int main(int argc, char *argv[]) {
//  -r rule files
//  -p turn on image output

    string lr_form = "./lr-form.txt";
    string lex_file;
    string dot_file;
    string json_file;
    string png_file;
    bool ispng = false;
    for (int i = 0; i < argc; i++) {
        if (string(argv[i]) == "-r") {
            lr_form = argv[i + 1];
            i++;
        } else if (string(argv[i]) == "-p") {
            ispng = true;
        } else {
            lex_file = argv[i];
        }
    }
    if (lex_file.empty()) {
        cerr << "No Lexical File" << endl;
        return 1;
    }
    for (int i = (int) lex_file.length() - 1; i >= 0; i--) {
        if (lex_file[i] == '.') {
            dot_file = lex_file.substr(0, i) + ".dot";
            json_file = lex_file.substr(0, i) + ".json";
            png_file = lex_file.substr(0, i) + ".png";
            break;
        }
    }
    cout << lex_file << endl;
    cout << dot_file << endl;
    cout << json_file << endl;
    cout << png_file << endl;

    auto *a = new LR();
    ofstream ast(json_file);
    ifstream lexer_in(lex_file);
    a->read_lr1_form(lr_form);
    char input[1000];
    regex r(R"(\[@\d+,\d+:\d+='(.+)',<(.+)>,\d+:\d+\])");
    vector<string> types, tokens;
    while (lexer_in.getline(input, 1000)) {
        string input1 = input;
        std::sregex_iterator iter(input1.begin(), input1.end(), r);
        std::sregex_iterator end;
        string type, value;
        value = (*iter)[1];
        type = (*iter)[2];
        if (type == "EOF")
            break;
        types.push_back(type);
        tokens.push_back(value);
//        cout << type << " " << value << endl;
//        cout << a->parse(split_space(input), split_space(input)) << endl;
//        a->node_st.back()->eliminate();
//        ast << a->node_st.back()->to_string("", true) << endl;
    }
    if (a->parse(types, tokens) == 0) {
        cout << "Parse OK" << endl;
        a->node_st.back()->to_ast();
        if (ispng) {
            a->node_st.back()->to_dot(dot_file);
            system(("dot -Tpng " + dot_file + " -o " + png_file).data());
        }
        ast << a->node_st.back()->to_string(true) << endl;
    }
    ast.close();
    return 0;
}
