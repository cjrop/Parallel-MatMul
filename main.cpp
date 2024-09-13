#include <iostream>
#include <vector>
#include <thread>
#include <fstream> // for fileinput
#include <sstream> // for stringstream
#include <istream> // for istream_iterator
#include <iterator> // for istream_iterator
#include <mutex> // for mutex

using std::string; using std::vector; using std::thread;
using std::cout; using std::endl; using std::fstream;
using std::ifstream; using std::getline; using std::stringstream;
using std::istream_iterator; using std::ofstream; using std::pair;
using std::make_pair;

// ==================== GLOBAL VECTOR VARIABLES ====================
vector<vector<int>> matrixA;
vector<vector<int>> matrixB;
vector<vector<int>> result;
vector<vector<pair<int, int>>> cellsByThread;
std::mutex resultMutex;


// ===================== FUNCTION PROTOTYPES =====================
void printMatrix(const vector<vector<int>>& matrix);
void loadMatrixFromFile(fstream& fileInput, vector<vector<int>>& matrix);
void computeCell(int rowMatrixA, int colMatrixB);
void computeCellsForThread(int threadIndex);

// ===================== MAIN =====================
int main(){
    cout << "Enter filename: ";
    string fileName;
    getline(std::cin, fileName);
    fstream fileInput;
    fileInput.open(fileName);
    //check if file is open
    if(!fileInput.is_open()) {
        cout << "File not found\n";
        return 1;
    }

    loadMatrixFromFile(fileInput, matrixA);    //load matrix A from file
    loadMatrixFromFile(fileInput, matrixB);    //load matrix B from file

    // multiply matrices
    for(auto i = 0 ; i <  matrixA.size(); ++i){
        result.emplace_back(matrixB[0].size(), 0); //push back a row
    }

    unsigned int maxThreads = thread::hardware_concurrency(); // get number of threads
    vector<thread> threads; // vector of threads
    for(auto i =0; i < maxThreads; ++i){ // create threads
        cellsByThread.emplace_back(); // push back a vector of cells
    }

    int counter = 0;
    for(auto i = 0; i < matrixA.size(); ++i){ // for each row of matrix A
        for(auto j = 0; j < matrixB[0].size(); ++j) { // for each column of matrix B
            //distributing all the cells that needs to be computed
            cellsByThread[counter % maxThreads].push_back(make_pair(i,j));
            counter++; // increment counter
        }
    }

    threads.reserve(maxThreads); // reserve space for threads
    for(auto i = 0; i < maxThreads; ++i){ // create threads
        threads.emplace_back(computeCellsForThread, i); // push back a thread
    }

    for(auto & thread : threads){ // join threads
        thread.join(); //waits for finished execution of threads
    }
    printMatrix(result); // print result

    return 0;
} //end main

// ====================== CODE IMPLEMENTATION ======================
//this function will take a 2d vec and print it
void printMatrix(const vector<vector<int>>& matrix){
    for(const auto& row : matrix){
        for(auto cell : row){
            cout << cell << ' ';
        }
        cout << endl;
    }
    cout << endl;
}

void loadMatrixFromFile(fstream& fileInput, vector<vector<int>>& matrix) {
    // load matrix from file and store it in the matrix vector 
    if (!fileInput.is_open()) { // Added 10Jul
        throw std::runtime_error("Failed to open file");
    }
    string line;
    //consume the first dimensions of the text
    getline(fileInput, line);
    getline(fileInput, line); // consume the second dimensions
    // if it's empty skip, if it's not, then parse it
    if (!line.empty()) {
        stringstream ss(line); 
        auto start = istream_iterator<int>{ ss }; // 
        auto end = istream_iterator<int>{}; // end of stream
        matrix.emplace_back(start, end); // push back a row
    }

    while (getline(fileInput, line)) { // while there are more lines
        // skip when we get to the break between the matrices
        if (line.empty())
            break;

        stringstream ss(line);
        auto start = istream_iterator<int>{ ss };
        auto end = istream_iterator<int>{};
        matrix.emplace_back(start, end);
    }
}

void computeCell(int rowMatrixA, int colMatrixB){
    // computes the cell value in the result matrix using the row from matrix A and the column from matrix B
    int res = 0;
    // iterate through the row of matrix A
    // and multiply the cell value with the column from matrix B
    // and add the result to the res variable
    // compute cell value by multiplying the row from matrix A with the column from matrix B
    for(auto col = 0; col < matrixA[rowMatrixA].size(); ++col){
        res += matrixA[rowMatrixA][col] * matrixB[col][colMatrixB]; 
    }
    // Ensure thread safety
    std::lock_guard<std::mutex> guard(resultMutex);
    result[rowMatrixA][colMatrixB] = res; //store into global result vec
}

void computeCellsForThread(int threadIndex) {
    // Directly compute the cells assigned to this thread
    for(auto& cell : cellsByThread[threadIndex]){
        computeCell(cell.first, cell.second);
    }
}