FOLDERS = alloc tests/danluu

all:
	for folder in $(FOLDERS); do \
		cd $$folder; \
		make; \
		cd ..; \
	done

clean:
	for folder in $(FOLDERS); do \
		cd $$folder; \
		make clean; \
		cd ..; \
	done
