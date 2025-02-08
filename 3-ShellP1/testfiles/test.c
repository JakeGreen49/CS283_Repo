#include <stdio.h>
#include <stdlib.h>
#include <string.h>



int main(){
	size_t line_buffsz = 0;
   	char* line = NULL;    
	FILE* fp = fopen("dragon_output", "r");
	ssize_t line_size;
	int line_num = 1;

	size_t sp_count = 0;
	size_t at_count = 0;
	size_t pc_count = 0;

	while ((line_size = getline(&line, &line_buffsz, fp)) != -1) {
		printf("Line %d:", line_num);

		for (int i=0; i < line_buffsz; i++) {

			if (i != 0) {
				// Space character
				if (line[i] == ' ' && line[i-1] == ' ') {
					sp_count++;
					if (line[i + 1] != ' ') {
						printf("%lds", sp_count);
						sp_count = 0;
					}
				}
				else if (line[i] == ' ' && line[i-1] != ' ') {
					sp_count++;
					if (line[i + 1] != ' ') {
						printf("%lds", sp_count);
						sp_count = 0;
					}
				}

				// At sign
				if (line[i] == '@' && line[i-1] == '@') {
					at_count++;
					if (line[i + 1] != '@') {
						printf("%lda", at_count);
						at_count = 0;
					}
				}
				else if (line[i] == '@' && line[i-1] != '@') {
					at_count++;
					if (line[i + 1] != '@') {
						printf("%lda", at_count);
						at_count = 0;
					}
				}

				// Percent sign
				if (line[i] == '%' && line[i-1] == '%') {
					pc_count++;
					if (line[i + 1] != '%') {
						printf("%ldp", pc_count);
						pc_count = 0;
					}
				}
				else if (line[i] == '%' && line[i-1] != '%') {
					pc_count++;
					if (line[i + 1] != '%') {
						printf("%ldp", pc_count);
						pc_count = 0;
					}
				}

			}
			else {
				if (line[i] == ' ') sp_count++;
				else if (line[i] == '@') at_count++;
				else if (line[i] == '%') pc_count++;
			}
		}
	
		printf("\n");	
		line_num++;
	}

    printf("\n");
}
