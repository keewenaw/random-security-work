/*
 * scbf.c - shift cipher brute forcer
 * 
 * This program bruteforces a shift cipher
 * Decryption of a encrypted character is d(c) = (c - k) % m
 * m = 26
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define maxLength 100 // Max length of ciphertext array

char ciphertext[maxLength] = "zqfiadweqogdufknqefoxmeeqhqd"; // Ciphertext to bruteforce, max length 100 chars
int m = 26; // Value to mod (c - k) by
int k = 0; // Key to use
char results[26][maxLength]; // Array to store decrypted strings

// Functions we'll use
void usage(char name[]);
void getArgs(int argc, char* argv[]);
void decrypt();
void printResults();

// Main
int main(int argc, char *argv[]) {
	getArgs(argc, argv); // Parse command-line arguements
	decrypt(); // Decrypt the ciphertext
	printResults(); // Print out what we found out
	return 0;
} // End main

// This function gets the command-line arguments, if any, and 
// updates the respective variables in the program
void getArgs(int argc, char* argv[]) {
	int i;
	for (i = 1; i < argc; i = i + 2) {
		if (strcmp(argv[i], "-h") == 0) usage(argv[0]); // Print out help menu
		else if (strcmp(argv[i], "-m") == 0) { // Replace m
			if (atoi(argv[i + 1]) < 1) { // m needs to be in bounds
				printf("ERROR: m must be greater than one! Defaulting to m = 26.\n");
				m = 26;
			}
			else m = atoi(argv[i + 1]); // We're good
		}
		else if (strcmp(argv[i], "-k") == 0) { // Replace k; need to keep it in m's range
			if (atoi(argv[i + 1]) < 0) { // k needs to be in bounds
				printf("ERROR: k must be a nonnegative number! Defaulting to k = 0.\n");
				k = 0;
			}
			else k = atoi(argv[i + 1]) % m; // We're good
		}
		else if (strcmp(argv[i], "-c") == 0) { // Replace ciphertext
			if (strlen(argv[i + 1]) < 1) {
				printf("ERROR: ciphertext must be a valid string, length > 0. Defaulting to \'zqfiadweqogdufknqefoxmeeqhqd\'.\n");
			}
			memset(ciphertext, '\0', sizeof(ciphertext)); // Reset ciphertext
			char tempString[100]; // Temp string to ensure the ciphertext is all lowercase
			strncpy(tempString, argv[i + 1], sizeof(tempString)); // Put the command line arg into the temp variable
			int counter;
			for (counter = 0; counter < strlen(tempString); counter++) { // Convert each char in tempString to lower case
				tempString[counter] = tolower(tempString[counter]);
			}
			strncpy(ciphertext, tempString, sizeof(tempString)); // Put the lower case chars back into ciphertext
		}
		else { // You dun goofd; time to shit bricks
				printf("ERROR: %s switch is not a valid option!\n", argv[i]); 
				usage(argv[0]);
				exit(1);
		} // End else
	} // End for
} // End getArgs()

// This function prints out the help menu
void usage(char name[]) {
	printf("Shift cipher brute-forcer - 9/20/2012\n");
	printf("%s [-h] [-m value] [-k value] [-c string]\n", name);
	printf("Usage:\n");
	printf("\t-h: prints out help menu and exits\n");
	printf("\t-m modValue: value to mod (c - k) by (default: %d)\n", m);
	printf("\t-k key: key to start at (default: %d)\n", k);
	printf("\t-c ciphertext: ciphertext to brute force (default: %s)\n", ciphertext);
	exit(0);
} // End usage()

// This function decrypts the given ciphertext, using all possible keys
// Use the formula d(c) = (c - k) % m
// Lowercase chars go from 'a' = 97 to 'z' = 122
void decrypt() {
	int try; // What try are we on? 0-m
	int encryptedCharIndex; // Index of the char to decrypt
	int cipherLength = strlen(ciphertext); // Size of ciphertext
	char decryptedString[maxLength]; // Decrypted string
	char encryptedChar; // Encrypted char we want to decrypt
	for (try = 0; try < m; try = try + 1) { // For each valid key
		for (encryptedCharIndex = 0; encryptedCharIndex < strlen(ciphertext) && ciphertext[encryptedCharIndex] != '\0'; encryptedCharIndex = encryptedCharIndex + 1) { // While there are chars to decrypt
			encryptedChar = ciphertext[encryptedCharIndex]; // Get the next encrypted char
			encryptedChar = encryptedChar - k; // Decrypt it
			if (encryptedChar < 'a') encryptedChar = encryptedChar + 26;
			decryptedString[encryptedCharIndex] = encryptedChar; // Store it in our decrypted string
			encryptedChar = '\0'; // Reset the encryptedChar variable
		} // End for
		strncpy(results[k], decryptedString, strlen(decryptedString)); // Put the full decrypted string in our output array
		k = (k + 1) % m; // Update k
	} // End for
} // End decrypt()

// This function prints out each key used to decrypt the ciphertext, with the associated plaintext
void printResults() {
	printf("\nShift cipher brute-forcer - 9/20/2012\n\n");
	printf("Our known ciphertext is: %s\n\n", ciphertext);
	int indexOfResults;
	for (indexOfResults = 0; indexOfResults < m; indexOfResults++) {
		printf("The string decrypted with key value %d: %s\n", indexOfResults, results[indexOfResults]);
	} // End for
	printf("\n"); // Basic formatting
}
