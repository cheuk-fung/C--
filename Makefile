CC = clang
CFLAGS = -Wall -g -DNGDEBUG
LIBS = -ly

PRG = c--
OBJS = parser.o lexer.o syntree.o env.o stab.o trie.o asm.o

$(PRG): $(OBJS)
	$(CC) $(CFLAGS) $(LIBS) -o $@ $^

lexer.h lexer.c: lexer.l
	flex $<
parser.h parser.c: parser.y
	bison $<
parser.o: global.h lexer.h env.h syntree.h stab.h asm.h
lexer.o: global.h parser.h env.h stab.h
syntree.o: syntree.h global.h lexer.h
env.o: env.h trie.h stab.h
stab.o: stab.h env.h
trie.o: trie.h stab.h
asm.o: asm.h syntree.h
.PHONY: clean
clean:
	$(RM) $(PRG) $(OBJS) parser.h parser.c lexer.h lexer.c a.out a.out.s msg.out

show-qsort:
	./c-- < sample/qsort.c
	vim a.out.s

run-qsort:
	./c-- < sample/qsort.c
	./a.out
