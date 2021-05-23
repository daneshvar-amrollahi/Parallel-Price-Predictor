#include <iostream>
#include <fstream>
#include <vector>
#include <math.h>

using namespace std;


#define FILENAME "dataset.csv"
#define CLASSIFIER_COL 5
#define PRICE_COL 8

struct Item
{
    int row;
    int x;
    bool category;
};

vector<int> separateByComma(string s)
{
    vector<int> res;
    string cur = "";
    for (int i = 0 ; i < s.size() ; i++)
        if (s[i] == ',')
        {
            res.push_back(stoi(cur));
            cur = "";
        }
        else
        cur += s[i];

    res.push_back(stoi(cur));
    return res;
}

vector<string> readCSV()
{
    string line;
    vector<string> res;
    
    ifstream fin(FILENAME);
    while (getline(fin, line))
        res.push_back(line);
    fin.close();

    return res;
}

Item getItem(int row, int x, bool category)
{
    Item item;
    item.row = row;
    item.x = x;
    item.category = category;
    return item;
}

vector<Item> extractItems(vector<string> lines, int threshold)
{
    vector<Item> res;
    for (int row = 1 ; row < 10; row++)
    {
        vector<int> line = separateByComma(lines[row]);

        for (auto x: line)
            cout << x << " ";
        cout << endl;

        
        Item item = getItem(row, line[CLASSIFIER_COL], line[PRICE_COL] >= threshold);
        res.push_back(item);
    }
    return res;
}



void findMeanStd(const vector<Item>& items, double& mean_0, double& mean_1, double& std_0, double& std_1)
{
    int expensive_cnt = 0;
    for (auto item: items)
        expensive_cnt += (item.category == 1);

    int n = items.size();
    for (auto item: items)
    {
        if (item.category)
            mean_1 += (1 / (double)(expensive_cnt)) * (item.x);
        else
            mean_0 += (1 / (double)(n - expensive_cnt)) * (item.x);
    }

    for (auto item: items)
    {
        if (item.category)
            std_1 += (1 / (double)(expensive_cnt)) * (item.x - mean_1) * (item.x - mean_1);
        else
            std_0 += (1 / (double)(expensive_cnt)) * (item.x - mean_0) * (item.x - mean_0);
    }

    std_0 = sqrt(std_0);
    std_1 = sqrt(std_1);
}

bool predictPriceCategory(Item& item, const double& mean_0, const double& mean_1, const double& std_0, const double& std_1)
{
    if (mean_0 - std_0 <= item.x && item.x <= mean_0 + std_0)
        return 0;
    if (mean_1 - std_1 <= item.x && item.x <= mean_1 + std_1)
        return 1;
    return rand() % 2;
}

double predictPriceCategories(const vector<Item>& items, const double& mean_0, const double& mean_1, const double& std_0, const double& std_1)
{
    srand (time(NULL));
    int correct = 0;
    for (auto item: items)
        correct += (predictPriceCategory(item, mean_0, mean_1, std_0, std_1) == item.category);

    return (double)(correct) / (double)(items.size());
}

int main(int argc, char *argv[])
{
    int threshold = atoi(argv[1]);
    vector<string> lines = readCSV();
    vector<Item> items = extractItems(lines, threshold);

    double mean_0, mean_1, std_0, std_1;

    findMeanStd(items, mean_0, mean_1, std_0, std_1);

    double accuracy = predictPriceCategories(items, mean_0, mean_1, std_0, std_1);

    cout << "(" << mean_0 - std_0 << " , " << mean_0 + std_0 << ")" << endl;
    cout << "(" << mean_1 - std_1 << " , " << mean_1 + std_1 << ")" << endl;
    cout << accuracy << endl;
    return 0;
}