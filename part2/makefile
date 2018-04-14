prodcon:	main.c producer.c consumer.c 
	rm -f Producer_* Consumer.txt
	gcc -pthread -o producer producer.c
	gcc -pthread -o consumer consumer.c
	gcc -pthread -o prodcon main.c
