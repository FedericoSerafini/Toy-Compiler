all:
	make RE_parser

SOURCES = RE_parser.c

RE_parser: $(SOURCES)
	gcc -Wall -Wextra -O0 -g -fsanitize=address $(SOURCES) -o RE_parser

clean :
	rm -f RE_parser

