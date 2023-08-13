#include <stdbool.h>
#include <unistd.h>
#include <pthread.h>
#include "socketutil.h"

struct AcceptedSocket
{
    int acceptedSocketFD;
    struct sockaddr_in address;
    int error;
    bool acceptedSuccessfully;
};

struct AcceptedSocket *acceptIncomingConnection(int serverSocketFD);

void acceptNewConnectionAndReceiveAndPrintItsData(int serverSocketFD);

void receiveAndPrintIncomingData(int socketFD);

void startAcceptingIncomingConnections(int serverSocketFD);

void receiveAndPrintIncomingDataOnSeparateThread(struct AcceptedSocket *pSocket);

void sendReceivedMessageToTheOtherClients(char *buffer,int socketFD);

struct AcceptedSocket acceptedSockets[10];
int acceptedSocketsCount = 0;


void startAcceptingIncomingConnections(int serverSocketFD)
{
    while(1) {
        struct AcceptedSocket* clientSocket = acceptIncomingConnection(serverSocketFD);
        acceptedSockets[acceptedSocketsCount++] = *clientSocket;
        receiveAndPrintIncomingDataOnSeparateThread(clientSocket);
    }
}
void receiveAndPrintIncomingDataOnSeparateThread(struct AcceptedSocket *pSocket) {
    pthread_t  id;
    pthread_create(&id, NULL, (void *(*)(void *)) receiveAndPrintIncomingData, (void *) pSocket->acceptedSocketFD);
}
void receiveAndPrintIncomingData(int socketFD)
{
    char buffer[1024];
    while(1)
    {
        ssize_t amountReceived = recv(socketFD, buffer, 1024, 0);
        if(amountReceived > 0)
        {
            buffer[amountReceived] = 0;
            printf("Response was %s\n", buffer);
            sendReceivedMessageToTheOtherClients(buffer,socketFD);
        }
        if(amountReceived==0)
        {
            break;
        }
    }
    close(socketFD);
}

void sendReceivedMessageToTheOtherClients(char *buffer,int socketFD)
{
    for(int i = 0; i < acceptedSocketsCount; i++)
    {
        if(acceptedSockets[i].acceptedSocketFD != socketFD)
        {
            send(acceptedSockets[i].acceptedSocketFD, buffer, strlen(buffer),0);
        }
    }
}
struct AcceptedSocket * acceptIncomingConnection(int serverSocketFD) {
    struct sockaddr_in clientAddress;
    int clientAddressSize = sizeof (struct sockaddr_in);
    int clientSocketFD = accept(serverSocketFD, &clientAddress, &clientAddressSize);

    struct AcceptedSocket* acceptedSocket = malloc(sizeof (struct AcceptedSocket));
    acceptedSocket->address = clientAddress;
    acceptedSocket->acceptedSocketFD = clientSocketFD;
    acceptedSocket->acceptedSuccessfully = clientSocketFD>0;

    if(!acceptedSocket->acceptedSuccessfully) {
        acceptedSocket->error = clientSocketFD;
    }
    return acceptedSocket;
}
int main() {
    int serverSocketFD = createTCPIpv4Socket();
    struct sockaddr_in * serverAddress = createIPv4Address("", 2000);
    int result = bind(serverSocketFD, serverAddress, sizeof (*serverAddress));
    if(result == 0 ) {
        printf("socket was bound successfully\n");
    }else {
        printf("connect fail\n");
    }
    int listenResult = listen(serverSocketFD, 10);
    startAcceptingIncomingConnections(serverSocketFD);
    shutdown(serverSocketFD, SHUT_RDWR);
    return 0;
}