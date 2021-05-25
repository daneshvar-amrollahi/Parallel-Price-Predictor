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
double std_1;
double sum2[MAX_THREAD_NUMBERS];
long sum[MAX_THREAD_NUMBERS];
double total_mean_1;
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

    
    //printf("This is thread %ld, claasifierColNum: %d  priceColNum: %d\n", thread_id, classifierColNum, priceColNum);

    string line;
    while (fin >> line)
    {
        vector<int> cur = separateByComma(line);

        bool category = (cur[priceColNum] >= threshold);

        Item item{cur[classifierColNum], category};

        items[thread_id].push_back(item);
    }

    //printf("This is thread %ld, Read %d items\n", thread_id, (int)items[thread_id].size());

    fin.close();
    pthread_exit(NULL);
}


void readInput()
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
    //printf("NUMBER OF THREADS: %d\n", NUMBER_OF_THREADS);

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

void* calcSums(void* tid)
{
    long thread_id = (long)tid; 

    sum[thread_id] = 0;
    int n = items[thread_id].size();
    for (int i = 0 ; i < n ; i++)
        if (items[thread_id][i].category)
        {
            expensive_cnt[thread_id] += 1;
            sum[thread_id] += (items[thread_id][i].x); //no other thread touches this
        }
    //printf("This is thread %ld, my sum is %ld\n", thread_id, sum[thread_id]);
    pthread_exit(NULL);
}


void calcMean()
{
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
    total_expensive_cnt = 0;
    for (int i = 0 ; i < NUMBER_OF_THREADS ; i++)
    {
        total_expensive_cnt += expensive_cnt[i];
        total_items += items[i].size();
        total_sum += sum[i];
    }

    //printf("%f %d\n", total_sum, total_expensive_cnt);
    total_mean_1 = total_sum / total_expensive_cnt;
    //printf("%f\n", total_mean_1);
}

void* calcSum2(void* tid)
{
    long thread_id = (long)tid; 

    int n = items[thread_id].size();
    for (int i = 0 ; i < n ; i++)
        if (items[thread_id][i].category)
            sum2[thread_id] += (items[thread_id][i].x - total_mean_1) * (items[thread_id][i].x - total_mean_1);

    pthread_exit(NULL);
}

void calcSTD()
{
    pthread_t threads[NUMBER_OF_THREADS];
    int return_code;
    for (long tid = 0 ; tid < NUMBER_OF_THREADS ; tid++)
    {
        return_code = pthread_create(&threads[tid], NULL, calcSum2, (void*)tid);

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

    double total_sum2 = 0;
    for (int i = 0 ; i < NUMBER_OF_THREADS ; i++)
        total_sum2 += sum2[i];

    std_1 = sqrt(total_sum2 / total_expensive_cnt);
}

bool predictSingle(Item& item)
{
    return (total_mean_1 - std_1 <= item.x && item.x <= total_mean_1 + std_1);
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

    clock_t start, end;
    start = clock();
    readInput();                //PARALLEL
    
    calcMean();                 //PARALLEL 


    calcSTD();                  //PARALLEL

    predictPriceCategories();   //PARALLEL

    double accuracy = getAccuracy();
    end = clock();

    double time_taken = double(end - start) / double(CLOCKS_PER_SEC);    
    cout << "Accuracy: " << fixed << setprecision(2) << accuracy * 100 << "%" << endl;
    printf("Time Elapsed: %f\n", time_taken);
    return 0;
}

//  2433.31 589.787