#include <iostream>
#include <fstream>
using namespace std;

int main()
{
    fstream outer;
    outer.open("bomb", ios::binary | ios::out | ios::in);
    char spe = 0xe;
    /* 使用前，在反汇编代码中确认下面尖括号中函数的地址（第一行代码地址），填入其中后运行即可 */
    outer.seekp(0x0000000000001d78 + 0x9D, ios::beg);
    outer << spe;
    spe = 0xc3;
    outer.seekp(0x0000000000001f20 + 0x4, ios::beg);
    outer << spe;
    outer.seekp(0x00000000000022a0 + 0x4, ios::beg);
    outer << spe;
    outer.close();
}