#include <iostream>
#include <fstream>
#include <vector>
#include <math.h>
#include <iomanip> 
#include <pthread.h>
#include <stdio.h>

using namespace std;


#define CLASSIFIER_COL 5
#define PRICE_COL 8
#define COMMA ','
#define EMPTY_STR ""
#define NUM
#define FILENAME "dataset.csv"

#define NUMBER_OF_THREADS 4

int threshold;
vector<string> lines;
string head;

void* writeToFile(void* tid)
{
    long thread_id = (long)tid;
    //printf("In writeToFile: this is thread %ld\n", thread_id);

    int len = (lines.size()) / NUMBER_OF_THREADS;

    int start = thread_id * len;
    int end = start + len;

    //printf("In writeToFile: this is threa %ld, %d %d\n", thread_id, start, end);
    
    FILE *fp;
    string filename = "dataset_" + to_string(thread_id) + ".csv"; 
    fp = fopen(filename.c_str(), "w+");
    fprintf(fp, head.c_str());
    
    for (int i = start ; i < end ; i++)
        fprintf(fp, lines[i].c_str());    

    fclose(fp);

    pthread_exit(NULL);
}

void breakCSV()
{
    string line;   
    ifstream fin(FILENAME);
    int row = 0;
    while (getline(fin, line))
    {
        if (row == 0) 
        {
            head = line;
            row++;
            continue;
        }
        lines.push_back(line);
        row++;
    }

    pthread_t threads[NUMBER_OF_THREADS];
    int return_code;
    for (long tid = 0 ; tid < NUMBER_OF_THREADS ; tid++)
    {
        //printf("in readInput: Creating thread %ld\n", tid);
        return_code = pthread_create(&threads[tid], NULL, writeToFile, (void*)tid);

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

    //printf("breakCSV() exiting\n");
    pthread_exit(NULL);
    fin.close();
}

int main(int argc, char *argv[])
{
    threshold = atoi(argv[1]);
    breakCSV();          

    readInput();     
    return 0;
}