#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <bits/stdc++.h>

using namespace std;

bool searchFileHash(string,string);

void updateSeederList(string fromClient,string seederFile){
    cout<<"inside\n";
    
    cout<<fromClient<<"\n";
    
    bool present = searchFileHash(fromClient,seederFile);
    if(present)
        return;


    char file[seederFile.length()+1];
    strcpy(file,(char *)seederFile.c_str());
    cout<<"inside\n";
    cout<<fromClient<<"\n";
    FILE *seedFilefp = fopen(file,"a");
    char buffer[fromClient.length()+1];
    strcpy(buffer,(char *)fromClient.c_str());
    
    fputs (buffer,seedFilefp);
    fprintf(seedFilefp, "\n");
    fclose(seedFilefp);
}

bool searchFileHash(string fromClient,string seederFile){

    char file[seederFile.length()+1];
    strcpy(file,(char *)seederFile.c_str());

    ifstream seedFilefp;

    seedFilefp.open (seederFile, ifstream::in);


    string fromFile,temp;

    while(getline(seedFilefp,fromFile)){
        if(fromFile.compare(fromClient)==0){
            return true;
        }
    }

    return false;
}

string searchHash(string fromClient,string seederFile){

    char file[seederFile.length()+1];
    strcpy(file,(char *)seederFile.c_str());

    ifstream seedFilefp;

    seedFilefp.open (seederFile, ifstream::in);


    string fromFile,temp;

    while(getline(seedFilefp,fromFile)){

        size_t found = fromFile.find(fromClient);
        if (found!=std::string::npos){
            return fromFile;
        }
    }

    return "";
}

void removeFromSeederList(string fromClient,string seederFile){
    
    cout<<fromClient<<"\n";
    cout<<"inside remove\n";
    bool present = searchFileHash(fromClient,seederFile);
    if(!present)
        return;

    char file[seederFile.length()+1];
    strcpy(file,(char *)seederFile.c_str());

    string line;

    ifstream fin;
    fin.open(file);
    ofstream temp;
    temp.open("temp.txt");
    
    while (getline(fin,line))
    {
        cout<<line<<"\n";

        if(line==fromClient)
            line = "";
        temp << line << endl;

    }

    temp.close();
    fin.close();
    remove(file);
    rename("temp.txt",file);
}


int main(int argc, char *argv[]){

    string seperator = ":";

    if(argc<5){
        printf("Invalid number of argument\n");
        exit(0);
    }

    string temp,tracker1IP,tracker1Port,tracker2IP,tracker2Port,seederFile,logFile;

    temp = argv[1];

    size_t found = temp.find(seperator);
    
    if (found!=string::npos){
        tracker1IP = temp.substr(0,found);
        tracker1Port = temp.substr(found+1);
    }

    temp = argv[2];

    found = temp.find(seperator);

    if (found!=string::npos){
        tracker2IP = temp.substr(0,found);
        tracker2Port = temp.substr(found+1);
    }

    seederFile = argv[3];

    logFile = argv[4];

    //opening seeder list file
    //fstream seedfp = openSeederList(seederFile);

    //connectToTracker2();
    //syncWithTracker2();

    int listenfd = 0;
    int connfd = 0;
    struct sockaddr_in serv_addr;
    char sendBuff[524289];
    
    listenfd = socket(AF_INET, SOCK_STREAM, 0);

    printf("Socket retrieve success\n");

    memset(&serv_addr, '0', sizeof(serv_addr));
    memset(sendBuff, '0', sizeof(sendBuff));

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = inet_addr(tracker1IP.c_str());

    int port = stoi (tracker1Port,nullptr,10);

    serv_addr.sin_port = htons(port);

    bind(listenfd, (struct sockaddr*)&serv_addr,sizeof(serv_addr));

    if(listen(listenfd, 10) == -1)
    {
        printf("Failed to listen\n");
        return -1;
    }

    char buffer[524288];
    while(1){

        connfd = accept(listenfd, (struct sockaddr*)NULL ,NULL);
        int n;
        memset(buffer, 0, 524288);
        n = read( connfd , buffer, 524288); 
        //write(connfd, buff, nread);
        
        string fromClient = buffer;
        cout<<fromClient<<"\n";

        found = fromClient.find_first_of("|");

        if (found!=string::npos){
            temp = fromClient.substr(0,found);
            fromClient = fromClient.substr(found+1);
        }

        string clientAddress;
        cout<<fromClient<<"\n";
        if(temp.compare("fetch")==0){
            clientAddress = searchHash(fromClient,seederFile);    
        }else if(temp.compare("share")==0){
            updateSeederList(fromClient,seederFile);
        }else if(temp.compare("remove")==0){
            removeFromSeederList(fromClient,seederFile);
        }
        

        if(clientAddress.length()==0)
            cout<<"No client has the requested file\n";
        else{
            strcpy(buffer,(char *)clientAddress.c_str());
            write(connfd, buffer, 524288);    
            memset(buffer, 0, 524288);
        }

        close(connfd);
        
    }


    return 0;
}