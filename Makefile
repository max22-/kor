all: kor

kor: kor.c kor.h main.c
	$(CC) kor.c main.c -o kor -Wall -std=c89

.PHONY: run clean

run: kor
	./kor

clean:
	rm -f kor
