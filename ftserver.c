/*
 Name : Alexandra Dohm
 Program :  Server Side TCP FTP transfer, Project 2
 Description : This chat server implements TCP FTP file transfer with commands -l and -g
 Course : CS 372
 Last Modified : 01.12.19
 Resources :
 - My submission for project 1 in this class
 - Beej’s Guide to Network Programming Using Internet Sockets
 - https://www.geeksforgeeks.org/socket-programming-cc/
 */

#include <unistd.h>
#include <stdio.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <string.h>
#include <dirent.h>
#include <arpa/inet.h>
#include <netdb.h>

/*
 Verify user has included correct number of inputs in program
 Pre Condition: arg count
 Post Condition: verification that correct number of args included
 */
void checkInputs(int argc) {
    if (argc != 2) {
        fprintf(stderr, "Wrong format. Include server port number and server hostname. Exiting.\n");
        exit(1);
    }
}

/*
 If socket is hogging the port, allow server to reuse the port
 Pre Condition: sockfd already generated
 Post Condition: port is freed for use
 */
void reuseAddress(int sockfd) {
    int yes = 1;
    
    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1) {
        perror("setsockopt");
        exit(1);
    }
}

/*
 Create Address Endpoint using client port and ip address
 Pre Condition: Valid port number and ip address
 Post Condition: addrinfo struct for socket creation. pointer returned to addrinfo for socket creation
 */
struct addrinfo * createAddress(const char * port, char * ip) {
    
    int status;
    struct addrinfo hints;
    struct addrinfo * res;
    
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    
    //error output
    if((status = getaddrinfo(ip, port, &hints, &res)) != 0){
        fprintf(stderr, "getaddrinfo error: %s\n", gai_strerror(status));
        exit(1);
    }
    
    return res;
}

/*
 Create Address Endpoint using client port
 Pre Condition: Valid port number
 Post Condition: addrinfo struct for socket creation. pointer returned to addrinfo for socket creation
 */
struct addrinfo * createServerAddress(const char * port) {
    
    int status;
    struct addrinfo hints;
    struct addrinfo * res;
    
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;
    
    //error output
    if((status = getaddrinfo(NULL, port, &hints, &res)) != 0){
        fprintf(stderr, "getaddrinfo error: %s\n", gai_strerror(status));
        exit(1);
    }
    
    return res;
}

/*
 Initiate connection of socket
 Pre Condition: file descriptor sockfd and address already initialized
 Post Condition: Socket bound to port and address, ready to listen for connection
 */
void connectSocketIP(int sockfd, struct addrinfo * res) {
    if (connect(sockfd, res->ai_addr, res->ai_addrlen) == -1){
        fprintf(stderr, "Error in connection.\n");
        exit(1);
    }
}

/*
 Create socket on server
 Pre Condition: res - initialied linked list to addrinfo struct
 Post Condition: socket number created
 */
int createSocket(struct addrinfo * res) {
    int sockfd;
    if ((sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol)) == -1) {
        fprintf(stderr, "Socket Creation Error\n");
        exit(1);
    }
    
    //see if socket is hogging port, if so, free
    reuseAddress(sockfd);
    return sockfd;
}


/*
 Binds the socket to the address and port number specified in servaddr
 Pre Condition: socket fd and address
 Post Condition: socket bound to port and address, ready to listen for connection
 */
void bindSocket(int sockfd, struct addrinfo *res) {
    if (bind(sockfd, res->ai_addr, res->ai_addrlen) == -1) {
        close(sockfd);
        perror("bind");
        exit(1);
    }
}

/*
 Start server to listen for connections from clients
 Pre Condition: socket bound to address and port number
 Post Condition: server is started and waiting for connections
 */
void listenForConnection(int sockfd) {
    // listen - 5 pending connections queue will hold
    if (listen(sockfd, 5) == -1) {
        perror("listen");
        exit(1);
    }
}

/*
 Send file to client
 Pre Condition: server, file, client ip address, data port all initialized
 Post Condition: -
 */
void getFile(char* dataPort, char* ip, char* fileName, char* serverName) {
    printf("Connection from %s\n", serverName);
    printf("File \"%s\" requested on port %s\n", fileName, dataPort);
    
    sleep(2); // allow client to open port for connection
    char *fileError = "File Not Found";
    
    //create data socket
    struct addrinfo * servaddr = createAddress(dataPort, ip);
    int data_socket = createSocket(servaddr);
    connectSocketIP(data_socket, servaddr);
    
    printf("Sending \"%s\" to port %s on client\n", fileName, dataPort);
    char fileBuffer[1024];
    memset(fileBuffer, 0, sizeof(fileBuffer));
    
    FILE *fd  = fopen(fileName, "r");
    if (fd == NULL) {
        fprintf(stderr, "Open file error. Sending error message to client.\n");
        send(data_socket, fileError, strlen(fileError), 0);
        exit(1);
    } else {
        
        //Infinite loop for large txt files
        while(1) {
            
            //Read data into buffer
            long bytes = fread(fileBuffer, sizeof(char), sizeof(fileBuffer), fd);
            if (bytes == 0) {
                break; //done reading file
            } else if (bytes < 0) {
                fprintf(stderr, "Error reading file\n");
                return;
            }
            
            //Send data to client
            long writeBytes = send(data_socket, fileBuffer, strlen(fileBuffer), 0);
            if (writeBytes < 0) {
                fprintf(stderr, "Error writing to socket\n");
                return;
            }
            //Clear buffer for next file transfer
            memset(fileBuffer, 0, sizeof(fileBuffer));
            
        }
        
        memset(fileBuffer, 0, sizeof(fileBuffer));
        strcpy(fileBuffer, "** done **");
        send(data_socket, fileBuffer, strlen(fileBuffer),0);
        printf("Data sent, closing data socket\n");
    }
    
    close(data_socket);
    freeaddrinfo(servaddr);
}

/*
 Send directory listing to client
 Pre Condition: server, client ip address, data port all initialized
 Post Condition: -
 */
void listFile(char* dataPort, char* ip, char * serverName) {
    printf("Connection from %s\n", serverName);
    printf("List directory requested on port %s\n", dataPort);

    sleep(2); // allow client to open port for connection
    
    //create data socket
    struct addrinfo * servaddr = createAddress(dataPort, ip);
    int data_socket = createSocket(servaddr);
    connectSocketIP(data_socket, servaddr);
    printf("Sending directory contents to %s\n", dataPort);
    
    char fileName[100];
    memset(fileName, 0, sizeof(fileName));
    
    DIR *d;
    struct dirent *dir;
    d = opendir(".");
    if (d) {
        //for each txt file, send to client
        while ((dir = readdir(d)) != NULL) {
            if(strstr(dir->d_name, ".txt") != NULL && dir->d_type == DT_REG) {
                strcpy(fileName, dir->d_name);
                fileName[strlen(fileName)] = '\n';
    
                send(data_socket, fileName, strlen(fileName),0);
                memset(fileName, 0, sizeof(fileName));
            }
        }
        closedir(d);
    }
    
    // close socket and free address information
    printf("Directory listing sent, closing data socket\n");
    close(data_socket);
    freeaddrinfo(servaddr);
}

/*
 Decide whether client wants to list files or send files
 Parses client input and stores in corresponding variables
 Pre Condition: input string received from client
 Post Condition: initiates listing files or transferring files
 */
void decideAction(char *buffer, char* ip) {
    char delim[] = " ";
    char *ptr = strtok(buffer, delim);
    
    char dataPort[10];
    char fileName[100];
    char serverName[20];
    memset(dataPort, 0, sizeof(dataPort));
    memset(fileName, 0, sizeof(fileName)); //check if file exists here, return to main
    memset(serverName, 0, sizeof(serverName));
    
    if (strcmp(ptr, "-g") == 0) {
        strcpy(fileName, strtok(NULL , delim));
        strcpy(dataPort, strtok(NULL , delim));
        strcpy(serverName, strtok(NULL , delim));
        getFile(dataPort, ip, fileName, serverName);

    } else if (strcmp(ptr, "-l") == 0) {
        strcpy(dataPort, strtok(NULL , delim));
        strcpy(serverName, strtok(NULL , delim));
        listFile(dataPort, ip, serverName);
    }
}

/*
 Given new socket number, find client ip address for data socket
 Pre Condition: new socket already created
 Post Condition: return ip address of client
 */
char* getClientIP(int newSocket) {
    struct sockaddr_in addr;
    char *clientIP = malloc(20 * sizeof(char));
    
    socklen_t addr_size = sizeof(struct sockaddr_in);
    getpeername(newSocket, (struct sockaddr *)&addr, &addr_size);

    strcpy(clientIP, inet_ntoa(addr.sin_addr));
    return clientIP;
}


/*
 Pre Condition: port number passed when function is callsed
 Post Condition: server listens for client connections
 */
int main(int argc, char const *argv[])
{
    long valread;
    int new_socket;
    char buffer[1024] = {0};
    
    //verify user inputs correct number of inputs
    checkInputs(argc);
    printf("Server open on %s\n", argv[1]);
    
    //create address and socket
    struct addrinfo * servaddr = createServerAddress(argv[1]);
    int addrlen = sizeof(servaddr);
    int sockfd = createSocket(servaddr);

    //bind socket & address and listen for client connection
    bindSocket(sockfd, servaddr);
    listenForConnection(sockfd);

    while (1) {
        //accept incoming connection
        if ((new_socket = accept(sockfd, (struct sockaddr *)&servaddr, (socklen_t*)&addrlen)) < 0) {
            perror("accept");
            exit(EXIT_FAILURE);
        }
        
        char* clientIP = getClientIP(new_socket);
        
        //take data from client, decide action
        valread = read(new_socket, buffer, 1024);
        decideAction(buffer, clientIP);
        memset(buffer, 0, sizeof(buffer)); //clear buffer
        
        free(clientIP);
        close(new_socket);
    }
    return 0;
}
