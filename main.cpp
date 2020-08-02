#include <iostream>
#include <stdio.h>
#include <time.h>
#include <omp.h>
#include <string>
#include <fstream>
#include <bits/stdc++.h>
#include <stdlib.h>

using namespace std;

static string dataLogger = "***Data Logger***";
static int index = 1;
char endOfLine = '\n';

class cMatrix
{
public:
    int width;
    int height;
    float** data;
    double timeSeq, timePar;
    bool errorFlag;

    cMatrix(int w, int h);//default constructor, sets all elements to 0
    cMatrix(string sourceName);//reading constructor
    cMatrix(const cMatrix& source);//copy constructor
    ~cMatrix();
    void mPrint(string name);//print the elements to the file
};

cMatrix::cMatrix(int w, int h)//default constructor
{
    width = w;
    height = h;
    timePar = 0;
    timeSeq = 0;
    errorFlag = false;
    data = new float* [height];
    for (int i = 0; i < height; i++)
    {
        data[i] = new float[width];
        for (int j = 0; j < width; j++)
        {
            data[i][j] = 0;
        }
    }
}

cMatrix::cMatrix(const cMatrix& source)//copy constructor
{
    srand( time( NULL ) );
    width = source.width;
    height = source.height;
    timePar = source.timePar;
    timeSeq = source.timeSeq;
    data = new float* [height];
    for (int i = 0; i < height; i++)
    {
        data[i] = new float[width];
        for (int j = 0; j < width; j++)
        {
            data[i][j] = source.data[i][j];
        }
    }
}

cMatrix::cMatrix(string sourceName)//file copy constructor
{
    errorFlag = false;
    string line;
    string s;

    dataLogger += endOfLine;
    dataLogger += "File reading... ";

    ifstream sourceFile;
    sourceFile.open(sourceName);

    if (!sourceFile.is_open()){
        cout<<"Cannot open files."<<endl;
        dataLogger += "Cannot open files.";
        errorFlag = true;
        width = 1;
        height = 1;
        data = new float*[1];
        data[0] = new float;
        data[0][0] = 0;
    }

    sourceFile>>height;
    if(sourceFile.fail()){
        cout<<"Cannot read files."<<endl;
        dataLogger += "Cannot read files.";
        errorFlag = true;
        width = 1;
        height = 1;
        data = new float*[1];
        data[0] = new float;
        data[0][0] = 0;
    }

    sourceFile>>width;
    if(sourceFile.fail()){
        cout<<"Cannot read files."<<endl;
        dataLogger += "Cannot read files.";
        errorFlag = true;
        width = 1;
        height = 1;
        data = new float*[1];
        data[0] = new float;
        data[0][0] = 0;
    }

    getline(sourceFile, line); //before reading rows we need to change line

    data = new float* [height];
    for (int i = 0; i < height; i++)
    {
        data[i] = new float[width];
        getline(sourceFile, line);

        for (int j = 0; j < width; j++)
        {
            s = line.substr(0,line.find(";"));
            line.erase(0,s.length()+1);
            data[i][j] = atof(s.c_str());

            if(sourceFile.fail()){
                cout<<"Cannot read files."<<endl;
                dataLogger += "Cannot read files.";
                errorFlag = true;
                for (int i = 0; i < height; i++){//to prevent memory leaks
                    delete data[i];
                }
                delete data;
                width = 1;
                height = 1;
                data = new float*[1];
                data[0] = new float;
                data[0][0] = 0;
                return;
            }
        }
    }
    dataLogger += " OK";
}

cMatrix::~cMatrix()//destructor deallocates memory
{
    for (int i = 0; i < height; i++)
    {
        delete data[i];
    }
    delete data;
}

void cMatrix::mPrint(string name)//printing to file
{
    using namespace std;

    ofstream resultFile;
    errorFlag = false;

    string outputName = name;
    outputName += "_";
    outputName += to_string(timeSeq);
    outputName += "_";
    outputName += to_string(timePar);
    outputName += ".csv";

    dataLogger += endOfLine;
    dataLogger += "File writing... ";
    dataLogger += outputName;

    resultFile.open (outputName);

    resultFile<<height<<endl;
    resultFile<<width<<endl;

    for (int i = 0; i < height; i++)
    {
        for (int j = 0; j < width; j++)
        {
            resultFile<<fixed << setprecision(6)<<data[i][j];
            if(j<(width-1)){
                resultFile<<";";
            }
        }
        resultFile<<endl;
    }
    resultFile<<endl;
    resultFile.close();
    dataLogger += " OK";
}

cMatrix matrixMultiply(cMatrix* firstArg, cMatrix* secondArg, bool* errors)
//multiplies two given matrices, returns the result and gives report about error existence
{
    double time;
    dataLogger += endOfLine;
    dataLogger += "Multiplication trial: ";
    dataLogger += to_string(index);
    dataLogger += ", ";
    dataLogger += "first matrix: ";
    dataLogger += to_string(firstArg->height);
    dataLogger += "x";
    dataLogger += to_string(firstArg->width);
    dataLogger += ", ";
    dataLogger += "second matrix: ";
    dataLogger += to_string(secondArg->height);
    dataLogger += "x";
    dataLogger += to_string(secondArg->width);
    dataLogger += ", ";

    if (firstArg->width!=secondArg->height){
        std::cout<<"Dimension mismatch. Multiplication."<<std::endl;
        dataLogger += "Dimension mismatch. Multiplication.";
        *errors = true;
        cMatrix result = cMatrix(1, 1);
        return result;
    }

    cMatrix result = cMatrix( secondArg->width, firstArg->height);//result declaration

    //*************sequence part*******************************
    time = omp_get_wtime();
    std::cout<<"Sequence multiplication."<<std::endl;
    for (int i = 0; i < result.height; i++)
    {
        for (int j = 0; j < result.width; j++)
        {
            //default data value is set to 0 in class constructor
            for (int k = 0; k < firstArg->width; k++){
                result.data[i][j] = result.data[i][j] + (firstArg->data[i][k] * secondArg->data[k][j]);
            }
        }
    }
    *errors = false;
    time = omp_get_wtime() - time;
    result.timeSeq = time;
    cout<<"Sequence time: "<<result.timeSeq<<endl;
    dataLogger += "Sequence time: ";
    dataLogger += to_string(result.timeSeq);

    //*************parallel part*******************************
    time = omp_get_wtime();
    #pragma omp parallel
    //omp_set_num_threads(threadNum);
    {
        string message = "Parallel multiplication. Thread: ";
        message += to_string(omp_get_thread_num());
        message += endOfLine;
        std::cout<<message;

        #pragma omp for
        for (int i = 0; i < result.height; i++)
        {
            for (int j = 0; j < result.width; j++)
            {
                //default data value is set to 0 in class constructor
                for (int k = 0; k < firstArg->width; k++){
                    result.data[i][j] = result.data[i][j] + (firstArg->data[i][k] * secondArg->data[k][j]);
                }
            }
        }
        *errors = false;
    }
    time = omp_get_wtime() - time;
    result.timePar = time;
    cout<<"Parallel time: "<<result.timePar<<endl;
    dataLogger += ", Parallel time: ";
    dataLogger += to_string(result.timePar);
    //dataLogger += ", Thread number: ";
    //dataLogger += to_string(threadNum);

    return result;
}

int main()
{
    bool errors = false;
    string name = "C";

    int option;
    bool dataFlag = false;



    do{
            cout<<"Choose 1 to see the dataLogger:"<<endl;
            cout<<"Choose 2 to exit:"<<endl;
            cout<<"Choose 3 to perform multiplication:"<<endl;
            cin.clear();

            if(dataFlag){
                cin.ignore(10000,'\n');
            }

            cin>>option;

            dataFlag = true;
            if(cin.fail()){
                cout<<"Choose a correct value."<<endl;
                dataFlag = true;
                continue;
            }
            if (option==1){
                cout<<dataLogger<<endl;
                dataFlag = false;
            }

            else if (option==2){
                break;
            }

            else if (option ==3){
                cMatrix matrixA = cMatrix("A.csv");
                cMatrix matrixB = cMatrix("B.csv");

                cMatrix matrixC = cMatrix(matrixMultiply(&matrixA, &matrixB, &errors));
                if(!errors){
                    matrixC.mPrint(name);
                }
                else{
                    cout<<"Error."<<endl;
                }
            }

    }while(1);

    cout << "End of program." << endl;
    return 0;
}
