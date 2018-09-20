#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <arpa/inet.h>
#include <bits/stdc++.h>
#include <openssl/sha.h>
#include <thread> 
#include <pthread.h>
#include "mtorrent.h"


using namespace std;


struct socketInfo{

	string ip;
	string port;
};

string& ltrim(std::string& s){
    auto it = std::find_if(s.begin(), s.end(),[](char c) {
                                return !std::isspace<char>(c, std::locale::classic());
                            });
    s.erase(s.begin(), it);
    return s;
}

string& rtrim(std::string& s){
    auto it = std::find_if(s.rbegin(), s.rend(),
                        [](char c) {
                            return !std::isspace<char>(c, std::locale::classic());
                        });
    s.erase(it.base(), s.end());
    return s;
}

string& trim(std::string& s){
    return ltrim(rtrim(s));
}



string getSHA1FromMtorrentFile(string filename){
    char file[filename.length()+1];
    strcpy(file,filename.c_str());

    ifstream torrentFile(file);
    
    string line;

    for(int i=0;i<4;i++){
        getline(torrentFile,line);
    }

    getline(torrentFile,line);

    char data[line.length()+1];
    strcpy(data,line.c_str());
    char buf[SHA_DIGEST_LENGTH*2];
    unsigned char hash[SHA_DIGEST_LENGTH]={'\0'};
        
    SHA1((unsigned char*)data, strlen(data), hash);

    for (int i=0; i < SHA_DIGEST_LENGTH; i++) {
        sprintf((char*)&(buf[i*2]), "%02x", hash[i]);
    }

    string toTracker(buf);

    return toTracker;
}


void sendSeedFileInfoToTracker(string filename,string clientIP,string clientPort,string tracker1IP,string tracker1Port){

    string tempfilename = filename+".mtorrent";

    string toTracker = getSHA1FromMtorrentFile(tempfilename);

    toTracker = "share|"+filename+"|"+clientIP+"|"+clientPort+"|"+toTracker;

    char trackerInfo[toTracker.length()+1];
    strcpy(trackerInfo,toTracker.c_str());

    int sockfd = 0;
    struct sockaddr_in serv_addr;

    if((sockfd = socket(AF_INET, SOCK_STREAM, 0))< 0){
            
            printf("\n Error : Could not create socket \n");
            //return 1;
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = inet_addr(tracker1IP.c_str());
    serv_addr.sin_port = htons(stoi (tracker1Port,nullptr,10));

    
    if(connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr))<0)
    {
        printf("\n Error : Connect Failed \n");
        //return 1;
    }

    
    send(sockfd,trackerInfo,strlen(trackerInfo),0);

}


string getFileInfoFromTracker(string filename,string tracker1IP,string tracker1Port){

    filename = filename+".mtorrent";

    cout<<"inside getFileInfoFromTracker()\n";
    cout<<"filename: "<<filename<<"\n";
    string toTracker = getSHA1FromMtorrentFile(filename);

    cout<<"from get sha1 from torrent file: "<<toTracker<<"\n";

    toTracker = "fetch|"+toTracker;

    char trackerInfo[toTracker.length()+1];
    strcpy(trackerInfo,toTracker.c_str());

    int sockfd = 0;
    int bytesReceived = 0;
    char recvBuff[524289];
    memset(recvBuff, '0', sizeof(recvBuff));
    struct sockaddr_in serv_addr;

    if((sockfd = socket(AF_INET, SOCK_STREAM, 0))< 0){
            
            printf("\n Error : Could not create socket \n");
            //return 1;
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = inet_addr(tracker1IP.c_str());
    serv_addr.sin_port = htons(stoi (tracker1Port,nullptr,10));

    
    if(connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr))<0)
    {
        printf("\n Error : Connect Failed \n");
        //return 1;
    }

    string fromTracker;
    send(sockfd,trackerInfo,strlen(trackerInfo),0);

    while((bytesReceived = read(sockfd, recvBuff, 524288)) > 0){
            recvBuff[524288] = '\0';
            printf("Bytes received %d\n",bytesReceived);    
            string temp = recvBuff;
            fromTracker += temp;
    }


    return fromTracker;

}


void fireClientThread(string filename,string sourceIP,string sourcePort,string tracker1IP,string tracker1Port){

	int sockfd = 0;
    int bytesReceived = 0;
    char recvBuff[524289];
    memset(recvBuff, '0', sizeof(recvBuff));
    struct sockaddr_in serv_addr;


    char file[filename.length()+1];
    strcpy(file,filename.c_str());


    if((sockfd = socket(AF_INET, SOCK_STREAM, 0))< 0){

        printf("\n Error : Could not create socket \n");
    }

    serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = inet_addr(sourceIP.c_str());
	serv_addr.sin_port = htons(stoi (sourcePort,nullptr,10));

    if(connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr))<0){

        printf("\n Error : Connect Failed \n");
        
    }

    
    send(sockfd,file,strlen(file),0);


    FILE *fp = fopen(file, "ab"); 
    if(NULL == fp){

        printf("Error opening file");
        
    }

    while((bytesReceived = read(sockfd, recvBuff, 524288)) > 0){

        //printf("Bytes received %d\n",bytesReceived);    
        fwrite(recvBuff, 1,bytesReceived,fp);
    }

    //sendSeedFileInfoToTracker(filename,sourceIP,sourcePort,tracker1IP,tracker1Port);
  
    if(bytesReceived < 0){

        printf("\n Read Error \n");
    }
    fclose(fp);  

}


void fireServerThread(struct socketInfo MY_SOCKET){

	int listenfd = 0;
    int connfd = 0;
    struct sockaddr_in serv_addr;
    char sendBuff[524289];
    int numrv;

    listenfd = socket(AF_INET, SOCK_STREAM, 0);

    //printf("Socket retrieve success\n");

    memset(&serv_addr, '0', sizeof(serv_addr));
    memset(sendBuff, '0', sizeof(sendBuff));

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = inet_addr((MY_SOCKET.ip).c_str());
    serv_addr.sin_port = htons(stoi (MY_SOCKET.port,nullptr,10));

    bind(listenfd, (struct sockaddr*)&serv_addr,sizeof(serv_addr));

    if(listen(listenfd, 10) == -1)
    {
        printf("Failed to listen\n");
        
    }

    char buffer[524289];
    
    while(true){

        connfd = accept(listenfd, (struct sockaddr*)NULL ,NULL);

        
        memset(buffer, 0, 524289);
        
        //get file details from client who wants to download
        read( connfd , buffer, 524289); 

        string totrim = buffer;

        totrim = trim(totrim);

        char fileToSend[totrim.length()+1];
        
        strcpy(fileToSend,(char*)totrim.c_str());


        printf("File to send%s\n",fileToSend);

        FILE *fp = fopen(fileToSend,"rb");
        if(fp==NULL)
        {
            printf("File open error");
            
        }   

        /* Read data from file and send it */
        while(1)
        {
            /* First read file in chunks of 256 bytes */
            unsigned char buff[524289]={0};
            int nread = fread(buff,1,524288,fp);
            printf("Bytes read %d \n", nread);        

            /* If read was success, send data. */
            if(nread > 0)
            {
                printf("Sending \n");
                write(connfd, buff, nread);
            }

           
            if (nread < 524288)
            {
                if (feof(fp))
                    printf("End of file\n");
                if (ferror(fp))
                    printf("Error reading\n");

                break;
            }


        }

        close(connfd);
        fclose(fp);

}}

int main(int argc, char *argv[]){

	string seperator = ":";

    if(argc<5){
        printf("Invalid number of argument\n");
        exit(0);
    }

    string temp,tracker1IP,tracker1Port,tracker2IP,tracker2Port,logFile;
    
    struct socketInfo MY_SOCKET;
    
    temp = argv[1];
    
    size_t found = temp.find(seperator);
    
    if (found!=string::npos){
        MY_SOCKET.ip = temp.substr(0,found);
        MY_SOCKET.port = temp.substr(found+1);
    }
    
    temp = argv[2];
    
    found = temp.find(seperator);
    
    if (found!=string::npos){
        tracker1IP = temp.substr(0,found);
        tracker1Port = temp.substr(found+1);
    }

    temp = argv[3];

    found = temp.find(seperator);
    
    if (found!=string::npos){
        tracker2IP = temp.substr(0,found);
        tracker2Port = temp.substr(found+1);
    }

	logFile = argv[4];    

	thread serverThread(fireServerThread, MY_SOCKET);
	
	//thread clientThread(fireClientThread, MY_SOCKET);
	while(true){
		string command,temp;
		cout<<"Enter Your Command: ";
		getline(cin,command);	
		//cout<<command<<"\n";
        //temp = command.substr(0,5);

		if(command.substr(0,5)=="share"){

			string filename = command.substr(6);
            cout<<filename<<"\n";

            filename = trim(filename);
            tracker1IP = trim(tracker1IP);
            tracker1Port = trim(tracker1Port);
            tracker2IP = trim(tracker2IP);
            tracker2Port = trim(tracker2Port);
            MY_SOCKET.ip = trim(MY_SOCKET.ip);
            MY_SOCKET.port = trim(MY_SOCKET.port);

            createMtorrentFile(filename,tracker1IP,tracker1Port,tracker2IP,tracker2Port);
        
            sendSeedFileInfoToTracker(filename,MY_SOCKET.ip,MY_SOCKET.port,tracker1IP,tracker1Port);
		
		}else if(command.substr(0,3)=="get"){

			string filename = command.substr(3);

            filename = trim(filename);
            tracker1IP = trim(tracker1IP);
            tracker1Port = trim(tracker1Port);
            tracker2IP = trim(tracker2IP);
            tracker2Port = trim(tracker2Port);
            MY_SOCKET.ip = trim(MY_SOCKET.ip);
            MY_SOCKET.port = trim(MY_SOCKET.port);

            cout<<"Sending filename to tracker: "<<filename<<"\n";
            cout<<"tracker ip: "<<tracker1IP<<"\n";
            cout<<"tracker port: "<<tracker1Port<<"\n";

		    string fromTracker = getFileInfoFromTracker(filename,tracker1IP,tracker1Port);
		    
            cout<<"fromTracker: "<<fromTracker<<"\n";            

            size_t found = fromTracker.find_first_of("|");
		    fromTracker = fromTracker.substr(found+1);
		    found = fromTracker.find_first_of("|");
		    string sourceIP = fromTracker.substr(0,found);

		    fromTracker = fromTracker.substr(found+1);

            found = fromTracker.find_first_of("|");
            string sourcePort = fromTracker.substr(0,found);

		    thread clientThread(fireClientThread, filename,sourceIP,sourcePort,tracker1IP,tracker1Port);
			clientThread.join();
		}
	}
	
	serverThread.join();
	
	return 0;
}