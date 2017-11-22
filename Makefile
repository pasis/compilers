.PHONY: c doc

all: c

c:
	make -C ./src

doc:
	make -C ./doc
