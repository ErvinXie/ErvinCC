#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <algorithm>
#include "utils.h"
using namespace std;

vector<pair<pair<int, string>, string>> x;

int main() {
    ifstream my;
    my.open("../testfiles/lr1_form.txt");
    char t[1000];
    while (my.getline(t,1000)) {
        auto xs = split_space(t);
        x.push_back({{atoi(xs[0].data()),xs[1]},t});
    }
    sort(x.begin(),x.end());
    ofstream out;
    out.open("../testfiles/lr1_form2.txt");
    auto last = x.begin()->first;
    for(const auto& c:x){
        if(c.first==last){
            cout<<c.first.first<<" "<<c.first.second<<endl;
        }
        out<<c.second<<endl;
        last = c.first;
    }

    return 0;
}
