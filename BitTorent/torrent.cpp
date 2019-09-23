#include<bits/stdc++.h>
#include <openssl/sha.h>

using namespace std;

int SHA_CHAR_LENGTH = 20;
int CHUNCK_SIZE = 524288; //figure in bytes
bool LOGFLAG = true;

void log_message(string msg){

	string logFile = "log.txt";
	fstream fio;

	fio.open((char*)logFile.c_str(),ios::app | ios::in);
	
	fio<<msg<<"\n";
	fio.close();
}

streampos getFileSize(string filePath){

    streampos fsize = 0;
    ifstream file((char*)filePath.c_str(), ios::binary );

    fsize = file.tellg();
    file.seekg( 0, ios::end);
    fsize = file.tellg() - fsize;
    file.close();

    return fsize;
}

bool createTorrentFile(string fileName,string peerIp,string peerPort, int chunckSize=CHUNCK_SIZE){

	if(LOGFLAG)
		log_message("Entering createTorrentFile(): "+fileName+" "+peerIp+" "+peerPort);

    string torrentName = fileName+".torrent";
    
    fstream fio; 

	try{
	
		fio.open((char*)torrentName.c_str(), ios::trunc | ios::out | ios::in); 
 
		fio<<fileName<<"\n";

	    streampos size;

	    try{

			size = getFileSize(fileName);	    	
	    	fio<<size<<"\n";
	    }
	    catch(...){
	    	if(LOGFLAG)
				log_message("createTorrentFile(): Error while getting file size\n");
			return false;
	    }

	    fio<<chunckSize<<"\n";

	    int numChuncks = size%chunckSize==0?(size/chunckSize):floor(size/chunckSize)+1;

	    for(int i = 1;i<=numChuncks;i++){
	    	if(i!=numChuncks)
	    		fio<<i<<",";
	    	else
	    		fio<<i;
	    }
	    fio<<"\n";

	    try{

			ifstream file;
			file.open((char*)fileName.c_str(), ios::binary);
	    
			char buf[SHA_CHAR_LENGTH+1];
		    char data[chunckSize+1]={0};
			unsigned char hash[SHA_DIGEST_LENGTH] = {'\0'};


			while(!file.eof()){

				file.read(data, chunckSize);
				streamsize s = ((file) ? chunckSize : file.gcount());

        
		        SHA1((unsigned char*)data, chunckSize, hash);

		        for (int i=0; i < SHA_CHAR_LENGTH/2 ; i++) {
		            sprintf((char*)&(buf[i*2]), "%02x", hash[i]);
		        }	

				fio<<string(buf)<<"\n";

				memset(data,0,sizeof data);
				memset(buf,0,sizeof buf);
				memset(hash,0,sizeof hash);

				if(!file) 
					break;
			}


			file.close();

	    }catch(...){
	    	if(LOGFLAG)
				log_message("createTorrentFile(): Error while opening data file for creating hash\n");
			return false;	
	    }
		
		fio<<peerIp+":"+peerPort<<"\n";

	    fio.close();
	} 
	catch(...){
		if(LOGFLAG)
			log_message("createTorrentFile(): Error while opening torrent file\n");
		return false;
	}

	if(LOGFLAG)
		log_message("Exiting createTorrentFile(): success");

    return true;
}

bool updateTorrentFile(string fileName,string peerIp,string peerPort){

	if(LOGFLAG)
		log_message("Entering updateTorrentFile(): "+peerIp+" "+peerPort);

    string torrentName = fileName+".torrent";
    string val = peerIp+":"+peerPort+"\n";
    ofstream fo; 

	try{

		fo.open((char*)torrentName.c_str(), ios::app);
		fo.seekp(0);

		fo.write((char*)val.c_str(),val.length());

		fo.close();
	}
	catch(...){
		if(LOGFLAG)
			log_message("updateTorrentFile(): Error while opening torrent file\n");
		return false;	
	}

	if(LOGFLAG)
		log_message("Exiting updateTorrentFile(): success");

	return true;
}

int main(){

	string fileName = "a.pdf";
	string peerIp = "10.0.0.5";
	string peerPort = "4000";

	bool flag = createTorrentFile(fileName,peerIp,peerPort);

	if(flag)
		cout<<"Create operation success\n";
	else
		cout<<"Create operation failed\n";

	peerIp = "30.0.0.5";
	peerPort = "5000";

	flag = updateTorrentFile(fileName,peerIp,peerPort);

	if(flag)
		cout<<"Update operation success\n";
	else
	cout<<"Update operation failed\n";

	peerIp = "130.0.0.5";
	peerPort = "2000";

	flag = updateTorrentFile(fileName,peerIp,peerPort);

	if(flag)
		cout<<"Update operation success\n";
	else
	cout<<"Update operation failed\n";

	return 0;
}