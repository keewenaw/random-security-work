/**
 * Basic encryption program to break (given to us beforehand)
 * 
 * Just a toy algorithm, not actually used
 *
 * To compile me, run "gcc encrypt.c -Wall -lcrypto -o encrypt"
 */



#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <string.h>
#include <openssl/blowfish.h>
#include <openssl/sha.h>
#include <sys/types.h>
#include <unistd.h>

#define KEYSIZE 20
#define MAXPLAINTEXTSIZE 1024	/* should be divisible by 8B */


/**
 * Prints out a string in hexidecimal
 */
void print_in_hex( unsigned char *s, unsigned int len )
{
  unsigned int c;
  printf( "0x" );
  for( c = 0; c < len; c++ )
    printf( "%.2x", s[c] );
  printf( "\n" );
}


/**
 * Generates a 160 bit key and an IV
 */
void gen_160bit_key( unsigned char *iv, unsigned char *key )
{
  unsigned char part1[KEYSIZE];
  unsigned char part2[KEYSIZE];
  pid_t pid;

  /* first, create an IV, based on this process' pid */
  pid = getpid();
  SHA1((void *)&pid, sizeof(pid_t), part1);
  memcpy( iv, part1, 8 );	/* PIDs are 8 bytes */

  /* now, create a key */
  long t;
  t = time(NULL);
  SHA1((void *)&t, sizeof(t), part1);
  // hash, again
  SHA1((void *)part1, KEYSIZE, part2);
  // and hash, again
  SHA1((void *)part2, KEYSIZE, key);
}
  



int main( int argc, char *argv[] )
{
  BF_KEY bf_key;
  unsigned char iv[8];
  unsigned char plaintext[MAXPLAINTEXTSIZE];
  unsigned char ciphertext[MAXPLAINTEXTSIZE];
  unsigned char key[KEYSIZE];
  int br;

  /* read in input from stdin 
     Let's assume the input is a multiple of 8 bytes. */
  br = read( STDIN_FILENO, plaintext, MAXPLAINTEXTSIZE );

  /* generate a key using SHA-1 over some changing value */
  gen_160bit_key( iv, key );

  /* output the IV */
  printf( "IV=" );
  print_in_hex( iv, 8 );

  /* set the key */
  BF_set_key( &bf_key, KEYSIZE, key );

  /* encrypt using Blowfish in CBC mode */
  BF_cbc_encrypt(plaintext, ciphertext, br, &bf_key, iv, BF_ENCRYPT );

  printf( "ciphertext=" );
  print_in_hex( ciphertext, br );

  return 0;
}
