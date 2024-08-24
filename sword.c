#include <slib/slstr.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

void error() {
	fprintf(stderr, "usage: sword [flags] <word>, tries to find similar words in a file\n\t-d <number>, sets minimum word distance. 3 by default\n\t-p <path>, sets path to wordlist.\n\t\t/usr/local/share/dict/words.txt by default\n\t-c <delimeter>, char delimeter in wordlist. '\\n' by default\n\t-i, case insensitive\n");
	exit(1);
}

int main(int argc, char **argv) {

	sl_str *path=sl_str_create("/usr/local/share/dict/words.txt");
	if (path==NULL)
		return -1;

	unsigned long int minimum=3;
	bool casesensitive=true;


	char delim='\n';
	int strindex =-1;

	for (int i=1;i<argc;i++) {
		if (strcmp("-d", argv[i])==0) {
			if (argc<=i+1)
				error();
			minimum=atol(argv[i+1]);
			if (minimum==0)
				error();
			i++;
			continue;
		}
		if (strcmp("-p", argv[i])==0) {
			if (argc<=i+1)
				error();
			if (sl_str_set(path, argv[i+1])!=0)
				return -1;
			i++;
			continue;
		}
		if (strcmp("-c", argv[i])==0) {
			if (argc<=i+1)
				error();
			delim=argv[i+1][0];
			i++;
			continue;
		}
		if (strcmp("-i", argv[i])==0) {
			casesensitive=false;
			continue;
		}
		strindex=i;
	}

	sl_str *word = sl_str_create_cap(64);
	if (word==NULL)
		return -1;
	
	if (strindex==-1 && sl_str_fgetsx(word, stdin, EOF, 32)!=-1) {
		// if there is no word in argv
		sl_str_trim(word, '\n');
		if (word->len==0)
			// if sdin is empty
			error();
	}

	if (strindex!=-1)
		// if there is word in argv, overwrite stdin
		sl_str_set(word, argv[strindex]);

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

		if (sl_str_distance(w, word) <= minimum)
			printf("%s\n", w->data);
		sl_str_clear(w);
	}
	fclose(wordlist);
	sl_str_free(w);
	sl_str_free(word);
}
