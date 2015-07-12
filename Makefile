TARGETS = malloc/buddy malloc/linked-list tests/danluu tests/waterloo tests/wisc
GAWK	= 3.1.8 4.1.3

all:
	@for target in $(TARGETS); do \
		cd $$target; \
		make; \
		echo; \
		cd $(PWD); done
	@for version in $(GAWK); do \
		cd gawk; \
		tar xf gawk-$$version.tar.bz2; \
		cd gawk-$$version; \
		./configure; \
		echo; \
		cd $(PWD); done
