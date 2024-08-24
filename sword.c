#include <slib/str.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>
#include <stdarg.h>

#define assert(x) if(!(x)) {fprintf(stderr, "could not assert on line %d file %s\n", __LINE__, __FILE__); exit(EXIT_FAILURE);}

bool lowercase, trimspace, alnum;

void usage() {
	fprintf(stderr, "usage: sword [flags] <word>, tries to find similar words in a file\n");
	fprintf(stderr, "\t-d <number>, sets minimum word distance. 3 by default\n");
	fprintf(stderr, "\t-p <path>, sets path to file.\n");
	fprintf(stderr, "\t\t/usr/local/share/dict/words.txt by default\n");
	fprintf(stderr, "\t-c <delimeter>, char delimeter in file. '\\n' by default\n");
	fprintf(stderr, "\t-i, case insensitive\n");
	fprintf(stderr, "\t-r, get text from stdin\n");
	fprintf(stderr, "\t-a, replaces non alpha numeric chars with spaces\n");
	fprintf(stderr, "\t-ts, trims consecutive spaces\n");
	exit(EXIT_SUCCESS);
}

void die(const char *fmt, ...) {
	va_list ap;
	va_start(ap,fmt);
	vfprintf(stderr, fmt, ap);
	va_end(ap);
	exit(EXIT_FAILURE);
}

void toalnum(sl_str *str) {
	for (size_t i = 0; i < str->len; i++) {
		if (!isalnum(str->data[i]))
			str->data[i] = ' ';
	}
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
			if (matrix[i][j] != insert && matrix[i][j] != delete && is_diagonal(i,j,str->len,str2->len)) {
				if (matrix[i][j]>max)
					return matrix[i][j];
			}
		}
	}
	return matrix[str->len][str2->len];
}

void filter(sl_str *str) {
	if (alnum)
		toalnum(str);
	if (lowercase)
		sl_str_tolower(str);
	if (trimspace)
		sl_str_trim_all(str, ' ');
}

int main(int argc, char **argv) {

	sl_str *path = sl_str_create("/usr/local/share/dict/words.txt");
	assert(path);

	unsigned long int minimum = 3;
	lowercase = false;
	bool usestdin = false;
	trimspace = false;
	alnum = false;
	char delim = '\n';
	int strindex = -1;

	for (int i = 1; i < argc; i++) {
		if (strcmp("-d", argv[i]) == 0) {
			if (argc <= i+1)
				die("missing -d <number>\n");

			minimum=atol(argv[i+1]);
			if (minimum == 0)
				die("-d '%s' is invalid\n", argv[i+1]);

			i++;
			continue;
		}
		if (strcmp("-p", argv[i]) == 0) {
			if (argc<= i+ 1)
				die("missing -p <path>\n");

			assert(!sl_str_set(path, argv[i+1]));
			i++;
			continue;
		}
		if (strcmp("-c", argv[i]) == 0) {
			if (argc <= i+1)
				die("missing -c <char>\n");

			if (strlen(argv[i+1]) != 1)
				die("-c '%s' is invalid\n", argv[i+1]);

			delim = argv[i+1][0];
			i++;
			continue;
		}
		if (strcmp("-i", argv[i]) == 0) {
			lowercase = true;
			continue;
		}
		if (strcmp("-a", argv[i]) == 0) {
			alnum = true;
			continue;
		}
		if (strcmp("-r", argv[i]) == 0) {
			usestdin = true;
			continue;
		}
		if (strcmp("-ts", argv[i]) == 0) {
			trimspace = true;
			continue;
		}
		if (strcmp("-h", argv[i]) == 0) {
			usage();
			continue;
		}
		strindex = i;
	}

	sl_str *word = sl_str_create_cap(32);
	assert(word);
	
	if (strindex == -1 && sl_str_fgetsx2(word, stdin, EOF, 32) == 0 && usestdin == false) {
		// if there is no word in argv
		sl_str_trim(word, '\n');

		if (word->len == 0)
			// if sdin is empty
			die("empty stdin\n");
	}

	if (strindex != -1)
		// if there is word in argv, overwrite stdin
		assert(!sl_str_set(word, argv[strindex]));

	if (strindex == -1 && usestdin == true)
		die("no word in args found\n");

	filter(word);

	FILE *file;
	if (usestdin == true)
		file = stdin;
	else
		file = fopen(path->data, "r");

	if (file == NULL)
		die("could not open FILE '%s'\n", path->data);

	sl_str *w = sl_str_create_cap(32);
	assert(w);

	int status;

	while (true) {

		status = sl_str_fgetsx2(w, file, delim, 32);
		/* status == 0 : found delim
		 * status == 1 : found EOF
		 * stauts == -1 : failed to malloc */

		assert(status != -1);

		if (w->len == 0) {
			if (status == 1)
				break;
			continue;
		}

		filter(w);

		if (w->len > word->len + minimum || word->len > w->len + minimum) {
			sl_str_clear(w);
			continue;
		}

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
	return EXIT_SUCCESS;
}
