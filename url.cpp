#include "url.h"
#include <set>

url_t
parse_single_URL(std::string site) {
    url_t output_url;
    std::string type, hostName;
    size_t firstColon, hostEnd, startPos, tmpPos, fileEnd;
    firstColon = site.find(":");
    if(firstColon > 6 || firstColon == std::string::npos) {
        // This is a malformed URL...
        startPos = 0;
    } else {
        startPos = firstColon + 3;
    }
    type = site.substr(0, firstColon);
    /* parse host */
    tmpPos = site.find(":", startPos);
    if(tmpPos == std::string::npos) {
        tmpPos = site.find("/", startPos);
    }
    hostEnd = (tmpPos == std::string::npos)?tmpPos:tmpPos - startPos;
    hostName = site.substr(startPos,hostEnd);
    output_url.type = type;
    output_url.host = hostName;

    /* parse file */
    if(hostEnd != std::string::npos) {
        startPos = tmpPos;
        tmpPos = site.find("?", startPos);
        if(tmpPos == std::string::npos) {
            fileEnd = tmpPos;
        } else {
            fileEnd = tmpPos - startPos;
        }
        output_url.file = site.substr(startPos+1, fileEnd);
    }
    //printf("host: %s file: %s\n", output_url.host.c_str(), output_url.file.c_str());
    return output_url;
}


/* 
 * parse_URLs(buf, size, urls);
 *
 * will take a buffer (HTML) and scan through for all URLs.
 *
 * where buf is the buffer you read from the web server, the size is the
 * size of the buffer and urls is a set containing url_t structs (see
 * url.h).
 */



void
parse_URLs (char *buffer,int size,std::set<url_t, lturl, std::allocator<url_t> > &urlSet) {
    
    
    std::set<url_t, lturl, std::allocator<url_t> > tempSet;
    
    size_t startPos, endPos, urlStart;
    std::string buf, temp_string;
    url_t temp;
    const char * protocolStr[] = { "http://", "https://", "ftp://", "telnet://" };
    const char * linkStart[] = { "href=\"", "HREF=\"", "href =\"", "HREF =\"" };
    
    
    for(int n=0;n<HREF_MAX;n++) {
        for(int i=0;i<PROTOCOL_MAX;i++) {
            buf = buf.assign(buffer, size);
            
            
            startPos = buf.find(linkStart[n]);
            
            
            if(startPos != std::string::npos) {
                urlStart = buf.find(protocolStr[i], startPos);
            } else {
                urlStart = std::string::npos;
            }
            while(urlStart != std::string::npos) {
                
                
                
                
                const char* errstr = NULL;
                int         erroff = 0;
                //pcre* regex = pcre_compile("axb", PCRE_CASELESS|PCRE_UTF8, &errstr, &erroff, NULL);

                pcre* regex = pcre_compile("href[\\s]*=[\\s]*\\\".*?\"", 0, &errstr, &erroff, NULL);
                
                
                int ovector[10];
                int limit = 10;
                int rc = pcre_exec(regex, NULL, buf.data(), buf.size(), 0, 0, ovector, limit);
                
                int index = 0;
                
                if(rc>0){
                    const char* result = NULL;
                    if (pcre_get_substring(buf.data(), ovector, rc, 0, &result) >= 0) {

                        std::string s=result;
                        
                        if (s.length()>8) {
                            if((s.at(6)=='h')&&(s.at(7)=='t')){
                                temp_string=s.substr(6,s.length()-7);
                                
                            }
                        }
                    }
                }
                
                
                

                endPos = buf.find('.com', urlStart);
                
             
                
                if((endPos != std::string::npos)&&(endPos-urlStart)) {
                 
                    
                    temp = parse_single_URL(temp_string);
                    
                    
                    tempSet.insert(temp);
                    std::cout<<temp.host<<"\n";
                    
                    buf = buf.assign(buf, endPos+1, buf.length()-endPos);
                    startPos = buf.find(linkStart[n]);
                    
                    if(startPos != std::string::npos) {
                        urlStart = buf.find('\"', startPos);
                        
                        
                    } else {
                        urlStart = std::string::npos;
                    }
                } else {
                    urlStart = std::string::npos;
                }
            }
            
            
        }}
    urlSet = tempSet;
    return;
}

std::string
inputSite(const char *argument) {
    std::string blah;
    blah.assign(argument);
    return blah;
}
