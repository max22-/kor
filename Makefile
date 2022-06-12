all: kor

kor: kor.c kor.h main.c
	$(CC) kor.c main.c -o kor -Wall -std=c89

output.img: hello.kor
	./korasm.py hello.kor

.PHONY: run clean

run: kor output.img
	./kor output.img

clean:
	rm -f kor output.img
