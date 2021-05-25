#include <iostream>
#include <fstream>
#include <vector>
#include <math.h>
#include <iomanip> 
#include <pthread.h>
#include <stdio.h>

using namespace std;

#define COMMA ','
#define EMPTY_STR ""
#define NUM
#define FILENAME "dataset.csv"
#define CLASSIFIER "GrLivArea"
#define SALE_PRICE "SalePrice"


int NUMBER_OF_THREADS;
int threshold;
int expensive_cnt;
vector<string> lines;
string head;
double mean_1, std_1;
int correct;
pthread_mutex_t mutex_expensive_cnt, mutex_mean_1, mutex_std_1, mutex_correct;



struct Item
{
    int x;
    bool category;
};

const int MAX_THREAD_NUMBERS = 1e5 + 10; //obviously no!
vector<Item> items[MAX_THREAD_NUMBERS];

int getColNum(string head, string key)
{
    int cnt = 0;
    string cur = "";
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

void* getItems(void* tid)
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

    
    printf("This is thread %ld, claasifierColNum: %d  priceColNum: %d\n", thread_id, classifierColNum, priceColNum);

    string line;
    while (fin >> line)
    {
        vector<int> cur = separateByComma(line);

        bool category = (cur[priceColNum] >= threshold);

        Item item{cur[classifierColNum], category};

        pthread_mutex_lock (&mutex_expensive_cnt);
        expensive_cnt += category;
        pthread_mutex_unlock (&mutex_expensive_cnt);

        items[thread_id].push_back(item);
    }

    printf("This is thread %ld, Read %d items\n", thread_id, (int)items[thread_id].size());

    fin.close();
    pthread_exit(NULL);
}


void readInput()
{
    string line;
    for (int i = 0 ; ; i++)
    {
        ifstream fin("dataset_" + to_string(i) + ".csv");
        if (fin.good())
            NUMBER_OF_THREADS++;
        else
            break;
    }
    printf("NUMBER OF THREADS: %d\n", NUMBER_OF_THREADS);

    pthread_t threads[NUMBER_OF_THREADS];
    int return_code;
    for (long tid = 0 ; tid < NUMBER_OF_THREADS ; tid++)
    {
        return_code = pthread_create(&threads[tid], NULL, getItems, (void*)tid);

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

}

void* calcMeans(void* tid)
{
    long thread_id = (long)tid; 

    int n = items[thread_id].size();
    for (int i = 0 ; i < n ; i++)
        if (items[thread_id][i].category)
        {
            pthread_mutex_lock (&mutex_mean_1);
            mean_1 += (1 / (double)(expensive_cnt)) * (items[thread_id][i].x);
            pthread_mutex_unlock (&mutex_mean_1);
        }

    pthread_exit(NULL);
}


void calcMean()
{
    pthread_t threads[NUMBER_OF_THREADS];
    int return_code;
    for (long tid = 0 ; tid < NUMBER_OF_THREADS ; tid++)
    {
        return_code = pthread_create(&threads[tid], NULL, calcMeans, (void*)tid);

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
}

void* calcSTDs(void* tid)
{
    long thread_id = (long)tid; 

    int n = items[thread_id].size();
    for (int i = 0 ; i < n ; i++)
        if (items[thread_id][i].category)
        {
            pthread_mutex_lock (&mutex_std_1);
            std_1 += (1 / (double)(expensive_cnt)) * (items[thread_id][i].x - mean_1) * (items[thread_id][i].x - mean_1);
            pthread_mutex_unlock (&mutex_std_1);
        }

    pthread_exit(NULL);
}

void calcSTD()
{
    pthread_t threads[NUMBER_OF_THREADS];
    int return_code;
    for (long tid = 0 ; tid < NUMBER_OF_THREADS ; tid++)
    {
        return_code = pthread_create(&threads[tid], NULL, calcSTDs, (void*)tid);

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

    std_1 = sqrt(std_1);
}

bool predictSingle(Item& item)
{
    return (mean_1 - std_1 <= item.x && item.x <= mean_1 + std_1);
}

void* predictPrices(void* tid)
{
    long thread_id = (long)tid; 

    int n = items[thread_id].size();
    for (int i = 0 ; i < n ; i++)
    {
        pthread_mutex_lock (&mutex_correct);
        correct += (predictSingle(items[thread_id][i]) == items[thread_id][i].category);
        pthread_mutex_unlock (&mutex_correct);
    }

    pthread_exit(NULL);
}

void predictPriceCategories()
{
    pthread_t threads[NUMBER_OF_THREADS];
    int return_code;
    for (long tid = 0 ; tid < NUMBER_OF_THREADS ; tid++)
    {
        return_code = pthread_create(&threads[tid], NULL, predictPrices, (void*)tid);

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
} 

double getAccuracy()
{
    double res = 0;
    int total_items = 0;
    for (int i = 0 ; ; i++)
    {
        if (items[i].size() == 0)
            break;
        total_items += (int)items[i].size();
    }
    return (((double)(correct)) / ((double)(total_items)));
}

int main(int argc, char *argv[])
{
    threshold = atoi(argv[1]);          

    readInput();                //PARALLEL
    
    calcMean();                 //PARALLEL 

    calcSTD();                  //PARALLEL

    predictPriceCategories();   //PARALLEL


    double accuracy = getAccuracy();
    cout << "Accuracy: " << fixed << setprecision(2) << accuracy * 100 << "%" << endl;
    return 0;
}