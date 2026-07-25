#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include "kamelia.h"

// Chave pública fixa do cliente (carregada do arquivo)
static unsigned char chave_publica_cliente_fixa[crypto_box_PUBLICKEYBYTES];
static int chave_cliente_carregada = 0;

void carregar_chave_publica_cliente_server() {
    if (chave_cliente_carregada) return;
    
    FILE* fp = fopen("keys_client/PUBLIC_KEY.kam", "rb");
    if (fp) {
        fread(chave_publica_cliente_fixa, 1, crypto_box_PUBLICKEYBYTES, fp);
        fclose(fp);
        printf("[+] Chave pública do cliente carregada!\n");
        chave_cliente_carregada = 1;
    } else {
        printf("[!] Chave pública do cliente NÃO encontrada! Rode o cliente primeiro.\n");
        chave_cliente_carregada = 1;
    }
}

void server_onion(int porta){
    printf("[+] Servidor onion rodando na porta %d\n", porta);
    
    // Carrega chave pública fixa do cliente
    carregar_chave_publica_cliente_server();
    
    // Gera chaves do servidor
    unsigned char server_priv[crypto_box_SECRETKEYBYTES];
    unsigned char server_pub[crypto_box_PUBLICKEYBYTES];
    crypto_box_keypair(server_pub, server_priv);
    
    int sock = create_socket(porta);
    if(sock < 0){
        printf("[!] Erro ao criar socket!\n");
        return;
    }
    
    unsigned char buffer[MAX_PACOTE];
    struct sockaddr_in remetente;
    
    while(1){
        int bytes = receive_msg(sock, (char*)buffer, &remetente);
        if(bytes > 0){
            buffer[bytes] = '\0';
            printf("[+] Recebido: %s\n", buffer);
            
            if(strcmp((char*)buffer, "PING") == 0){
                char resposta[] = "PONG";
                unsigned char pacote[MAX_PACOTE];
                int tamanho;
                if(cifrar_pacote((unsigned char*)resposta, strlen(resposta) + 1, 
                                 chave_publica_cliente_fixa, server_priv, 
                                 pacote, &tamanho) == 0){
                    sendto(sock, pacote, tamanho, 0, 
                           (struct sockaddr*)&remetente, sizeof(remetente));
                    printf("[+] PONG enviado (cifrado)\n");
                }
            }
        }
    }
    close(sock);
}
