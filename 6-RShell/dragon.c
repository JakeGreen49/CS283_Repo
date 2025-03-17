#include <stdio.h>
#include "dshlib.h"

// EXTRA CREDIT - print the drexel dragon from the readme.md
extern void print_dragon(){
    int line_index = 0;
    int offset = 0;
    char* line = DRAGON[line_index];
    int replacements;

    char c;
    int i;

    // Iterate over each line in the DRAGON array
    while (line_index < NUM_DRAGON_LINES) {
        // For each line, parse out and print the required number of characters
        while ((replacements = sscanf(line + offset, "%d%c", &i, &c)) > 0) {

            // Print out the specified number of characters
            for (int j=0; j < i; j++) {

                if (c == 's') fputc(' ', stdout);
                else if (c == 'a') fputc('@', stdout);
                else if (c == 'p') fputc('%', stdout);

            }

            // Increment the offset for each digit scanned in
            for (int k=i; k != 0; k /= 10) {
                offset++;
            }
            offset++; // Increment the offset once more for the character scanned in
        }

        // Advance the line and reset the offset
        line_index++;
        line = DRAGON[line_index];
        offset = 0;

        // Print the newline character in anticipation of the next line
        printf("\n");

    }
}
