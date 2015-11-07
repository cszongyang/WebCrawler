//
//  main.cpp
//  WebCrawler
//
//  Created by zongyang li on 10/10/15.
//  Copyright © 2015 zongyang li. All rights reserved.
//

#include <stdio.h>
#include <string.h>
#include <queue>
#include <set>
#include <map>
#include "url.h"
#include <stdarg.h>
#include <errno.h>
#include <sys/socket.h>
#include <netdb.h>
#include <resolv.h>


#define MAXBUF  1024
void PANIC(char *msg);
#define PANIC(msg)  {perror(msg); abort();}

using namespace std;

typedef struct {
    url_t url;
    int depth;
} urlStruct;

typedef struct {
  //  char data[MAXBUF+1];
    char *data;
    url_t url;
    int threadID;
    int depth;
    int size;
} dataStruct;

//function declarations
dataStruct retrieveWebPage(url_t url, int currentDepth);

void parseWebPage(dataStruct& dataToParse);
void addUrlToFetchQueue(urlStruct& url);

queue<urlStruct> urlQueue;
queue<dataStruct> dataQueue;
map<std::string, int> prevSearched;


int depth = 0;

//numberInQueue is incremented when a url is added to the fetcher queue
//and decremented when a url is pulled off of the parser queue
int numberInQueue = 0;



int main(int argc, const char * argv[]) {
    // insert code here...
    url_t output_url;
    output_url = parse_single_URL(argv[1]);
    depth=atoi(argv[2]);
    urlStruct init;
    init.url=output_url;
    urlQueue.push(init);
    
    
    
    for (int i=0; i<10; i++) {
        while(!urlQueue.empty()){
            
            
            
            urlStruct nextUrl=urlQueue.front();
            urlQueue.pop();
            
            
            dataStruct temp1 = retrieveWebPage(nextUrl.url, nextUrl.depth);
            if (temp1.data) {
                dataQueue.push(temp1);
                
            }
        }
        
        if (!dataQueue.empty()) {
            std::cout<<"\n DATAQUEUE SIZE:"<<dataQueue.size()<<"\n";
            dataStruct data=dataQueue.front();
            dataQueue.pop();
            
            parseWebPage(data);
            std::cout<<"\n"<<urlQueue.size()<<"\n";
            
            numberInQueue--;
            
        }
    
        
    }
    
    
    return 0;
}





///////////////////////////////////////////////////////////////////////////////////////
// Retrieves data from a url
///////////////////////////////////////////////////////////////////////////////////////
dataStruct retrieveWebPage(url_t url, int currentDepth) {
    cout<<"currentDepth:  "<<currentDepth<<"\n";
    int size = 0;
    dataStruct temp1;
    int sockfd, bytes_read, total_bytes = 0;
    struct sockaddr_in dest;
    char buffer[MAXBUF];
    struct hostent *hostEntity;
    temp1.data=new char[1024];
    
    
    /*---Should probably check arguments here---*/
    if ( (sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0 )
        PANIC("Socket");
    
    /*---Initialize server address/port struct---*/
    bzero(&dest, sizeof(dest));
    dest.sin_family = AF_INET;
    dest.sin_port = htons(80); /*default HTTP Server port */
    
    
    hostEntity = gethostbyname(url.host.c_str());
    if( hostEntity == NULL) {
      //  printf("Failed to look up hostname. %s\n", url.host.c_str());
        //exit(1);
        return temp1;
    }
    memcpy(&dest.sin_addr.s_addr, hostEntity->h_addr, hostEntity->h_length);
    
    /*---Connect to server---*/
    if ( connect(sockfd, (struct sockaddr*)&dest, sizeof(dest)) != 0 )
        PANIC("Connect");
    
    snprintf(buffer, MAXBUF, "GET /%s HTTP/1.1\r\nHost: %s\r\n", url.file.c_str(), url.host.c_str());
    snprintf(buffer+strlen(buffer), MAXBUF, "Connection: close\r\n");
    snprintf(buffer+strlen(buffer), MAXBUF, "User-Agent: Mozilla/5.0\r\n");
    snprintf(buffer+strlen(buffer), MAXBUF, "Cache-Control: max-age=0\r\n");
    snprintf(buffer+strlen(buffer), MAXBUF, "Accept: text/*;q=0.3, text/html;q=0.7, text/html;level=1\r\n");
    snprintf(buffer+strlen(buffer), MAXBUF, "Accept-Language: en-US,en;q=0.8\r\n");
    snprintf(buffer+strlen(buffer), MAXBUF, "Accept-Charset: ISO-8859-1,utf-8;q=0.7,*;q=0.3\r\n");
    snprintf(buffer+strlen(buffer), MAXBUF, "\r\n");
    
    send(sockfd, buffer, sizeof(buffer), 0);
    
    printf("succesfully wrote the buffers, need to read now");
    /*---While there's data, read and print it---*/
    do {
        bzero(buffer, sizeof(buffer));
        bytes_read = recv(sockfd, buffer, sizeof(buffer), 0);
        
        if ( bytes_read > 0 ) {
         
            
            temp1.data = (char*) realloc (temp1.data, sizeof(buffer)+total_bytes);
            memcpy(temp1.data+total_bytes, buffer, sizeof(buffer));
            total_bytes += bytes_read;
            //            printf("%s", buffer);
        }
    } while ( bytes_read > 0 );
    
    temp1.depth = currentDepth;
    temp1.size = total_bytes;
    temp1.url.file.clear();
    temp1.url.file.append(url.file);
    temp1.url.host.clear();
    temp1.url.host.append(url.host);
    temp1.url.type.clear();
    temp1.url.type.append(url.type);
    
    
    
    /*---Clean up---*/
    close(sockfd);
    
    return temp1;
}



///////////////////////////////////////////////////////////////////////////////////////
// Adds a url to the fetcher queue
///////////////////////////////////////////////////////////////////////////////////////
void addUrlToFetchQueue(urlStruct& url) {
    int counter=0;
    std::string theUrl = url.url.host + "/" + url.url.file;
    map<std::string, int>::iterator iter = prevSearched.find(theUrl);
    
    
    
    if(iter == prevSearched.end() && url.depth <= depth) {
        
        prevSearched.insert(make_pair(theUrl, url.depth));
        numberInQueue++;
        urlQueue.push(url);
        
    }
    
}


///////////////////////////////////////////////////////////////////////////////////////
// Parses the links from a url
///////////////////////////////////////////////////////////////////////////////////////
void parseWebPage(dataStruct& dataToParse){
    set<url_t, lturl, std::allocator<url_t> > urlSet;
    
    parse_URLs(dataToParse.data, dataToParse.size, urlSet);
    
    
    
    int* depthOfSet = &dataToParse.depth;
    int actualDepth = *depthOfSet;
    
    set<url_t>::iterator dataPointer;
    for(dataPointer = urlSet.begin(); dataPointer != urlSet.end(); dataPointer++){
        urlStruct currentUrl;
        currentUrl.url = *dataPointer;
        currentUrl.depth = actualDepth + 1;
        
        if (currentUrl.url.host.length()!=0) {
            addUrlToFetchQueue(currentUrl);
            
        }
        
        
    }
    
}



