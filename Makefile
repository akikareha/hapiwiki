all:
	make -C cgi
	make -C experiment
	make -C wiki

clean:
	make -C cgi clean
	make -C experiment clean
	make -C wiki clean
