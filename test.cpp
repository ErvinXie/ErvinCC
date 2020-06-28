//
// Created by 谢威宇 on 2020/6/28.
//
#include <iostream>
#define CID 7111701
#define SID 1120172150
using namespace std;
int main() {
    int k = 2;
    int answer = SID;
    if ((SID - CID) % 10 > 5) {
        cout<<"!"<<endl;
        for (; k < 100; k++) {
            answer-=k*2;
        }
    } else {
        answer = CID;
    }
    std::cout<<answer<<std::endl;
    return 0;
}