#include <stdio.h>
#include <string.h>
#include <stdlib.h>


#define BUFFER_SZ 50

//prototypes
void usage(char *);
void print_buff(char *, int);
int  setup_buff(char *, char *, int);

//prototypes for functions to handle required functionality
int  count_words(char *, int);
//add additional prototypes here
int  string_reverse(char *, int);
int  print_words(char *, int);
int  cmp_chars(char*, char*, int, int);
int  replace_word(char*, int, char*, char*);

int setup_buff(char *buff, char *user_str, int len){
    //TODO: #4:  Implement the setup buff as per the directions
	int buff_len = 0; // Tracks the end of the user str in the buff
	char c;

	for (int i=0; *(user_str + i) != '\0'; i++) {
		// The user string is longer than the buffer, return -1.
		if (buff_len == len) return -1;

		// Store the current character in variable c, replacing any tabs with a space
		c = (*(user_str + i) == '\t') ? ' ' : *(user_str + i);

		// User string contains non-letter/punctuation characters
		if (c < ' ') return -2;

		// Skip over a leading whitespace
		if (i == 0 && c == ' ') {
			continue;
		}

		// Continue if there is duplicate whitespace
		if (i > 0) {
			if (c == ' ' && *(user_str + i - 1) == ' ') {
				continue;
			}
		}

		// Insert the user character into the buff
		*(buff + buff_len) = c;
		buff_len++;
			
	}

	// Take care of a trailing space
	if (*(buff + buff_len - 1) == ' ') *(buff + buff_len - 1) = '.';

	// Add the periods to the end of the buffer
	for (int end = buff_len; end < len; end++) {
		*(buff + end) = '.';
	}

    return buff_len; 
}

void print_buff(char *buff, int len){
    //printf("Buffer:  ");
	printf("Buffer:  [");
    for (int i=0; i<len; i++){
        putchar(*(buff+i));
    }
    //putchar('\n');
	printf("]\n");
}

void usage(char *exename){
    printf("usage: %s [-h|c|r|w|x] \"string\" [other args]\n", exename);

}

//ADD OTHER HELPER FUNCTIONS HERE FOR OTHER REQUIRED PROGRAM OPTIONS

int count_words(char *buff, int str_len){
	// The buffer will (almost) always start with a word
	int wordcount = 1;

	for (int i=0; i < str_len; i++) {
		if (*(buff + i) == ' ') wordcount++;
	}

	// In the case of an empty string (ie. user input string only had spaces) the word count will be 0
	if (str_len == 0) return 0;

    return wordcount;
}

int string_reverse(char* buff, int str_len) {
	char temp;
	char* user_str = malloc((sizeof(char) * str_len) + 1);
	if (user_str == NULL) return -99; // Malloc error 

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

	//printf("Reversed String: %s\n", user_str);
	free(user_str);

	return 0;
}

int print_words(char* buff, int str_len) {

	int char_count = 0;
	int num_words = 1;
	
	// If the user string is empty (ie. only spaces inputted) there are no words to print out
	if (str_len == 0) {
		printf("Word Print\n----------\n");
		return 0;
	}

	printf("Word Print\n----------\n1. ");
	
	for (int i=0; i < str_len; i++) {
		// When we reach a space, print out the word count and increment the number of words
		if (*(buff + i) == ' ') {
			num_words++;
			//printf("%c(%d)\n%d. ", *(buff + i), char_count, num_words);
			printf("(%d)\n%d. ", char_count, num_words);
			char_count = 0;
		} 

		// Print out the word character by character
		else {
			printf("%c", *(buff + i));
			char_count++;
		}
	}

	printf("(%d)\n\nNumber of words returned: %d\n", char_count, num_words);

	return num_words;
}

// Helper function used in replace_word
int cmp_chars(char* buff, char* word, int wrd_len, int start) {
	// Compare the word to the start position in the buffer, return 1 for a match and 0 for no match
	for (int i=0; i < wrd_len; i++) {
		if (*(buff + start + i) != *(word + i)) return 0;
	}

	return 1;
}

int replace_word(char* buff, int str_len, char* old, char* new) {
	// Calculate the length of the word we want to replace (char* old)
	int old_len = 0;
	for (int j=0; *(old + j) != '\0'; j++) {
		old_len++;
	}

	// Do the same for the word we will be placing in (char* new)
	int new_len = 0;
	for (int j=0; *(new + j) != '\0'; j++) {
		new_len++;
	}
	
	// Scan the buffer for the first instance of the old word
	int old_pos = -1;
	for (int i=0; i < str_len; i++) {
		if (cmp_chars(buff, old, old_len, i)) {
			old_pos = i;
			break;
		}
	}

	// The word to be replaced was not found in the user string
	if (old_pos < 0) return old_pos;

	// Allocate space for the string with the replacement
	char* new_str = malloc(sizeof(char) * (str_len - old_len + new_len));
	if (new_str == NULL) return -99; // Malloc error

	// Copy over the buffer into the new string, replacing the first instance of the old word
	int true_pos = 0;
	for (int i=0; i < str_len; i++) {
		if (i == old_pos) {
			// Copy in the new word
			for (int j=0; j < new_len; j++) {
				*(new_str + true_pos + j) = *(new + j);
			}
			
			// Increment the position counter for the new string by the new word length and the position counter for the buffer by the old word length
			true_pos += new_len; i += old_len - 1;
		}

		else {
			// Copy over the buffer
			*(new_str + true_pos) = *(buff + i);
			true_pos++;
		}
	}


	// If the string overflows the buffer, return the amount of characters that overflow
	if (true_pos > BUFFER_SZ) {
		free(new_str);
		return BUFFER_SZ - true_pos;
	}

	// Print the modified string and return the number of characters replaced
	else {
		//*(new_str + true_pos) = '\0';
		//printf("Modified String: %s\n", new_str);
		setup_buff(buff, new_str, BUFFER_SZ);

		free(new_str);
		return abs(true_pos - str_len);
	}
}


int main(int argc, char *argv[]){

    char *buff;             //placehoder for the internal buffer
    char *input_string;     //holds the string provided by the user on cmd line
    char opt;               //used to capture user option from cmd line
    int  rc;                //used for return codes
    int  user_str_len;      //length of user supplied string

    //TODO:  #1. WHY IS THIS SAFE, aka what if arv[1] does not exist?
    //      This is safe because the if statement checks for the number of arguments (ie. argc) being less than 2;
    //      if argv[1] does not exist, then argc must be less than 2, so the if statement block will execute and the program will exit
	//		before it checks the value of argv[1].
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
            rc = count_words(buff, user_str_len);  //you need to implement
            if (rc < 0){
                printf("Error counting words, rc = %d\n", rc);
                exit(2);
            }
            printf("Word Count: %d\n", rc);
            break;

        //TODO:  #5 Implement the other cases for 'r' and 'w' by extending
        //       the case statement options

		case 'r':
			rc = string_reverse(buff, user_str_len);
			if (rc != 0) {
				printf("Error in reversing string, rc = %d\n", rc);
				exit(2);
			}
			break;

		case 'w':
			rc = print_words(buff, user_str_len);
			if (rc < 0) {
				printf("Error counting words, rc = %d\n", rc);
				exit(2);
			}
			break;
		case 'x':
			// The user has not provided all the needed arguments
			if (argc < 4) {
	        	usage(argv[0]);
        		exit(1);
			}

			rc = replace_word(buff, user_str_len, argv[3], argv[4]);
			if (rc < 0) {
				//printf("Error replacing words, rc = %d\n", rc);
				printf("Not Implemented!");
				exit(2);
			}
			break;
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
//			know something unexpected has occurred and to exit with an error status.
//			This is generally good practice when the buffer size is not hardcoded to be a certain size.
