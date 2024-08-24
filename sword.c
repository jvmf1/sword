#include <slib/slstr.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

void usage() {
	fprintf(stderr, "usage: sword [flags] <word>, tries to find similar words in a file\n");
	fprintf(stderr, "\t-d <number>, sets minimum word distance. 3 by default\n");
	fprintf(stderr, "\t-p <path>, sets path to file.\n");
	fprintf(stderr, "\t\t/usr/local/share/dict/words.txt by default\n");
	fprintf(stderr, "\t-c <delimeter>, char delimeter in file. '\\n' by default\n");
	fprintf(stderr, "\t-i, case insensitive\n");
	fprintf(stderr, "\t-r, get text from stdin\n");
	exit(1);
}

bool is_diagonal(size_t i, size_t j, size_t len, size_t len2) {
	if (i == j)
		return true;
	size_t smaller = len;
	size_t bigger = len;
	if (len2 < smaller)
		smaller = len2;
	if (len2 > bigger)
		bigger = len2;

	if (smaller == len) {
		if (i == smaller && j >= smaller)
			return true;
		return false;
	} else {
		if ( j== smaller && i >= smaller)
			return true;
		return false;
	}
}

size_t levenshtein_distance(sl_str *str, sl_str * str2, size_t max){
	size_t matrix[str->len + 1][str2->len + 1];
	size_t i,j;
	size_t delete, insert, substitute, minimum;
	for (i = 0; i <= str->len; i++) {
		matrix[i][0] = i;
	}
	for (i = 0; i <= str2->len; i++) {
		matrix[0][i] = i;
	}
	for (i = 1; i <= str->len; i++) {

		for (j = 1; j <= str2->len; j++) {

			if (str->data[i-1] == str2->data[j-1]) {
				matrix[i][j] = matrix[i-1][j-1];
			}
			else {

				delete = matrix[i-1][j] + 1;
				insert = matrix[i][j-1] + 1;
				substitute = matrix[i-1][j-1] + 1;
				minimum = delete;
				if (insert < minimum) {
					minimum = insert;
				}
				if (substitute < minimum) {
					minimum = substitute;
				}
				matrix[i][j] = minimum;
			}
			if (is_diagonal(i,j,str->len,str2->len)) {
				if (matrix[i][j]>max)
					return matrix[i][j];
			}
		}
	}
	return matrix[str->len][str2->len];
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

	int status;

	while (true) {

		status = sl_str_fgetsx(w, file, delim, 32);
		/* status == 0 : found delim
		 * status == 1 : found EOF
		 * stauts == -1 : failed to malloc */

		if (status == -1)
			return -1;

		if (w->len == 0) {
			if (status == 1)
				break;
			continue;
		}

		if (w->len > word->len + minimum || word->len > w->len + minimum) {
			sl_str_clear(w);
			continue;
		}

		if (casesensitive == false)
			sl_str_tolower(w);

		if (levenshtein_distance(w, word, minimum) <= minimum)
			printf("%s\n", w->data);

		if (status == 1)
			break;

		sl_str_clear(w);

	}

	fclose(file);
	sl_str_free(w);
	sl_str_free(word);
	sl_str_free(path);
}
