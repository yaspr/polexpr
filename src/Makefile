all:
	@rm -rf ../bin
	@mkdir ../bin
	gcc -std=c99 -g3 -O2 polexpr.c -o ../bin/polexpr

test:
	@../bin/polexpr $(E)

clean:
	rm *~
