##
#**************************************************
#Author: Matthew Cheong
#File: Makefile
#Created Date: Thursday, May 3rd 2018, 2:02:59 am
#-----
#Last Modified: Thu May 03 2018
#Modified By: Matthew Cheong
#-----
#**************************************************
##

CC = gcc
CFLAGS = -std=c99 -pthread -D _XOPEN_SOURCE=500 -lrt
OBJ = sds.o
EXEC = sds

$(EXEC): $(OBJ)
	$(CC) $(CFLAGS) $(OBJ) -o $(EXEC)

sds.o: sds.c sds.h
	$(CC) $(CFLAGS) -c sds.c

clean:
	rm -f $(EXEC) $(OBJ)
