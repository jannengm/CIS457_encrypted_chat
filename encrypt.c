//
// Created by jannengm on 11/11/16.
//

#include "encrypt.h"

void handleErrors(void)
{
    ERR_print_errors_fp(stderr);
    abort();
}

int rsa_encrypt(unsigned char* in, size_t inlen, EVP_PKEY *key, unsigned char* out){
    EVP_PKEY_CTX *ctx;
    size_t outlen;
    ctx = EVP_PKEY_CTX_new(key, NULL);
    if (!ctx)
        handleErrors();
    if (EVP_PKEY_encrypt_init(ctx) <= 0)
        handleErrors();
    if (EVP_PKEY_CTX_set_rsa_padding(ctx, RSA_PKCS1_OAEP_PADDING) <= 0)
        handleErrors();
    if (EVP_PKEY_encrypt(ctx, NULL, &outlen, in, inlen) <= 0)
        handleErrors();
    if (EVP_PKEY_encrypt(ctx, out, &outlen, in, inlen) <= 0)
        handleErrors();
    return (int)outlen;
}

int rsa_decrypt(unsigned char* in, size_t inlen, EVP_PKEY *key, unsigned char* out){
    EVP_PKEY_CTX *ctx;
    size_t outlen;
    ctx = EVP_PKEY_CTX_new(key,NULL);		/*Context*/
    if (!ctx)
        handleErrors();
    if (EVP_PKEY_decrypt_init(ctx) <= 0)
        handleErrors();
    if (EVP_PKEY_CTX_set_rsa_padding(ctx, RSA_PKCS1_OAEP_PADDING) <= 0)
        handleErrors();
    if (EVP_PKEY_decrypt(ctx, NULL, &outlen, in, inlen) <= 0)
        handleErrors();
    if (EVP_PKEY_decrypt(ctx, out, &outlen, in, inlen) <= 0)
        handleErrors();
    return (int)outlen;
}

/*plaintext - text to be encrypted
 *plaintext_len - length of message to be ecnrypted
 *key - encryption key
 *iv - Initialization Vector, another input to encrypt and decrypt functions
		Should be new for every new symmetric encryption key. Can send
		as plain text
 *ciphertext - the encrypted output*/
int encrypt(unsigned char *plaintext, int plaintext_len, unsigned char *key,
            unsigned char *iv, unsigned char *ciphertext){
    EVP_CIPHER_CTX *ctx;
    int len;
    int ciphertext_len;
    if(!(ctx = EVP_CIPHER_CTX_new())) handleErrors();
    if(1 != EVP_EncryptInit_ex(ctx, EVP_aes_256_cbc(), NULL, key, iv))
        handleErrors();
    if(1 != EVP_EncryptUpdate(ctx, ciphertext, &len, plaintext, plaintext_len))
        handleErrors();
    ciphertext_len = len;
    if(1 != EVP_EncryptFinal_ex(ctx, ciphertext + len, &len)) handleErrors();
    ciphertext_len += len;
    EVP_CIPHER_CTX_free(ctx);
    return ciphertext_len;
}

int decrypt(unsigned char *ciphertext, int ciphertext_len, unsigned char *key,
            unsigned char *iv, unsigned char *plaintext){
    EVP_CIPHER_CTX *ctx;
    int len;
    int plaintext_len;
    if(!(ctx = EVP_CIPHER_CTX_new())) handleErrors();
    if(1 != EVP_DecryptInit_ex(ctx, EVP_aes_256_cbc(), NULL, key, iv))
        handleErrors();
    if(1 != EVP_DecryptUpdate(ctx, plaintext, &len, ciphertext, ciphertext_len))
        handleErrors();
    plaintext_len = len;
    if(1 != EVP_DecryptFinal_ex(ctx, plaintext + len, &len)) handleErrors();
    plaintext_len += len;
    EVP_CIPHER_CTX_free(ctx);
    return plaintext_len;
}