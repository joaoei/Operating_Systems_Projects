#include "socketclient.hpp"
#include <iostream>

SocketClient::SocketClient(std::unordered_set<std::string> &blacklist, int &id) {
    memset(&endereco, 0, sizeof(endereco));
    endereco.sin_family = AF_INET;
    endereco.sin_port = htons(PORTNUM);
    endereco.sin_addr.s_addr = inet_addr(IP); //mudar IP
 
    /*
     * Criando o Socket
     *
     * PARAM1: AF_INET ou AF_INET6 (IPV4 ou IPV6)
     * PARAM2: SOCK_STREAM ou SOCK_DGRAM
     * PARAM3: protocolo (IP, UDP, TCP, etc). Valor 0 escolhe automaticamente
    */
    socketId = socket(AF_INET, SOCK_STREAM, 0);
 
    //Verificar erros
    if (socketId == -1)
    {
        printf("Falha ao executar socket()\n");
        exit(EXIT_FAILURE);
    }
 
    //Conectando o socket cliente ao socket servidor
    if ( connect (socketId, (struct sockaddr *)&endereco, sizeof(struct sockaddr)) == -1 )
    {
        printf("Falha ao executar connect()\n");
        exit(EXIT_FAILURE);
    }
    printf ("Cliente conectado ao servidor\n");
    /*
     * 1. Mandar id
            - se não tiver id, recebe um id
     * 2. Receber blacklist size
     * 3. Se size != 0, receber blacklist.
     */
     
    // Sending client ID to server
    int sendID = send(socketId, &id, sizeof(id), 0);

    if(sendID == -1){
        printf("Falha ao enviar ID do cliente para o servidor!");
        exit(EXIT_FAILURE);
    }
    
    if(sendID != sizeof(id)){
        printf("Erro no envio do id!\n");
    }

    // If client has no ID (0), it must receive one from the server
    if(id == 0) {
        unsigned int myID;
        int recvID = recv(socketId, &myID, sizeof(myID), 0);
        
        if(recvID != sizeof(myID)){
            printf("Recebimento do ID não deu certo!");
        }
        id = myID;
    }
    
    // Client receives blacklist size
    unsigned int BLsize;
    int recvBLsize = recv(socketId, &BLsize, sizeof(BLsize), 0);
    if(recvBLsize != sizeof(BLsize))
        printf("Recebimento do tamanho da Blacklist não deu certo!");
    
    // If blacklist is not empty, update client's blacklist
    if(BLsize != 0) {
        char blacklist_char[BLsize][16];
        int recvBL = recv(socketId, &blacklist_char, sizeof(blacklist_char), 0);
        
        if(recvBL != sizeof(blacklist_char))
            printf("Recebimento da Blacklist não deu certo!");
        else
            for (int i = 0; i < BLsize; i++)
                blacklist.insert(std::string(blacklist_char[i]));
    }

    /*
    ConnectionInfo msg1;
    int byteslidos = recv(socketId, &msg1, sizeof(ConnectionInfo),0);
    
    if (byteslidos == -1)
    {
        printf("Falha ao executar recv()");
        exit(EXIT_FAILURE);
    }
    else if (byteslidos == 0)
    {
        printf("Cliente finalizou a conexão\n");
        exit(EXIT_SUCCESS);
    }
    
    int blacklist_size = msg1.blacklistSize;
    id = msg1.id;
    
    OrderPacket msg2;
    bool last_packet = false;
    int i = 0;
    
    while (!last_packet) {
        byteslidos = recv(socketId, &msg2, sizeof(OrderPacket),0);
        
        if(byteslidos == -1){
            printf("Falha ao executar recv()");
            continue;
        }
        
        if(msg2.operation == 2){
            std::string bl_item(ms2.processName);
            blacklist.insert(bl_item);
            i++;
        }
        
        last_packet = msg2.lastPacket;
    }
    
    if(blacklist_size != i){
        printf("Blacklist não foi recebida completamente.");
    }
    */
}

SocketClient::~SocketClient() {
    close(socketId);
}

void SocketClient::sendPacket(const Packet &pack) {
    printf("Enviando mensagem\n");
    //Enviar uma msg para o cliente que se conectou
    bytesenviados = send(socketId,&pack,sizeof(pack),0);
 
    if (bytesenviados == -1)
    {
        printf("Falha ao executar send()");
        exit(EXIT_FAILURE);
    }
}

//continuar
void SocketClient::recvServerResponse(std::unordered_set<std::string> &blacklist){    
    char morePacks;
    int recvMorepacks = recv(socketId, &morePacks, sizeof(morePacks), 0);
    
    if(morePacks == 1){
        std::cout << "Recebeu ordem do servidor!\n";
        OrderPacket msg;
        bool lastPacket = false;
        int byteslidos;
        while(!lastPacket) {
            byteslidos = recv(socketId, &msg, sizeof(OrderPacket), 0);
            
            if (byteslidos == -1)
            {
                printf("Falha ao receber mais pacotes");
                exit(EXIT_FAILURE);
            }
            else if (byteslidos == 0)
            {
                printf("Servidor finalizou a conexão\n");
                exit(EXIT_SUCCESS);
            }
            
            lastPacket = msg.lastPacket;
            
            if(msg.operation == 1) {
                kill(msg.pid, SIGTERM);
            } else if (msg.operation == 2) {
                std::string bl_item(msg.processName);
                blacklist.insert(bl_item); 
            } else if (msg.operation == 3) {
                std::string bl_item(msg.processName);
                blacklist.erase(bl_item);
            }
        }
        
    }
    
    
}