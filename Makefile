CC = clang
CFLAGS = -Wall -g -DDEBUG
LIBS = -ly

PRG = c--
OBJS = parser.o lexer.o global.o syntree.o env.o stab.o trie.o

$(PRG): $(OBJS)
	$(CC) $(CFLAGS) $(LIBS) -o $@ $^

lexer.h lexer.c: lexer.l
	flex $<
parser.h parser.c: parser.y
	bison $<
parser.o: global.h lexer.h env.h syntree.h stab.h
lexer.o: global.h parser.h env.h stab.h
global.o: global.h stab.h
syntree.o: syntree.h global.h lexer.h
env.o: env.h trie.h stab.h
stab.o: stab.h
trie.o: trie.h stab.h
.PHONY: clean
clean:
	$(RM) $(PRG) $(OBJS) parser.h parser.c lexer.h lexer.c