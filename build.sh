set -e
gcc -std=c11 -O2 -Wall -Wextra firstParse.c hashmap.c list.c -o hw5-asm -lm
gcc -std=c11 -O2 -Wall -Wextra Converter.c -o hw5-sim -lm
