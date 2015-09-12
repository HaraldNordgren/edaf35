MALLOC	= buddy linked-list
TEST 	= danluu waterloo wisc
GAWK	= 3.1.8 4.1.3

RESULTS	= results.txt

all:
	@for malloc in $(MALLOC); do \
		cd malloc/$$malloc; \
		make; \
		echo; \
		cd $(PWD); done
	@for test in $(TEST); do \
		cd tests/$$test; \
		make 2> /dev/null; \
		echo; \
		cd $(PWD); done
	@for version in $(GAWK); do \
		cd gawk; \
		tar xf gawk-$$version.tar.*; \
		cd gawk-$$version; \
		./configure; \
		make check; \
		sed -i 's/version\.$$(OBJEXT)/& malloc\.$$(OBJEXT)/g' Makefile; \
		sed -i 's/version.c \\/&\n    malloc.c \\/g' Makefile; \
		echo; \
		cd $(PWD); done

check:
	@if [ -f $(RESULTS) ]; then \
		rm $(RESULTS); \
	fi
	@for version in $(GAWK); do \
		for malloc in $(MALLOC); do \
			scripts/gawk-check/make-check $$version malloc/$$malloc; \
			if [ $$? -eq 0 ]; then \
				echo "gawk-$$version with $$malloc malloc SUCCEEDED" >> $(RESULTS); \
			else \
				echo "gawk-$$version with $$malloc malloc FAILED" >> $(RESULTS); \
			fi \
		done \
	done; \
	cat $(RESULTS); \
	rm $(RESULTS);
