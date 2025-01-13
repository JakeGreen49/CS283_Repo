#include <stdio.h>
#include <string.h>
#include <stdlib.h>


#define BUFFER_SZ 50

//prototypes
void usage(char *);
void print_buff(char *, int);
int  setup_buff(char *, char *, int);

//prototypes for functions to handle required functionality
int  count_words(char *, int, int);
//add additional prototypes here


int setup_buff(char *buff, char *user_str, int len){
    //TODO: #4:  Implement the setup buff as per the directions
	int buff_len = 0; // Tracks the end of the user str in the buff
	int i = 0; // Increments over the user string
	char c;

	while (*(user_str + i) != '\0') {
		// The user string is longer than the buffer, return -1.
		if (buff_len == len) return(-1);

		// Store the current character in variable c, replacing any tabs with a space
		c = (*(user_str + i) == '\t') ? ' ' : *(user_str + i);

		// Skip over a leading whitespace
		if (i == 0 && c == ' ') {
			i++;
			continue;
		}

		// Continue if there is duplicate whitespace
		if (i > 0) {
			if (c == ' ' && *(user_str + i - 1) == ' ') {
				i++;
				continue;
			}
		}

		// Insert the user character into the buff
		*(buff + buff_len) = c;
		buff_len++;

		i++;
			
	}

	// Add the periods to the end of the buffer
	for (int end = buff_len; end < len; end++) {
		*(buff + end) = '.';
	}

    return buff_len; 
}

void print_buff(char *buff, int len){
    printf("Buffer:  ");
    for (int i=0; i<len; i++){
        putchar(*(buff+i));
    }
    putchar('\n');
}

void usage(char *exename){
    printf("usage: %s [-h|c|r|w|x] \"string\" [other args]\n", exename);

}

int count_words(char *buff, int len, int str_len){
	int wordcount = 1;

	for (int i=0; i < str_len; i++) {
		if (*(buff + i) == ' ') wordcount++;
	}

    return wordcount;
}

void string_reverse(char* buff, int len, int str_len) {
	char temp;
	char* user_str = malloc((sizeof(char) * str_len) + 1);

	// Reverse the string
	for (int i=0; i < str_len/2; i++) {
		temp = *(buff + i);
		
		// Swap the last character with the first, second last with the second, etc.
		*(buff + i) = *(buff + str_len - i - 1);
		*(buff + str_len - i - 1) = temp;
	}

	// Parse the user string from the buffer
	for (int i=0; i < str_len; i++) {
		*(user_str + i) = *(buff + i);
	}

	// Add the null terminator so we can directly insert the string with printf
	*(user_str + str_len) = '\0';

	printf("Reversed String: %s\n", user_str);
	free(user_str);
}

void print_words(char* buff, int len, int str_len) {
	int char_count = 0;
	int num_words = 1;

	printf("Word Print\n----------\n1. ");
	
	for (int i=0; i < str_len; i++) {
		// When we reach a space, print out the word count and increment the number of words
		if (*(buff + i) == ' ') {
			num_words++;
			printf("%c(%d)\n%d. ", *(buff + i), char_count, num_words);
			char_count = 0;
		} 

		// Print out the word character by character
		else {
			printf("%c", *(buff + i));
			char_count++;
		}
	}

	printf(" (%d)\n", char_count);
}

void replace_word(char* buff, int len, int str_len, char* old, char* new) {


}

//ADD OTHER HELPER FUNCTIONS HERE FOR OTHER REQUIRED PROGRAM OPTIONS

int main(int argc, char *argv[]){

    char *buff;             //placehoder for the internal buffer
    char *input_string;     //holds the string provided by the user on cmd line
    char opt;               //used to capture user option from cmd line
    int  rc;                //used for return codes
    int  user_str_len;      //length of user supplied string

    //TODO:  #1. WHY IS THIS SAFE, aka what if arv[1] does not exist?
    //      This is safe because the if statement checks for the number of arguments (ie. argc) being less than 2;
    //      if argv[1] does not exist, then argc must be less than 2, so the if statement block will execute and the program will exit.
    if ((argc < 2) || (*argv[1] != '-')){
        usage(argv[0]);
        exit(1);
    }

    opt = (char)*(argv[1]+1);   //get the option flag

    //handle the help flag and then exit normally
    if (opt == 'h'){
        usage(argv[0]);
        exit(0);
    }

    //WE NOW WILL HANDLE THE REQUIRED OPERATIONS

    //TODO:  #2 Document the purpose of the if statement below
    //      The expected usage of this program will have at minimum 3 arguments: the name of the program, the option flag, and the string. Hence,
	//		if one of these is missing, this block of code prints out the usage guide and exits with a status of 1.
    if (argc < 3){
        usage(argv[0]);
        exit(1);
    }

    input_string = argv[2]; //capture the user input string

    //TODO:  #3 Allocate space for the buffer using malloc and
    //          handle error if malloc fails by exiting with a 
    //          return code of 99
	buff = malloc(sizeof(char) * BUFFER_SZ);
	if (buff == NULL) exit(99);


    user_str_len = setup_buff(buff, input_string, BUFFER_SZ);     //see todos
    if (user_str_len < 0){
        printf("Error setting up buffer, error = %d", user_str_len);
        exit(2);
    }

    switch (opt){
        case 'c':
            rc = count_words(buff, BUFFER_SZ, user_str_len);  //you need to implement
            if (rc < 0){
                printf("Error counting words, rc = %d", rc);
                exit(2);
            }
            printf("Word Count: %d\n", rc);
            break;

        //TODO:  #5 Implement the other cases for 'r' and 'w' by extending
        //       the case statement options
		case 'h':
			usage(argv[0]);
			break;

		case 'r':
			string_reverse(buff, BUFFER_SZ, user_str_len);
			break;

		case 'w':
			print_words(buff, BUFFER_SZ, user_str_len);
			break;
		case 'x':
			//FIXME add in function
        default:
            usage(argv[0]);
            exit(1);
    }

    //TODO:  #6 Dont forget to free your buffer before exiting
    print_buff(buff,BUFFER_SZ);
	free(buff);
    exit(0);
}

//TODO:  #7  Notice all of the helper functions provided in the 
//          starter take both the buffer as well as the length.  Why
//          do you think providing both the pointer and the length
//          is a good practice, after all we know from main() that 
//          the buff variable will have exactly 50 bytes?
//  
//          This is good practice because it allows us to check that the buffer
//			provided to the function is the correct length, and if it is not we
//			know something unexpected has occurred and to exit with an error status
