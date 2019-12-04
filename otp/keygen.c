#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[]) {
	int i, n, k;
	time_t t;
	n = atoi(argv[1]); // get the first arg, keylen
	srand((unsigned) time(&t));
	char key_dict[27] = {'A','B','C','D','E','F','G','H','I','J','K','L','M','N','O','P','Q','R','S','T','U','V','W','X','Y','Z',' '};
	for(i = 0; i < n; i++){	// make a bunch of random characters in our bounds by printing the member k of the key_dict char array
		k = rand() % 27;
		printf("%c", key_dict[k]);	
	}
	printf("\n");
}
