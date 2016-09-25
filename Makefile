all: master slave
master: master.c
	gcc -o master master.c
slave: slave.c
	gcc -o slave slave.c
clean:
	rm -f *.o
cleanall:
	rm -f *.o master slave
checkin:
	git add *
	git commit -m "$m"
	git push origin master

