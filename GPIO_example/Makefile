CC=gcc
BINDIR=./bin
SRCDIR=./src

all : createdir $(BINDIR)/exec

$(BINDIR)/exec : $(BINDIR)/top.o $(BINDIR)/lex.yy.o  $(BINDIR)/parser.tab.o $(SRCDIR)/main.c
	$(CC) $^ -o $@ -lreadline 

$(BINDIR)/top.o : $(SRCDIR)/top.c  $(SRCDIR)/top.h
	$(CC) -c $< -o $@


$(BINDIR)/parser.tab.o: $(SRCDIR)/parser.tab.c  $(SRCDIR)/parser.tab.h
	$(CC) -c  $< -o $@ -lfl

$(BINDIR)/lex.yy.o  :  $(SRCDIR)/lex.yy.c
	$(CC) -c  $< -o $@ -lfl

$(SRCDIR)/lex.yy.c :
	flex $(SRCDIR)/parser.l

$(SRCDIR)/parser.tab.c $(SRCDIR)/parser.tab.h :
	bison -d $(SRCDIR)/parser.y

createdir :
	mkdir -p $(BINDIR)

clear :
	rm -r bin
	rm $(SRCDIR)/parser.tab.c
	rm $(SRCDIR)/parser.tab.h
	rm $(SRCDIR)/lex.yy.c
