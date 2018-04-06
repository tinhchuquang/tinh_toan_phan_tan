#include <stdio.h>
#include <string.h>
#include <iostream>
#include <fstream>
#include <stdlib.h>
#include <openssl/md5.h>

using namespace std;

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



int main(int argc, char * argv[]){
    char password[10];
    if(argc != 2) return -1;
    sscanf(argv[1], "%s", password);
    char mdString[33];
    encode_password_MD5(password, mdString);
    printf("md5 digest: %s\n", mdString);
    return 0;
}

