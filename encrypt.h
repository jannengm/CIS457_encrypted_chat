/*******************************************************************************
 * CIS 457 - Project 3: TCP Encrypted Chat Program
 * encrypt.h header file
 * @author Mark Jannenga
 *
 * Defines constants and declares functions used in encryption and decryption
 * of messages.
 ******************************************************************************/

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

/*******************************************************************************
 * Handles errors generated in the encryption and decryption functions defined
 * below.
 ******************************************************************************/
void handleErrors(void);

/*******************************************************************************
 * Encrypts a message using an asymmetric RSA public key
 *
 * @param in - The message to be encrypted
 * @param inlen - The size of the message to be encrypted
 * @param key - The RSA public key to use to encrypt the message
 * @param out - The unsigned character array to store the encrypted message in
 * @return outlen - The length of the encrypted character array
 ******************************************************************************/
int rsa_encrypt(unsigned char* in, size_t inlen, EVP_PKEY *key, unsigned char* out);

/*******************************************************************************
 * Decrypts an asymmetrically encrypted message using an RSA private key.
 *
 * @param in - The encrypted message
 * @param inlen - The length of the encrypted message
 * @param key - The RSA private key to use to decrypt the message
 * @param out - The character array to store the decrypted message in
 * @return outlen - The length of the decrypted message
 ******************************************************************************/
int rsa_decrypt(unsigned char* in, size_t inlen, EVP_PKEY *key, unsigned char* out);

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
            unsigned char *iv, unsigned char *ciphertext);

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
            unsigned char *iv, unsigned char *plaintext);

#endif //CIS457_ENCRYPTED_CHAT_ENCRYPT_H
