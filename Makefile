.PHONY: all
all: build run

.PHONY: build
build: create.c
	gcc -O2 -g -std=c99 create.c -o create

.PHONY: run
run:
	time ./create
