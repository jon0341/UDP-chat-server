#include <stdio.h>      /* for printf() and fprintf() */
#include <sys/socket.h> /* for socket() and bind() */
#include <arpa/inet.h>  /* for sockaddr_in and inet_ntoa() */
#include <stdlib.h>     /* for atoi() and exit() */
#include <string.h>     /* for memset() */
#include <unistd.h>     /* for close() */

void DieWithError(char *errorMessage);  /* External error handling function */

int main(int argc, char *argv[]) {
    int sock;                        /* Socket */
    struct sockaddr_in echoServAddr; /* Local address */
    struct sockaddr_in echoClntAddr; /* Client address */
    unsigned int cliAddrLen;         /* Length of incoming message */
    unsigned short echoServPort;     /* Server port */
    int recvMsgSize;                 /* Size of received message */
    int userCount = 0;
    char directoryString[100];

    enum requestMsgType {
        loginRequest,
        whoRequest,
        talkRequest,
        logoutRequest
    };

    enum responseMsgType {
        loginResponse,
        whoResponse,
        talkResponse,
        logoutResponse
    };

    typedef struct message {
        int msgType;
        char msg[100];
        unsigned int numField;
    } message;

    typedef struct user{
        struct sockaddr_in userAddress;
        char userID[5];
        unsigned int tcpPort;
    } user;

    //create a zeroed user struct, populate user directory array with it
    user zeroedUser;
    memset(&zeroedUser, 0, sizeof(user));
    user activeUserData[20];

    for (int i = 0; i < 20; i++) {
        activeUserData[i] = zeroedUser;
    }

    /* Test for correct number of parameters */
    if (argc != 2) {
        fprintf(stderr, "Usage:  %s <UDP SERVER PORT>\n", argv[0]);
        exit(1);
    }

    echoServPort = atoi(argv[1]);  /* First arg:  local port */

    /* Create socket for sending/receiving datagrams */
    if ((sock = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0)
        DieWithError("socket() failed");

    /* Construct local address structure */
    memset(&echoServAddr, 0, sizeof(echoServAddr));   /* Zero out structure */
    echoServAddr.sin_family = AF_INET;                /* Internet address family */
    echoServAddr.sin_addr.s_addr = htonl(INADDR_ANY); /* Any incoming interface */
    echoServAddr.sin_port = htons(echoServPort);      /* Local port */

    /* Bind to the local address */
    if (bind(sock, (struct sockaddr *) &echoServAddr, sizeof(echoServAddr)) < 0)
        DieWithError("bind() failed");

    for (;;) /* Run forever */
    {
        //clear receiving Buffer between messages
        message incomingReq;
        message response;
        memset(&response, 0, sizeof(response));
        memset(&incomingReq, 0, sizeof(incomingReq));

        /* Set the size  the in-out parameter */
        cliAddrLen = sizeof(echoClntAddr);

        /* Block until receive message from a client */
        if ((recvMsgSize = recvfrom(sock, &incomingReq, sizeof(incomingReq), 0,
                                    (struct sockaddr *) &echoClntAddr, &cliAddrLen)) < 0)
            DieWithError("recvfrom() failed");
        else {
            /*begin processing user msg*/
            printf("Incoming request from client: %s\n", inet_ntoa(echoClntAddr.sin_addr));
        }

        if (incomingReq.msgType == loginRequest) {
            printf("-------------\\0/-LOGIN REQUEST-\\0/---------------\n");

            //notification of user logging in
            printf("client requests ID %s\n", incomingReq.msg);
            printf("new user tcp port number: %d\n", activeUserData[userCount].tcpPort);

            //add the user to the array, display confirmation message they have been added
            //add user to first open spot in user array
            for(int i = 0; i < 20; i++) {
                if(activeUserData[i].userID[0] == '\0') {
                    activeUserData[i].userAddress = echoClntAddr;
                    strcpy(activeUserData[i].userID, incomingReq.msg);
                    activeUserData[i].tcpPort = incomingReq.numField;
                    break;
                }
            }
            printf("new user's address/ID/tcp Port added to directory:  "
                   "%s / %s / %d \n", inet_ntoa(activeUserData[userCount].userAddress.sin_addr),
                   activeUserData[userCount].userID, activeUserData[userCount].tcpPort);

            userCount++;
            char userToAdd[6];
            strcpy(userToAdd, activeUserData[userCount-1].userID);

            //construct acknowledgement/login success message, send to client
            message loginAck = {
                    loginResponse, "FROM SERVER: you are logged in!", .numField = 0
            };

            /* Send response to client */
            if (sendto(sock, &loginAck, sizeof(loginAck), 0,
                       (struct sockaddr *) &echoClntAddr, sizeof(echoClntAddr)) != sizeof(loginAck))
                DieWithError("sendto() sent a different number of bytes than expected");
            else {
                printf("user %s logged in, acknowledgement sent\n\n", incomingReq.msg);
            }
        }

        //client sent request for list of users
        if (incomingReq.msgType == whoRequest) {
            printf("--------------------\\0/-WHO REQUEST-\\0/--------------------\n");

            //construct response, memset it, assign its values
            message whoReqResponse;
            memset(&whoReqResponse, 0, sizeof(whoReqResponse));
                    whoReqResponse.msgType = whoResponse;
                    whoReqResponse.numField = 0;

            for(int i = 0; i < 20; i++) {
                strcat((char*)whoReqResponse.msg, activeUserData[i].userID);
                strcat((char*)whoReqResponse.msg, " ");
            }

            /* Send directory response back to client */
            if (sendto(sock, &whoReqResponse, sizeof(whoReqResponse), 0,
                       (struct sockaddr *) &echoClntAddr, sizeof(echoClntAddr)) == sizeof(whoReqResponse))
                printf("sent directory: %s\n\n", whoReqResponse.msg);
            else DieWithError("sendto() sent a different number of bytes than expected");
        }

        //client sent request to talk to specific user
        if (incomingReq.msgType == talkRequest) {

            printf("------------------------\\0/-TALK REQUEST-\\0/---------------------------------\n");
            printf("user requests connection to: %s\n", incomingReq.msg);

            //search users, if found then build string with their data to send
            for (int i = 0; i < 20; i++) {
                if (strcmp(activeUserData[i].userID, incomingReq.msg) == 0) {
                    printf("user %s's ip address: %s\n", incomingReq.msg,
                           inet_ntop(AF_INET, &(activeUserData[i].userAddress.sin_addr), response.msg, INET_ADDRSTRLEN));
                    printf("user %s's tcp port: %u\n", incomingReq.msg, activeUserData[i].tcpPort);
                    response.numField = activeUserData[i].tcpPort;
                    response.msgType = talkResponse;
                }
            }

            /* Send response to client */
            if (sendto(sock, &response, sizeof(response), 0,
                       (struct sockaddr *) &echoClntAddr, sizeof(echoClntAddr)) != sizeof(response))
                DieWithError("sendto() sent a different number of bytes than expected");
            else
                printf("Sent user %s ip address and tcp port: %s %u\n", incomingReq.msg, response.msg, response.numField);
        }

        if(incomingReq.msgType == logoutRequest) {
            printf("-------------\\0/-LOGOUT REQUEST-\\0/-----------------------\n");
            printf("user %s requests logout\n", incomingReq.msg);
            for(int i = 0; i < 20; i++) {
                if(strcmp(activeUserData[i].userID, incomingReq.msg) == 0) {
                    printf("searching directory for user: %s  found user: %s\n", activeUserData[i].userID, incomingReq.msg);
                    memset(&activeUserData[i], 0, sizeof(user));
                    response.msgType = logoutResponse;
                    strcpy(response.msg, " ");
                    break;
                }
            }
            if (sendto(sock, &response, sizeof(response), 0,
                       (struct sockaddr *) &echoClntAddr, sizeof(echoClntAddr)) != sizeof(response))
                DieWithError("sendto() sent a different number of bytes than expected");
            else
                printf("User %s successfully logged out\n\n", incomingReq.msg);
        }
    }
}
    /* NOT REACHED */
