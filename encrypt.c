/*******************************************************************************
 * CIS 457 - Project 3: TCP Encrypted Chat Program
 * encrypt.c source code
 * @author Mark Jannenga
 *
 * Implements functions declared in encrypt.h. The majority of the code was
 * provided by the instructor, Prof. Andrew Kalafut, which in turn was copied
 * heavily from the OpenSSL wiki and man pages.
 ******************************************************************************/

#include "encrypt.h"

/*******************************************************************************
 * Handles errors generated in the encryption and decryption functions defined
 * below.
 ******************************************************************************/
void handleErrors(void)
{
    ERR_print_errors_fp(stderr);
    abort();
}

/*******************************************************************************
 * Encrypts a message using an asymmetric RSA public key
 *
 * @param in - The message to be encrypted
 * @param inlen - The size of the message to be encrypted
 * @param key - The RSA public key to use to encrypt the message
 * @param out - The unsigned character array to store the encrypted message in
 * @return outlen - The length of the encrypted character array
 ******************************************************************************/
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

/*******************************************************************************
 * Decrypts an asymmetrically encrypted message using an RSA private key.
 *
 * @param in - The encrypted message
 * @param inlen - The length of the encrypted message
 * @param key - The RSA private key to use to decrypt the message
 * @param out - The character array to store the decrypted message in
 * @return outlen - The length of the decrypted message
 ******************************************************************************/
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

/*******************************************************************************
 * Encrypts a message using a symmetric key and an initialization vector
 *
 * @param plaintext - The message to be encrypted
 * @param plaintext_len - The length of the message
 * @param key - The symmetric key to use to encrypt the message
 * @param iv - The initialization vector
 * @param ciphertext - The location to store the encrypted message
 * @return ciphertext_len - The length of the encrypted message
 ******************************************************************************/
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

/*******************************************************************************
 * Decrypts a symmetrically encrypted message
 *
 * @param ciphertext - The encrypted message
 * @param ciphertext_len - The length of the encrypted message
 * @param key - The symmetric key to use to decrypt the message
 * @param iv - The initialization vector
 * @param plaintext - The location to store the decrypted message
 * @return plaintext_len - The length of the decrypted message
 ******************************************************************************/
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