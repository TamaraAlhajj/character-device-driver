#Tamara Alhajj
#100948027
#COMP 3000 Ex4

obj-m+=tamara_prime.o

all:
	make -C /lib/modules/$(shell uname -r)/build/ M=$(PWD) modules
	cc -o test test.c
run:
	sudo ./test 25
clean:
	make -C /lib/modules/$(shell uname -r)/build/ M=$(PWD) clean
	rm -f *.o *.ko test
