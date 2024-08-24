# sword
tool for finding similar word in a file
# install
install [jvmf1/slib](https://github.com/jvmf1/slib)
```
git clone https://github.com/jvmf1/sword
cd sword
sudo make install
```
# usage
```
usage: sword [flags] <word>, tries to find similar words in a file
	-d <number>, sets minimum word distance. 3 by default
	-p <path>, sets path to wordlist.
		/usr/local/share/dict/words.txt by default
	-c <delimeter>, char delimeter in wordlist. '\n' by default
	-i, case insensitive
```
# examples
```sh
$ sword -p /path/to/wordlist successfull
insuccessful
succesful
successful
successfully
successoral
unsuccessful
unsuccessfully
# you can set a distance with -d flag
$ sword -p /path/to/wordlist -d 1 successfull
succesful
successful
successfully
# you can also pipe into sword
echo 'word' | sword
```
