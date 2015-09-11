TARGETS = malloc/buddy malloc/linked-list tests/danluu tests/waterloo tests/wisc
GAWK	= 3.1.8 4.1.3

all:
	@for target in $(TARGETS); do \
		cd $$target; \
		make 2> /dev/null; \
		echo; \
		cd $(PWD); done
	@for version in $(GAWK); do \
		cd gawk; \
		tar xf gawk-$$version.tar.*; \
		cd gawk-$$version; \
		./configure; \
		make check; \
		sed -i 's/version\.$$(OBJEXT)/version\.$$(OBJEXT) malloc\.$$(OBJEXT)/g' Makefile; \
		sed -i 's/version.c \\/version.c \\\n    malloc.c \\/g' Makefile; \
		echo; \
		cd $(PWD); done
