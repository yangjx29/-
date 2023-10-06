#include <algorithm>
#include <iostream>
#include <vector>
using namespace std;

class Solution {
public:
    bool isSquare(vector<int>& sq) {
        if (sq.size() < 4) {
            return false;
        }
        int sum = 0;
        for (auto v : sq) {
            sum += v;
        }
        if (sum % 4 != 0) {
            return false;
        }
        int side_len = sum / 4;
        sort(sq.begin(), sq.end());
        if (sq[sq.size() - 1] > side_len) {
            return false;
        }
        int l = 0, r = sq.size() - 1;
        int cnt = 4;
        int tmp = sq[r];
        while (l <= r) {
            if (tmp < side_len) {
                tmp += sq[l];
                l++;
            } else if (tmp == side_len) {
                cnt--;
                r--;
                tmp = sq[r];
            } else {
                return false;
            }
        }
        return cnt == 0;
    }
};

int main() {
    vector<int> v = {1,1,2,2,2};
    Solution s;
    cout << s.isSquare(v) << endl;
    return 0;
}