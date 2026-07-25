#include <sodium.h>
#include <string.h>
#include <stdio.h>
#include "kamelia.h"

int cifrar_mensagem(const unsigned char *mensagem, int tamanho_mensagem, const unsigned char *chave_publica_receptor, const unsigned char *chave_privada_emissor, unsigned char *cifra, int *tamanho_cifra, unsigned char *nonce){
    if(sodium_init() < 0){
        printf("[!] Erro: sodium_init() falhou!\n");
        return -1;
    }
    randombytes_buf(nonce, crypto_box_NONCEBYTES);
    if(crypto_box_easy(cifra, mensagem, tamanho_mensagem, nonce, chave_publica_receptor, chave_privada_emissor) != 0){
        printf("[!] Erro ao cifrar mensagem!\n");
        return -1;
    }
    *tamanho_cifra = tamanho_mensagem + crypto_box_MACBYTES;
    return 0;
}

int decifrar_mensagem(const unsigned char *cifra, int tamanho_cifra, const unsigned char *chave_privada_receptor, const unsigned char *chave_publica_emissor, const unsigned char *nonce, unsigned char *mensagem_decifrada, int *tamanho_decifrado){
    if(sodium_init() < 0){
        printf("[!] Erro: sodium_init() falhou!\n");
        return -1;
    }
    if(crypto_box_open_easy(mensagem_decifrada, cifra, tamanho_cifra, nonce, chave_publica_emissor, chave_privada_receptor) != 0){
        printf("[!] Erro ao decifrar mensagem! (chave errada ou dados corrompidos)\n");
        return -1;
    }
    *tamanho_decifrado = tamanho_cifra - crypto_box_MACBYTES;
    return 0;
}

int cifrar_pacote(const unsigned char *dados, int tamanho_dados, const unsigned char *chave_publica_receptor, const unsigned char *chave_privada_emissor, unsigned char *pacote_final, int *tamanho_pacote){
    unsigned char nonce[crypto_box_NONCEBYTES];
    unsigned char cifra[MAX_PACOTE];
    int tamanho_cifra;
    if(cifrar_mensagem(dados, tamanho_dados, chave_publica_receptor, chave_privada_emissor, cifra, &tamanho_cifra, nonce) != 0){
        return -1;
    }
    memcpy(pacote_final, nonce, crypto_box_NONCEBYTES);
    memcpy(pacote_final + crypto_box_NONCEBYTES, cifra, tamanho_cifra);
    *tamanho_pacote = crypto_box_NONCEBYTES + tamanho_cifra;
    return 0;
}

int decifrar_pacote(const unsigned char *pacote, int tamanho_pacote, const unsigned char *chave_privada_receptor, const unsigned char *chave_publica_emissor, unsigned char *dados_decifrados, int *tamanho_decifrado){
    unsigned char nonce[crypto_box_NONCEBYTES];
    memcpy(nonce, pacote, crypto_box_NONCEBYTES);
    const unsigned char *cifra = pacote + crypto_box_NONCEBYTES;
    int tamanho_cifra = tamanho_pacote - crypto_box_NONCEBYTES;
    return decifrar_mensagem(cifra, tamanho_cifra, chave_privada_receptor, chave_publica_emissor, nonce, dados_decifrados, tamanho_decifrado);
}
