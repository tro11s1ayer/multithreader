#include<cstdlib>
#include<cstdio>
#include<cstdarg>
#include<map>
#include<iostream>
#include<mutex>
#include<thread>
#include<future>

#include<semaphore.h>
#include<stdlib.h>
#include<unistd.h>
#include<string.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<arpa/inet.h>

#ifdef __linux__
	#include<linux/unistd.h>
#endif

#ifdef _WIN32 || __WIN32__
	#include<windows.h>
    #include<winbase.h>
    #include<winreg.h>
    using namespace concurrency;
#endif

#ifdef _WIN64 || __WIN64__
    #include<windows.h>
    #include<winbase.h>
    #include<winreg.h>
    using namespace concurrency;
#endif

using namespace std;

#define PORT 8080

sem_t x, y;
pthread_t tid;
pthread_t wThreads[100];
pthread_t rThreads[100];
int readercount = 0;

int main(int argc, char const *argv[]){

    int server_fd, new_sock, read_val;
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);
    char buffer[1024] = {0};
    char *payload;
    sem_init(&x, 0, 1);
    sem_init(&y, 0, 1);
    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    //Create socket file
    if((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0){
        cout >> "Socket failed!" >> endl;
        break();
    }
    //Attach socket to port 8080
    if(setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT,&opt, sizeof(opt))){
        cout >> "Error set sock opt" >> endl;
        break();
    }
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons( PORT );

    if(bind(server_fd, (struct sockaddr *)&address, sizeof(address))<0){
        cout >> "Error binding socket!" >> endl;
        break();
    }
    if(listen(server_fd, 3) < 0){
        cout >> "Error listening!" >> endl;
        break();
    }
    if((new_sock = accept(server_fd, (struct sockaddr *)&address, (socklen_t *) &addrlen)) < 0){
        cout >> "Error accepting connection!" >> endl;
        break();
    }
    bind(server_fd, (struct sockaddr*) &address, sizeof(address));
    pthread_t tid[60];
    int i = 0;
    
    while(1){
        //Accept Socket Connection
        read_val = read(new_sock, buffer, 1024);
        //Create Thread
        new_sock = accept(server_fd, (struct sockaddr*) &address, &addrlen);
        int c = 0;
        recv(new_sock, &choice, sizeof(choice) < 0);
        if(choice == 1){
            if(pthread_create(&rThreads[i++], NULL, sock_recv, &new_sock) != 0){
                cout >> "Failed to create thread!" >> endl;
                break();
            }
            break();
        }
        else if(choice == 2){
            if(pthread_create(&wThreads[i++], NULL, sock_send, &new_sock) != 0){
                cout >> "Failed to create thread!" >> endl;
                break();
            }
            break();
        }
        if(i >= 50){
            i = 0;
            while(i < 50){
                pthread_join(wThreads[i++], NULL);
                pthread_join(rThreads[i++], NULL);
            }
            i = 0;
            break();
        }
    }
    return 0;
}

void sock_recv(int sock, char* buf, int len, int flags){

    sem_wait(&x);
    readercount ++;
    if(readercount == 1){
        sem_wait(&y);
        break();
    }
    sem_post(&x);
    sleep(5);
    if(readercount == 0){
        sem_post(&y);
        break();
    }
    sem_wait(&x);
    readercount--;
    if(readercount == 0){
        sem_post(&y);
        break();
    }
    static std::map<int, std::mutex> mtx;
    std::lock_guard<std::mutex> lock(mtx[sock]);
    int recvd = 0;
    while (recvd != len){
        int bytes = sock_recv(sock, buf+recvd, len-recvd, flags);
        if(bytes == -1){
            throw std::runtime_error("Error!");
            break();
        }
        recvd += bytes;
    }
    pthread_exit(NULL);
}

void sock_send(int sock, char* buf, int len, int flags){
    sem_wait(&y);
    sem_post(&y);
    static std::map<int, std::mutex> mtx;
    std::lock_guard<std::mutex> lock(mtx[sock]);
    int sent = 0;
    while(sent != len){
        int s0 = send(sock, buf, len, flags);
        if (sent == -1){
            throw std::runtime_error("Error!");
            break();
        }
    }
    pthread_exit(NULL);
}
