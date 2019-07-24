.PHONY: all compress help
all: catbin.exe gbfs.exe padbin.exe bin2s.exe bmp2tiles.exe
compress: all
	upx -9 $^
help:
	@echo Makefile for Pin Eight GBA tools
	@echo by Damian Yerrick
	@echo
	@echo make help: Display this message.
	@echo make all: Build tools.
	@echo make compress: Build tools and compress them with UPX.
	@echo make clean: Remove all executable files.

clean:
	-rm bin2s.exe
	-rm catbin.exe
	-rm padbin.exe
	-rm gbfs.exe
	-rm bmp2tiles.exe

bin2s.exe: bin2s.c
	gcc -Wall -O3 -s bin2s.c -o bin2s.exe

catbin.exe: catbin.c
	gcc -Wall -O3 -s catbin.c -o catbin.exe

gbfs.exe: gbfs.c
	gcc -Wall -O3 -s gbfs.c djbasename.c -o gbfs.exe

padbin.exe: padbin.c
	gcc -Wall -O3 -s padbin.c -o padbin.exe

bmp2tiles.exe: bmp2tiles.c encodetile.c bmp2tiles.h
	gcc -Wall -O3 -s bmp2tiles.c encodetile.c -lalleg -o bmp2tiles.exe
