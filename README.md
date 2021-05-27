## Parallel Price Predictor

The goal of this project is to classify houses in a dataset into two categories (expensive and inexpensive) based on their GrLivArea (the above ground (grade) living area). The classification is done by the Mean-Standard Deviation method. **POSIX Threads** is used for multithreading.



The inputs of the program are:
1. Directory of the dataset
2. Threshold for determining the category of a house price

The program is implemented in both serial and parallel. The serial version inputs one whole complete dataset. The parallel version inputs multiple datasets and assigns exactly 1 thread to each dataset. 

The program outputs the accuracy of the classifier. 

### How to run:

    $ make
    g++ -std=c++11  -pthread -c main.cpp -o main.o
    g++ -std=c++11  -pthread -o HousePricePrediction.out main.o
    $ ./HousePricePrediction.out ../datasets/ 330000
    Accuracy: 82.05%


There is also a CSVParser.cpp script used for breaking a large dataset into multiple datasets (set the constant NUMBER_OF_THREADS in the program manually). You simply put your dataset beside the binary file of this script and run it. This file is located in the /datasets/ directory.

### Results:
* Average run-time of serial version: 1.912s
* Average run-time of parallel version: 0.586s

* Speed-Up = 1.912 / 0.586 = 3.26 which is almost 4 (equal to number of threads)




