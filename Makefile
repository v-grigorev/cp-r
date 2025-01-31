mycp: mycp.c func.c
	gcc -Wall -Werror mycp.c func.c -lpthread -o mycp

clean:
	rm -f mycp
