#include <iostream>
#include <vector>
using namespace std;
string removeTrailingZeros(string num)
{
    return num.substr(0, num.find_last_not_of('0') + 1);
}
int main()
{
    std::cout << removeTrailingZeros("1230100");
    return 0;
}