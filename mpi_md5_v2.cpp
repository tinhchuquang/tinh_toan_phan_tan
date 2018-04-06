#include <stdio.h>
#include <string.h>
#include <iostream>
#include <fstream>
#include <stdlib.h>
#include <map>
#include <list>
#include <iterator>
#include <openssl/md5.h>
#include <math.h>
#include <mpi.h>
#include <time.h>
#define FILE_NAME "data.txt"
#define DATA 40
#define RESULT 60


using namespace std;

char character[26] = {'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l'
    , 'm', 'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z'};
map<char, int> dictionary;

void encode_password_MD5(const char *string, char *mdString){
    unsigned char digest[16];
    // char mdString[33];

    // encode password into MD5
    MD5_CTX ctx;
    MD5_Init(&ctx);
    MD5_Update(&ctx, string, strlen(string));
    MD5_Final(digest, &ctx);

    for (int i = 0; i < 16; i++)
        sprintf(&mdString[i*2], "%02x", (unsigned int)digest[i]);
    // return mdString;
}

void generator_password(char temp_password[], int *is_pause, int point, int i){
    map<char, int> ::iterator itr;
    if (i == point){
        if (temp_password[i] == 'z'){
            *is_pause = 1;
            return;
        }
        else{
            itr = dictionary.find(temp_password[i]);
            temp_password[i] = character[itr->second + 1];
        }
    }
    else{
        if(temp_password[i] == 'z'){
            temp_password[i] = 'a';
            generator_password(temp_password, is_pause, point, i+1);
        }
        else{
            itr = dictionary.find(temp_password[i]);
            temp_password[i] = character[itr->second + 1];
        }
    }
}


void write_password(int len_password){

    char temp_password[len_password]; 
    for(int i = 0; i< 26; i++){
        dictionary.insert(pair<char, int>(character[i], i));
    }
    for(int i= 0; i< len_password; i++){
        temp_password[i] = 'a';
    }
    ofstream password_file("password.txt");
    int is_pause = 0;
    while(password_file.is_open() && !is_pause){
        string str = "";
        for(int i = 0; i < len_password; i++){
            str += temp_password[i];
        }
        password_file << str;
        password_file <<"\n";
        generator_password(temp_password, &is_pause, len_password-1, 0);
    }
    password_file.close();

    // itr = dictionary.find('d');
    // cout<<itr->second<<"\n";

}


void split_data(list<string> *password_bound, int number_process, int len_password){
    int total_password = pow(26, len_password);
    int part_password = total_password / number_process;
    string line;
    ifstream readfile("password.txt");
    int count = 0;
    for (int i = 0; i< len_password; i++){
        line += 'a';
    }
    password_bound->push_back(line);
    
    for(int i = 0; i< number_process-1; i++){
        count = (i+1)*part_password;
        while(1){
            line.clear();
            if(count > 26){
                line += character[(count % 26)];
                count = count/26;
            }else{
                line += character[count];
                break;
            }
        }
        password_bound->push_back(line);
    }

    line.clear();
    for (int i = 0; i< len_password; i++){
        line += 'z';
    }
    password_bound->push_back(line);
}

void readfiletxt(){
    string line;
    ifstream readfile("password.txt");
    while(getline(readfile, line)){
        if(!line.compare("tinh")){
            cout<<line<<endl;
        }
    }
}

char *find_password(char password_begin[], char password_end[], char *mdString, char *temp_password_found, int len_password){
    int is_pause = 0;
    temp_password_found = NULL;
    char temp_mdString[33];
    while(!is_pause){
        
        encode_password_MD5(password_begin, temp_mdString);
        if(!strcmp(temp_mdString, mdString)){
            temp_password_found = password_begin;
            break;
        }
        if(!strcmp(password_begin, password_end))
            break;
        generator_password(password_begin, &is_pause, len_password-1, 0);
    }
    
    return temp_password_found;
}

void string_to_char(list<string>::iterator itr, char array_text[]){
    string text = *itr;
    for(int i = 0; i< text.length(); i++){
        array_text[i] = text[i];
    }
}

void rank0(char *mdString, int len_password){
    // printf("md5 digest: %s\n", mdString);
    clock_t t1,t2;
    t1 = clock();

    int number_process;
    MPI_Status status;
    MPI_Request request;
    MPI_Comm_size(MPI_COMM_WORLD, &number_process);
    list<string> password_bound;
    list<string>::iterator itr;
    split_data(&password_bound, number_process-1, len_password);
    itr = password_bound.begin();

    // for(;itr != password_bound.end(); itr++){
    //     cout<<(*itr)<<endl;
    // }

    // itr = password_bound.begin();
    char array_text[len_password];
    for(int i = 1; i< number_process; i++){
        // MPI_Send(&number_element, 1, MPI_INT, i, NE, MPI_COMM_WORLD);
        string_to_char(itr, array_text);
        MPI_Send(array_text, len_password, MPI_CHAR, i, DATA, MPI_COMM_WORLD);
        itr++;
        string_to_char(itr, array_text);
        MPI_Send(array_text, len_password, MPI_CHAR, i, DATA, MPI_COMM_WORLD);
    }
    
    char *temp_password_found;
    string temp_password;
 
    temp_password_found = NULL;
    char password_process[len_password];
    for (int i = 1; i < number_process; i++){
        MPI_Recv(password_process, len_password, MPI_CHAR, i, RESULT, MPI_COMM_WORLD, &status);
        if(strcmp(password_process, "No")){
            cout<<"password found :";
            for (int j = 0; j< len_password; j++)    
                cout<<password_process[j];
            cout<<" from :"<<i<<endl;
            temp_password_found = NULL;
        }
    }
    t2=clock();
    float diff ((float)t2-(float)t1);
    float seconds = diff / CLOCKS_PER_SEC;
    cout << "Time required = " << seconds << " seconds\n";
}

void ranki(char *mdString, int len_password){
    int number_element;
    char buff_begin[len_password];
    char buff_end[len_password];
    char *temp_password_found;
    MPI_Status status;
    MPI_Request request;

    MPI_Recv(buff_begin, len_password, MPI_CHAR, 0, DATA, MPI_COMM_WORLD, &status);
    MPI_Recv(buff_end, len_password, MPI_CHAR, 0, DATA, MPI_COMM_WORLD, &status);
    // cout<<buff_begin<<" : "<<buff_end<<endl;

    char password_process[len_password];
    char password_no_found[] = "No";
    temp_password_found = find_password(buff_begin, buff_end, mdString, temp_password_found, len_password);
    if(temp_password_found != NULL){
        for (int i = 0; i< len_password; i++)    
            password_process[i] = temp_password_found[i];
        MPI_Send(password_process, len_password, MPI_CHAR, 0, RESULT, MPI_COMM_WORLD);
    }else
        MPI_Send(password_no_found, len_password, MPI_CHAR, 0, RESULT, MPI_COMM_WORLD);
}



int main(int argc, char **argv) {
    
    int rank;
    MPI_Init(&argc, &argv);
    // encode MD5
    
    // char password[10];
    // if(argc != 2) return -1;
    // sscanf(argv[1], "%s", password);
    
    for(int i = 0; i< 26; i++){
        dictionary.insert(pair<char, int>(character[i], i));
    }

    char mdString[33];
    int len_password;
    if(argc != 3) return -1;
    sscanf(argv[1], "%s", mdString);
    sscanf(argv[2], "%d", &len_password);
    // printf("md5 digest: %s\n", mdString);
    // encode_password_MD5(password, mdString);
    
    // write_password(len_password);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    if (rank == 0)
        rank0(mdString, len_password);
    else
        ranki(mdString, len_password);
    // printf("md5 digest: %s\n", mdString);
    
    MPI_Finalize();
   
    return 0;
}