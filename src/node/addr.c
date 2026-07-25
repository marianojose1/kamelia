#include <sodium.h>
#include <stdio.h>
#include <string.h>
#include "kamelia.h"

void a_gen_addr(){
    if(sodium_init() < 0) return;
    unsigned char pub[crypto_sign_PUBLICKEYBYTES];
    FILE* fp = fopen("keys/PUBLIC_KEY.kam", "rb");
    if(!fp){
        printf("Erro ao abrir PUBLIC_KEY.kam!\n");
        return;
    }
    fread(pub, 1, crypto_sign_PUBLICKEYBYTES, fp);
    fclose(fp);
    unsigned char hash[crypto_hash_sha256_BYTES];
    crypto_hash_sha256(hash, pub, crypto_sign_PUBLICKEYBYTES);
    char base64[64];
    char addr[128];
    sodium_bin2base64(base64, sizeof(base64), hash, crypto_hash_sha256_BYTES, sodium_base64_VARIANT_URLSAFE_NO_PADDING);
    snprintf(addr, sizeof(addr), "%s.kamelia", base64);
    if(!check_archv("keys/hostname")){
        fp = fopen("keys/hostname", "w");
        if(!fp){
            printf("Erro ao salvar hostname!\n");
            return;
        }
        fprintf(fp, "%s\n", addr);
        printf("\nEndereço salvo: %s\n", addr);
        fclose(fp);
    } else {
        printf("\nEndereço já existente!\n");
    }
}
