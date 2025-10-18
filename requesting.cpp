#include <iostream>
#include <string>
#include <curl/curl.h>
size_t WriteCallback(void* contents,size_t size,size_t nmemb,std::string* output){
    output->append((char*)contents,size*nmbemb);
    return size*nmbemb;
}
int main (){
    Curl* curl;
    Curlcode=res;
    std::string response;
    curl=curl_easy_init();
    if (curl){
  curl_easy_setopt(curl, CURLOPT_URL, "https://jsonplaceholder.typicode.com/posts/1");
    curl_easy_sstopt(curl,CURLOPT_WRITEFUCNTION,WriteCallback);
    curl_easy_setopt(curl,CURLOPT_URL_WRITEDATA,&response);
    res=curl_easy_perform(curl);
    curl_easy_perform(curl);
    if (res==CURLE_OK){
        std::cout << "Response:\n" << response << std::endl;
    }
    return 0;
    }
}
