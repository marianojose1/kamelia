#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include "kamelia.h"

void relay_onion(int porta_local, int relay_id){
    printf("[+] Iniciando relay onion na porta %d (ID: %d)\n", porta_local, relay_id);
    
    unsigned char relay_priv[crypto_box_SECRETKEYBYTES];
    unsigned char relay_pub[crypto_box_PUBLICKEYBYTES];
    k_load_kx(relay_id, relay_pub, relay_priv);
    
    int sock = create_socket(porta_local);
    if(sock < 0){
        printf("[!] Erro ao criar socket!\n");
        return;
    }
    
    unsigned char buffer[MAX_PACOTE];
    struct sockaddr_in remetente;
    
    while(1){
        int bytes = receive_msg(sock, (char*)buffer, &remetente);
        if(bytes > 0){
            printf("[+] Recebido %d bytes\n", bytes);
            
            unsigned char dados_processados[MAX_PACOTE];
            int tam_processados;
            char proximo_ip[16];
            int proxima_porta;
            
            unsigned char cliente_pub[crypto_box_PUBLICKEYBYTES];
            unsigned char cliente_priv[crypto_box_SECRETKEYBYTES];
            k_load_cl_k(cliente_pub, cliente_priv);
            
            if(processar_pacote_onion(buffer, bytes, relay_priv, cliente_pub, 
                                      dados_processados, &tam_processados, 
                                      proximo_ip, &proxima_porta) == 0){
                
                struct sockaddr_in destino;
                memset(&destino, 0, sizeof(destino));
                destino.sin_family = AF_INET;
                destino.sin_port = htons(proxima_porta);
                inet_pton(AF_INET, proximo_ip, &destino.sin_addr);
                
                sendto(sock, dados_processados, tam_processados, 0, 
                       (struct sockaddr*)&destino, sizeof(destino));
                printf("[+] Reencaminhado para %s:%d\n", proximo_ip, proxima_porta);
            } else {
                printf("[!] Erro ao processar pacote\n");
            }
        }
    }
    close(sock);
}
