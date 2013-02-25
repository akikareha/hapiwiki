all:
	make -C cgi
	make -C util
	make -C experiment
	make -C wiki

clean:
	make -C cgi clean
	make -C util clean
	make -C experiment clean
	make -C wiki clean
