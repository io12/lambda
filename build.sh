#!/bin/sh
CFLAGS="-g -O0 -std=c99 -pedantic -Wall -Wextra"
LDFLAGS="-lreadline"
cc $CFLAGS $LDFLAGS *.c -o lambda
