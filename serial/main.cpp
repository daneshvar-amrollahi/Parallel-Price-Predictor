#include <iostream>
#include <fstream>
#include <vector>
#include <math.h>
#include <iomanip> 
#include <time.h>

using namespace std;


#define FILENAME "dataset.csv"
#define CLASSIFIER_COL 5
#define PRICE_COL 8
#define COMMA ','
#define EMPTY_STR ""

struct Item
{
    int row;
    int x;
    bool category;
};

vector<Item> items;
double mean_1, std_1;
double accuracy;
int expensive_cnt;
int threshold;

vector<int> separateByComma(string s)
{
    vector<int> res;
    string cur = EMPTY_STR;
    for (int i = 0 ; i < s.size() ; i++)
        if (s[i] == COMMA)
        {
            res.push_back(stoi(cur));
            cur = EMPTY_STR;
        }
        else
        cur += s[i];

    res.push_back(stoi(cur));
    return res;
}


void readInput()
{
    string line;   
    ifstream fin(FILENAME);

    int row = 0;
    while (fin >> line)
    {
        if (row == 0) 
        {
            row++;
            continue;
        }
        vector<int> cur = separateByComma(line);

        bool category = (cur[PRICE_COL] >= threshold);

        Item item{row, cur[CLASSIFIER_COL], category};
        expensive_cnt += category;

        items.push_back(item);
        row++;
    }
    fin.close();
}


void calcMean()
{
    int n = items.size();
    for (int i = 0 ; i < n ; i++)
        if (items[i].category)
            mean_1 += (1 / (double)(expensive_cnt)) * (items[i].x);
}

void calcSTD()
{
    int n = items.size();
    for (int i = 0 ; i < n ; i++)
        if (items[i].category)
            std_1 += (1 / (double)(expensive_cnt)) * (items[i].x - mean_1) * (items[i].x - mean_1);
        
    std_1 = sqrt(std_1);
}

bool predictSingle(Item& item)
{
    return (mean_1 - std_1 <= item.x && item.x <= mean_1 + std_1);
}

void predictPriceCategories()
{
    srand (time(NULL));
    int correct = 0;
    for (int i = 0 ; i < items.size() ; i++)
        correct += (predictSingle(items[i]) == items[i].category);

    accuracy = (double)(correct) / (double)(items.size()); 
}

int main(int argc, char *argv[])
{
    threshold = atoi(argv[1]);
    clock_t start, end;
    start = clock();
    readInput();                
    
    calcMean();                 
    calcSTD();                  

    predictPriceCategories();   

    end = clock();


    double time_taken = double(end - start) / double(CLOCKS_PER_SEC);    
    cout << "Accuracy: " << fixed << setprecision(2) << accuracy * 100 << "%" << endl;
    //printf("Time Elapsed: %f\n", time_taken);
    return 0;
}