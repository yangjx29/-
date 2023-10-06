// #include <algorithm>
// #include <iostream>
// #include <string>
// #include <vector>

// using namespace std;

// int n;

// class Record {
// public:
//     string m_country;
//     int m_Gi;
//     int m_Si;
//     int m_Bi;
//     Record(string mCountry, int mGi, int msi, int mBi) : m_country(mCountry), m_Gi(mGi), m_Si(msi), m_Bi(mBi) {}
// };
// bool myCompare(Record& record1, Record& record2) {
//     // 先按照金牌排序
//     if (record1.m_Gi != record2.m_Gi) {
//         return record1.m_Gi > record2.m_Gi;
//     } else if (record1.m_Si != record2.m_Si) {
//         return record1.m_Si > record2.m_Si;
//     } else if (record1.m_Bi != record2.m_Bi) {
//         return record1.m_Bi > record2.m_Bi;
//     } else {
//         string country1 = record1.m_country;
//         string country2 = record2.m_country;
//         transform(country1.begin(), country1.end(), country1.end(), ::tolower);
//         // transform(country2.begin(), country2.end(), country2.end(), ::tolower);
//         transform(country2.begin(), country2.end(), country2.end(), [](char c) {
//             return tolower(c);
//         });
//         return country1.compare(country2);
//     }
// }

// void myPrint(Record& record) {
//     cout << record.m_country << endl;
// }

// int main() {
//     cin >> n;
//     vector<Record> v;
//     v.reserve(n);
//     for (int i = 0; i < n; i++) {
//         string country;
//         int gold, silver, copper;
//         cin >> country >> gold >> silver >> copper;
//         Record record(country, gold, silver, copper);
//         v.push_back(record);
//     }
//     sort(v.begin(), v.end(), myCompare);
//     for (auto vec : v) {
//         myPrint(vec);
//     }
//     return 0;
// }

#include <algorithm>
#include <iostream>
#include <vector>

using namespace std;
int n;

class Record {
public:
    string m_country;
    int m_gold;
    int m_sliver;
    int m_bronze;
    Record(string country, int gold, int sliver, int bronze) : m_country(country), m_gold(gold), m_sliver(sliver), m_bronze(bronze) {}
};

int main() {
    cin >> n;
    vector<Record> v;
    v.reserve(n);
    for (int i = 0; i < n; i++) {
        string country;
        int gold, sliver, bronze;
        cin >> country >> gold >> sliver >> bronze;
        Record record(country, gold, sliver, bronze);
        v.push_back(record);
    }
    sort(v.begin(), v.end(), [](auto r1, auto r2) -> bool {
        if (r1.m_gold != r2.m_gold) {
            return r1.m_gold > r2.m_gold;
        } else if (r1.m_sliver != r2.m_sliver) {
            return r1.m_sliver > r2.m_sliver;
        } else if (r1.m_bronze != r2.m_bronze) {
            return r1.m_bronze > r2.m_bronze;
        } else {
            string c1 = r1.m_country;
            string c2 = r2.m_country;
            transform(c1.begin(), c1.end(), c1.end(), ::tolower);
            transform(c2.begin(), c2.end(), c2.end(), ::tolower);
            return c1.compare(c2);
        }
    });
    for (auto vec : v) {
        cout << vec.m_country << endl;
    }
    return 0;
}