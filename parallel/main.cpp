#include <iostream>
#include <fstream>
#include <vector>
#include <math.h>
#include <iomanip> 
#include <pthread.h>
#include <stdio.h>
#include <time.h>
#include <sys/stat.h>
#include <unistd.h>

using namespace std;

#define COMMA ','
#define EMPTY_STR ""
#define FILENAME "dataset.csv"
#define CLASSIFIER "GrLivArea"
#define SALE_PRICE "SalePrice"

const int MAX_THREAD_NUMBERS = 20; 

int NUMBER_OF_THREADS;
int threshold;
int expensive_cnt[MAX_THREAD_NUMBERS];
vector<string> lines;
string head;
double _std;
long sum[MAX_THREAD_NUMBERS];
long ps[MAX_THREAD_NUMBERS];
long sumsq[MAX_THREAD_NUMBERS];
double mean;
int correct[MAX_THREAD_NUMBERS];
int total_items;
int total_expensive_cnt;
int total_corrects;

struct Item
{
    int x;
    bool category;
};

vector<Item> items[MAX_THREAD_NUMBERS];

int getColNum(const string& head, const string& key)
{
    int cnt = 0;
    string cur = EMPTY_STR;
    for (int i = 0 ; i < head.size() ; i++)
    {
        if (head[i] == COMMA)
        {
            if (cur == key)
                return cnt;
            cnt++;
            cur = EMPTY_STR;
        }   
        else
        cur += head[i];
    }
    if (cur == key)
        return cnt;
    return -1;
}

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

void* calcSums(void* tid)
{
    long thread_id = (long)tid;

    string filename = "dataset_" + to_string(thread_id) + ".csv"; 
    ifstream fin(filename);

    string head;
    fin >> head;
    int classifierColNum = getColNum(head, CLASSIFIER);
    if (classifierColNum == -1)
    {
        printf("NO GrLivArea FOUND IN HEAD OF CSV\n");
        exit(-1);
    }

    int priceColNum = getColNum(head, SALE_PRICE);
    if (priceColNum == -1)
    {
        printf("NO SalePrice FOUND IN HEAD OF CSV\n");
        exit(-1);
    }


    string line;
    while (fin >> line)
    {
        vector<int> cur = separateByComma(line);

        bool category = (cur[priceColNum] >= threshold);

        Item item{cur[classifierColNum], category};

        if (category)
        {
            sum[thread_id] += item.x;
            sumsq[thread_id] += (item.x * item.x);
            expensive_cnt[thread_id]++;
        }

        items[thread_id].push_back(item);
    }


    fin.close();
    pthread_exit(NULL);
}


void calcMeanSTD()
{
    string line;
    for (int i = 0 ; ; i++)
    {
        struct stat buffer;   
        string name = "dataset_" + to_string(i) + ".csv"; 
        if (!(stat (name.c_str(), &buffer) == 0))
            break;
        
        NUMBER_OF_THREADS++;
    }
  

    pthread_t threads[NUMBER_OF_THREADS];
    int return_code;
    for (long tid = 0 ; tid < NUMBER_OF_THREADS ; tid++)
    {
        return_code = pthread_create(&threads[tid], NULL, calcSums, (void*)tid);

		if (return_code)
		{
            printf("ERROR; return code from pthread_create() is %d\n", return_code);
			exit(-1);
		}
    }

    for (long tid = 0 ; tid < NUMBER_OF_THREADS ; tid++)
    {
        return_code = pthread_join(threads[tid], NULL);
		if (return_code)
		{
			printf("ERROR; return code from pthread_join() is %d\n", return_code);
			exit(-1);
		}
    }

    double total_sum = 0;
    double total_sum_sq = 0;
    total_expensive_cnt = 0;
    total_items = 0;
    for (int i = 0 ; i < NUMBER_OF_THREADS ; i++)
    {
        total_sum += sum[i];
        total_sum_sq += sumsq[i];

        total_expensive_cnt += expensive_cnt[i];
        total_items += items[i].size();
    }

    //printf("%f %d\n", total_sum, total_expensive_cnt);
    mean = total_sum / total_expensive_cnt;
    _std = sqrt((total_sum_sq - ((total_sum * total_sum) / (total_expensive_cnt))) / (total_expensive_cnt));

}

bool predictSingle(Item& item)
{
    return (mean - _std <= item.x && item.x <= mean + _std);
}

void* calcCorrects(void* tid)
{
    long thread_id = (long)tid; 

    int n = items[thread_id].size();
    for (int i = 0 ; i < n ; i++)
        correct[thread_id] += (predictSingle(items[thread_id][i]) == items[thread_id][i].category);

    pthread_exit(NULL);
}

void predictPriceCategories()
{
    pthread_t threads[NUMBER_OF_THREADS];
    int return_code;
    for (long tid = 0 ; tid < NUMBER_OF_THREADS ; tid++)
    {
        return_code = pthread_create(&threads[tid], NULL, calcCorrects, (void*)tid);

		if (return_code)
		{
            printf("ERROR; return code from pthread_create() is %d\n", return_code);
			exit(-1);
		}
    }

    for (long tid = 0 ; tid < NUMBER_OF_THREADS ; tid++)
    {
        return_code = pthread_join(threads[tid], NULL);
		if (return_code)
		{
			printf("ERROR; return code from pthread_join() is %d\n", return_code);
			exit(-1);
		}
    }

    total_corrects = 0;
    for (int i = 0 ; i < NUMBER_OF_THREADS ; i++)
        total_corrects += correct[i];

} 

double getAccuracy()
{
    double res = 0;
    return (((double)(total_corrects)) / ((double)(total_items)));
}


int main(int argc, char *argv[])
{
    threshold = atoi(argv[1]);          

    calcMeanSTD();                

    predictPriceCategories();  

    double accuracy = getAccuracy();    

    cout << "Accuracy: " << fixed << setprecision(2) << accuracy * 100 << "%" << endl;
    return 0;
}