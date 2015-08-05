/**
 * Decrypts results of encalg.c
 * To compile, run "gcc decrypt.c -Wall -lcrypto -o decrypt"
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

// Prints out a string in hexidecimal
void print_in_hex( unsigned char *s, unsigned int len )
{
  unsigned int c;
  printf( "0x" );
  for( c = 0; c < len; c++ )
    printf( "%.2x", s[c] );
  printf( "\n" );
}

// Messy way to convert a single hex char to a number
int convert_to_dec(char c)
{
  unsigned int answer;
  switch(c)
  {
    case '0': answer = 0; break;
    case '1': answer = 1; break;
    case '2': answer = 2; break;
    case '3': answer = 3; break;
    case '4': answer = 4; break;
    case '5': answer = 5; break;
    case '6': answer = 6; break;
    case '7': answer = 7; break;
    case '8': answer = 8; break;
    case '9': answer = 9; break;
    case 'a': answer = 10; break;
    case 'b': answer = 11; break;
    case 'c': answer = 12; break;
    case 'd': answer = 13; break;
    case 'e': answer = 14; break;
    case 'f': answer = 15; break;
    default: answer = -1; break; // No touchie
  }
  return answer;
}

// Convert a hex string to ASCII
void hex_to_ascii(unsigned char *s, unsigned int len )
{
  unsigned int c, h1, h2, hfull;
  for ( c = 2; c < len; c = c + 2) // Take 2 hex digits at a time, ignoring the beginning "0x",
  {
     h1 = convert_to_dec(s[c]);
     h2 = convert_to_dec(s[c + 1]);
     hfull = ((h1 * 16) + h2); // Proper dec number
     printf("%c", (char)hfull); // Output the actual char the dec number correlates to
  }
  printf( "\n" );
}


// Generates a 160 bit key
void gen_160bit_key( unsigned char *key, long clocktime )
{
  unsigned char part1[KEYSIZE];
  unsigned char part2[KEYSIZE];
  long t = clocktime;
  // create a key
  SHA1((void *)&t, sizeof(t), part1); // (input, input_size, output)
  // hash, again
  SHA1((void *)part1, KEYSIZE, part2);
  // and hash, again
  SHA1((void *)part2, KEYSIZE, key);
}

int main( int argc, char *argv[] )
{
  // IV = "0x0d50d19654688f50" // 18 chars long: 0x + 16 chars (8 bytes originally)
  //    Original value is IV = {13, 80, 209, 150, 84, 104, 143, 80};
  // Ciphertext = "0x53f745f2a393face4e1981c268f7c7b414b1539b32939c84" // 50 chars long: 0x + 48 chars (24 bytes originally)
  //	Original value is ciphertext = {83, 247, 69, 242, 163, 147, 250, 206, 78, 25, 129, 194, 104, 247, 199, 180, 20, 177, 83, 155, 50, 147, 156, 132}
  // Find the original values by making hex_to_ascii print out a list of %d's, not %c's (ints, not chars)
  BF_KEY bf_key; 
  unsigned char plaintext[MAXPLAINTEXTSIZE]; // Format: password=[14 chars]
  unsigned char ciphertext[] = {83, 247, 69, 242, 163, 147, 250, 206, 78, 25, 129, 194, 104, 247, 199, 180, 20, 177, 83, 155, 50, 147, 156, 132};
  unsigned char key[KEYSIZE];
  int ct_length = 24;

  // Error checking to see if we have the proper values for the IV and ciphertext
/*  unsigned char IV[] = {13, 80, 209, 150, 84, 104, 143, 80};
  printf("IV=");
  print_in_hex(IV, 8); // Prints as "IV=0x0d50d19654688f50" => good
  printf("ciphertext=");
  print_in_hex(ciphertext, 24); // Prints as "ciphertext=0x53f745f2a393face4e1981c268f7c7b414b1539b32939c84" => good */

  // Run this loop for a longass time
  // ./decrypt | grep 70617373776f72643d (hex value of "password=")
  // Time where this shows up is 1349907372
  // 10/01/2012, 00:00:00 = 1349049600 in Unix time
  // 10/25/2012, 00:00:00 = 1351123200 in Unix time
  //long clocktime = 1349049600;
  //while (clocktime < 1351123201)
  long clocktime = 1349907372; // Wed, 10 Oct 2012 22:16:12 GMT
  //{
	// generate a key using SHA-1 over some changing value
	gen_160bit_key( key, clocktime );
	// set the key 
	BF_set_key( &bf_key, KEYSIZE, key );
	// decrypt the result
	unsigned char IV[] = {13, 80, 209, 150, 84, 104, 143, 80};
	BF_cbc_encrypt(ciphertext, plaintext, ct_length, &bf_key, IV, BF_DECRYPT );
	printf("Clocktime = %ld", clocktime);
	printf("\nHexed plaintext: ");
	print_in_hex(plaintext, ct_length); // Hex value: 0x70617373776f72643d737061676865747469303030303030
	unsigned char plaintext_hexed[] = "0x70617373776f72643d737061676865747469303030303030";
	printf("Decrypted password: " );
	hex_to_ascii(plaintext_hexed, 50); // Decrypted value: password=spaghetti000000
	//clocktime++;
  //}
  return 0;
}
