#include <sodium.h>
#include <string.h>
#include <stdio.h>
#include <arpa/inet.h>
#include <unistd.h>
#include "kamelia.h"
#include <stddef.h>

int montar_pacote_onion(const char *mensagem, TabelaDHT *tabela, const unsigned char *chave_privada_cliente, unsigned char *pacote_final, int *tamanho_final){
    if(tabela->total < 3){
        printf("[!] Erro: precisa de pelo menos 3 relays na tabela!\n");
        return -1;
    }
    EntradaRelay *r1 = &tabela->relays[0];
    EntradaRelay *r2 = &tabela->relays[1];
    EntradaRelay *r3 = &tabela->relays[2];
    
    printf("[+] Montando pacote onion:\n");
    printf("    Relay 1: %s:%d\n", r1->ip, r1->porta);
    printf("    Relay 2: %s:%d\n", r2->ip, r2->porta);
    printf("    Relay 3: %s:%d\n", r3->ip, r3->porta);
    
    // ============================================
    // CAMADA 3 (Relay 3 -> Servidor)
    // ============================================
    PacoteOnion p3;
    memset(&p3, 0, sizeof(p3));
    strcpy(p3.proximo_ip, "127.0.0.1");
    p3.proxima_porta = PORTA_PADRAO;
    memcpy(p3.dados, mensagem, strlen(mensagem) + 1);
    p3.tamanho_dados = strlen(mensagem) + 1;
    
    unsigned char cifra3[MAX_PACOTE];
    int tam_cifra3;
    int tam_real_p3 = offsetof(PacoteOnion, dados) + p3.tamanho_dados;
    if(cifrar_pacote((unsigned char*)&p3, tam_real_p3, r3->chave_publica, chave_privada_cliente, cifra3, &tam_cifra3) != 0){
        printf("[!] Erro ao cifrar camada 3!\n");
        return -1;
    }
    
    // ============================================
    // CAMADA 2 (Relay 2 -> Relay 3)
    // ============================================
    PacoteOnion p2;
    memset(&p2, 0, sizeof(p2));
    strcpy(p2.proximo_ip, r3->ip);
    p2.proxima_porta = r3->porta;
    memcpy(p2.dados, cifra3, tam_cifra3);
    p2.tamanho_dados = tam_cifra3;
    
    unsigned char cifra2[MAX_PACOTE];
    int tam_cifra2;
    int tam_real_p2 = offsetof(PacoteOnion, dados) + p2.tamanho_dados;
    if(cifrar_pacote((unsigned char*)&p2, tam_real_p2, r2->chave_publica, chave_privada_cliente, cifra2, &tam_cifra2) != 0){
        printf("[!] Erro ao cifrar camada 2!\n");
        return -1;
    }
    
    // ============================================
    // CAMADA 1 (Relay 1 -> Relay 2)
    // ============================================
    PacoteOnion p1;
    memset(&p1, 0, sizeof(p1));
    strcpy(p1.proximo_ip, r2->ip);
    p1.proxima_porta = r2->porta;
    memcpy(p1.dados, cifra2, tam_cifra2);
    p1.tamanho_dados = tam_cifra2;
    
    unsigned char cifra1[MAX_PACOTE];
    int tam_cifra1;
    int tam_real_p1 = offsetof(PacoteOnion, dados) + p1.tamanho_dados;
    if(cifrar_pacote((unsigned char*)&p1, tam_real_p1, r1->chave_publica, chave_privada_cliente, cifra1, &tam_cifra1) != 0){
        printf("[!] Erro ao cifrar camada 1!\n");
        return -1;
    }
    
    memcpy(pacote_final, cifra1, tam_cifra1);
    *tamanho_final = tam_cifra1;
    printf("[+] Pacote onion final: %d bytes\n", *tamanho_final);
    return 0;
}

int processar_pacote_onion(const unsigned char *pacote_recebido, int tamanho_recebido, const unsigned char *chave_privada_relay, const unsigned char *chave_publica_cliente, unsigned char *dados_processados, int *tamanho_processados, char *proximo_ip, int *proxima_porta){
    PacoteOnion pkg_decifrado;
    int tam_decifrado;
    if(decifrar_pacote(pacote_recebido, tamanho_recebido, chave_privada_relay, chave_publica_cliente, (unsigned char*)&pkg_decifrado, &tam_decifrado) != 0){
        printf("[!] Erro ao decifrar pacote no relay!\n");
        return -1;
    }
    strcpy(proximo_ip, pkg_decifrado.proximo_ip);
    *proxima_porta = pkg_decifrado.proxima_porta;
    memcpy(dados_processados, pkg_decifrado.dados, pkg_decifrado.tamanho_dados);
    *tamanho_processados = pkg_decifrado.tamanho_dados;
    printf("[+] Relay processou pacote:\n");
    printf("    Próximo destino: %s:%d\n", proximo_ip, *proxima_porta);
    printf("    Dados: %d bytes\n", *tamanho_processados);
    return 0;
}

void carregar_relays_fixos(TabelaDHT *tabela){
    tabela->total = 0;
    strcpy(tabela->relays[0].ip, "127.0.0.1");
    tabela->relays[0].porta = 5000;
    k_load_kx(1, tabela->relays[0].chave_publica, NULL);
    strcpy(tabela->relays[1].ip, "127.0.0.1");
    tabela->relays[1].porta = 5001;
    k_load_kx(2, tabela->relays[1].chave_publica, NULL);
    strcpy(tabela->relays[2].ip, "127.0.0.1");
    tabela->relays[2].porta = 5002;
    k_load_kx(3, tabela->relays[2].chave_publica, NULL);
    tabela->total = 3;
    printf("[+] Relays carregados: %d\n", tabela->total);
}

void cliente_enviar(const char *mensagem, TabelaDHT *tabela){
    if(sodium_init() < 0){
        printf("[!] Erro ao inicializar libsodium\n");
        return;
    }
    
    unsigned char cliente_priv[crypto_box_SECRETKEYBYTES];
    unsigned char cliente_pub[crypto_box_PUBLICKEYBYTES];
    k_load_cl_k(cliente_pub, cliente_priv);
    
    unsigned char pacote[MAX_PACOTE];
    int tamanho;
    if(montar_pacote_onion(mensagem, tabela, cliente_priv, pacote, &tamanho) != 0){
        printf("[!] Erro ao montar pacote onion\n");
        return;
    }
    
    int sock = create_socket(0);
    if(sock < 0){
        printf("[!] Erro ao criar socket\n");
        return;
    }
    
    struct sockaddr_in destino;
    memset(&destino, 0, sizeof(destino));
    destino.sin_family = AF_INET;
    destino.sin_port = htons(tabela->relays[0].porta);
    inet_pton(AF_INET, tabela->relays[0].ip, &destino.sin_addr);
    
    sendto(sock, pacote, tamanho, 0, (struct sockaddr*)&destino, sizeof(destino));
    printf("[+] Pacote enviado para %s:%d\n", tabela->relays[0].ip, tabela->relays[0].porta);
    
    close(sock);
}
