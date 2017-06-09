#include "socketmanager.h"

#include <cstdio>       //printf
#include <cstring>      //memset
#include <cstdlib>      //exit
#include <netinet/in.h> //htons
#include <arpa/inet.h>  //inet_addr
#include <sys/socket.h> //socket
#include <unistd.h>     //close
#include <thread>
#include <QDebug>

#define PORTNUM 11111
#define IP "192.168.7.1"

SocketManager::SocketManager(const std::vector<Trem*> &_trens) : trens(_trens), trainsEnable(false) {
    threadTrem = std::thread(&SocketManager::socketListener, this);
}

SocketManager::~SocketManager() {
    threadTrem.detach();
}

// Futuramente receberá um struct informando qual trem deverá ser modificado e qual a nova velocidade
void SocketManager::socketListener() {
    //variáveis do servidor
    struct sockaddr_in endereco;
    int socketId;

    //variáveis relacionadas com as conexões clientes
    struct sockaddr_in sender;
    socklen_t sendersize = sizeof(sender);
    int conexaoClienteId;

    Packet pacote;
    int byteslidos = 1;

    /*
     * Configurações do endereço
    */
    memset(&endereco, 0, sizeof(endereco));
    endereco.sin_family = AF_INET;
    endereco.sin_port = htons(PORTNUM);
    endereco.sin_addr.s_addr = inet_addr(IP);

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
        qDebug() << "Falha ao executar socket()\n";
        exit(EXIT_FAILURE);
    }

    //Conectando o socket a uma porta. Executado apenas no lado servidor
    if ( bind (socketId, (struct sockaddr *)&endereco, sizeof(struct sockaddr)) == -1 )
    {
        qDebug() << "Falha ao executar bind()\n";
        exit(EXIT_FAILURE);
    }

    //Habilitando o servidor a receber conexoes do cliente
    if ( listen( socketId, 10 ) == -1) {
        qDebug() << "Falha ao executar listen()\n";
        exit(EXIT_FAILURE);
    }

    //servidor ficar em um loop infinito
    while(true) {
        qDebug() << "Servidor: esperando conexões clientes\n";

        //receber uma msg do cliente
        qDebug() << "Servidor vai ficar esperando uma mensagem\n";

        conexaoClienteId = accept(socketId, (struct sockaddr *) &sender, &sendersize);

        if (conexaoClienteId == -1) {
            printf("Falha ao executar accept()");
            exit(EXIT_FAILURE);

        }

        while (true) {
            byteslidos = recv(conexaoClienteId, &pacote, sizeof(pacote), 0);
            if (byteslidos <= 0) break;
            qDebug() << byteslidos << "\n";
            qDebug() << "Mensagem recebida: " << pacote.toggle << " " <<pacote.target << " " << pacote.changeSpeed << "\n";
            modifyTrain(pacote);
        }

        close(conexaoClienteId);
    }
}

void SocketManager::modifyTrain(Packet pac) {
//    qDebug() << trens.size();

    if (pac.toggle) {
        if (pac.target >= 0) {
            trens[pac.target]->setEnable(!trens[pac.target]->getEnable());
        } else {
            trainsEnable = !trainsEnable;
            for (auto &ptr : trens)
                ptr->setEnable(trainsEnable);
        }
    } else {
        int speed = pac.changeSpeed/4 + 16;
        trens[pac.target]->setVelocidade(speed);
    }
}
