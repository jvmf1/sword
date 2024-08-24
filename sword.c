#include <slib/slstr.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

void usage() {
	fprintf(stderr, "usage: sword [flags] <word>, tries to find similar words in a file\n\t-d <number>, sets minimum word distance. 3 by default\n\t-p <path>, sets path to file.\n\t\t/usr/local/share/dict/words.txt by default\n\t-c <delimeter>, char delimeter in file. '\\n' by default\n\t-i, case insensitive\n\t-r, get text from stdin\n");
	exit(1);
}

int main(int argc, char **argv) {

	sl_str *path = sl_str_create("/usr/local/share/dict/words.txt");
	if (path == NULL)
		return -1;

	unsigned long int minimum = 3;
	bool casesensitive = true;
	bool usestdin = false;
	char delim = '\n';
	int strindex = -1;

	for (int i = 1; i < argc; i++) {
		if (strcmp("-d", argv[i]) == 0) {
			if (argc <= i+1) {
				fprintf(stderr, "missing -d <number>\n");
				usage();
			}
			minimum=atol(argv[i+1]);
			if (minimum == 0) {
				fprintf(stderr, "invalid -d <number>\n");
				usage();
			}
			i++;
			continue;
		}
		if (strcmp("-p", argv[i]) == 0) {
			if (argc<= i+ 1) {
				fprintf(stderr, "missing -p <path>\n");
				usage();
			}
			if (sl_str_set(path, argv[i+1]) != 0)
				return -1;
			i++;
			continue;
		}
		if (strcmp("-c", argv[i]) == 0) {
			if (argc <= i+1) {
				fprintf(stderr, "missing -c <char>\n");
				usage();
			}
			if (strlen(argv[i+1]) != 1) {
				fprintf(stderr, "invalid -c <char>\n");
				usage();
			}
			delim = argv[i+1][0];
			i++;
			continue;
		}
		if (strcmp("-i", argv[i]) == 0) {
			casesensitive = false;
			continue;
		}
		if (strcmp("-r", argv[i]) == 0) {
			usestdin = true;
			continue;
		}
		if (strcmp("-h", argv[i]) == 0) {
			usage();
			continue;
		}
		strindex = i;
	}

	sl_str *word = sl_str_create_cap(32);
	if (word == NULL)
		return -1;
	
	if (strindex == -1 && sl_str_fgetsx(word, stdin, EOF, 32) == 0 && usestdin == false) {
		// if there is no word in argv
		sl_str_trim(word, '\n');
		if (word->len == 0) {
			// if sdin is empty
			fprintf(stderr, "empty stdin\n");
			usage();
		}
	}

	if (strindex != -1)
		// if there is word in argv, overwrite stdin
		sl_str_set(word, argv[strindex]);

	if (strindex == -1 && usestdin == true) {
		fprintf(stderr, "no word in args found\n");
		usage();
	}

	if (casesensitive == false)
		sl_str_tolower(word);

	FILE *file;
	if (usestdin == true)
		file = stdin;
	else
		file = fopen(path->data, "r");

	if (file == NULL) {
		fprintf(stderr, "could not open FILE\n");
		usage();
		return -1;
	}

	sl_str *w = sl_str_create_cap(32);
	if (w == NULL)
		return -1;

	while (sl_str_fgetsx(w, file, delim, 32) == 0) {
		if (w->len > word->len + minimum || word->len > w->len + minimum) {
			sl_str_clear(w);
			continue;
		}

		if (casesensitive == false)
			sl_str_tolower(w);

		if (sl_str_distance(w, word) <= minimum)
			printf("%s\n", w->data);
		sl_str_clear(w);
	}
	fclose(file);
	sl_str_free(w);
	sl_str_free(word);
	sl_str_free(path);
}
