.PHONY: all clean llm

all:
	$(MAKE) -C src

clean:
	$(MAKE) -C src clean

llm:
	$(MAKE) -C src llm
