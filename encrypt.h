//
// Created by jannengm on 11/11/16.
//

#ifndef CIS457_ENCRYPTED_CHAT_ENCRYPT_H
#define CIS457_ENCRYPTED_CHAT_ENCRYPT_H

#include <openssl/conf.h>
#include <openssl/evp.h>
#include <openssl/err.h>
#include <openssl/pem.h>
#include <openssl/rand.h>
#include <openssl/rsa.h>
#include <string.h>

#define KEY_LEN 32
#define IV_LEN 16
#define ENCRYPTEDKEY_LEN 256

void handleErrors(void);
int rsa_encrypt(unsigned char* in, size_t inlen, EVP_PKEY *key, unsigned char* out);
int rsa_decrypt(unsigned char* in, size_t inlen, EVP_PKEY *key, unsigned char* out);
int encrypt(unsigned char *plaintext, int plaintext_len, unsigned char *key,
            unsigned char *iv, unsigned char *ciphertext);
int decrypt(unsigned char *ciphertext, int ciphertext_len, unsigned char *key,
            unsigned char *iv, unsigned char *plaintext);

#endif //CIS457_ENCRYPTED_CHAT_ENCRYPT_H
