#include <slib/slstr.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

void error() {
	fprintf(stderr, "usage: sword [flags] <word>, tries to find similar words in a file\n\t-d <number>, sets minimum word distance.\n\t-p <path>, sets path to wordlist.\n\t\t/usr/local/share/dict/words.txt by default\n\t-c <delimeter>, char delimeter in wordlist. '\\n' by default\n\t-i, case insensitive\n");
	exit(1);
}

int main(int argc, char **argv) {

	sl_str *path=sl_str_create("/usr/local/share/dict/words.txt");
	if (path==NULL)
		return -1;

	long int minimum=-1;
	bool casesensitive=true;


	char delim='\n';
	int strindex =-1;

	for (int i=1;i<argc;i++) {
		if (argv[i][0]=='-') {
			switch (argv[i][1]) {
				case 'd':
					if (argc<=i+1)
						error();
					minimum=atol(argv[i+1]);
					i++;
					break;
				case 'p':
					if (argc<=i+1)
						error();
					if (sl_str_set(path, argv[i+1])!=0)
						return -1;
					i++;
					break;
				case 'c':
					if (argc<=i+1)
						error();
					delim=argv[i+1][0];
					i++;
					break;
				case 'i':
					casesensitive=false;
					break;
				case '\0':
					error();
					break;
				default:
					error();

			}
		} else {
			strindex=i;
		}
	}

	sl_str *word = sl_str_create_cap(64);
	if (word==NULL)
		return -1;
	
	if (strindex==-1 && sl_str_fgetsx(word, stdin, EOF, 32)!=-1) {
		// if there is no entered word 
		sl_str_trim(word, '\n');
		if (word->len==0)
			// if sdin is empty
			error();
	}

	if (strindex!=-1)
		// if there is word, overwrite stdin
		sl_str_set(word, argv[strindex]);

	if (word->len<=5 && minimum==-1) {
		// if there is no -d flag and word is small
		minimum=2;
	}

	if (minimum<0)
		// if there is no -d flag set default value to 3
		minimum=3;

	if (casesensitive==false)
		sl_str_tolower(word);

	FILE *wordlist = fopen(path->data, "r");
	if (wordlist==NULL)
		return -1;

	sl_str *w = sl_str_create_cap(32);
	if (w==NULL)
		return -1;


	while (sl_str_fgetsx(w, wordlist, delim, 32) == 0) {
		if (w->len > word->len + minimum || word->len > w->len + minimum) {
			sl_str_clear(w);
			continue;
		}

		if (casesensitive==false)
			sl_str_tolower(w);

		if ((long int)sl_str_distance(w, word) <= minimum) printf("%s\n", w->data);
		sl_str_clear(w);
	}
	fclose(wordlist);
	sl_str_free(w);
	sl_str_free(word);
}
