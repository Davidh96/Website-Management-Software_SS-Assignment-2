serverObj = server.o networking.o log.o AccessControl.o
clientObj = client.o networking.o
server: $(serverObj)
	gcc -o server $(serverObj) -pthread

client: $(clientObj)
	gcc -o client $(clientObj)

server.o : server.c
	gcc -c server.c

networking.o: networking.c
	gcc -c networking.c

AccessControl.o: AccessControl.c
	gcc -c AccessControl.c

log.o: log.c
	gcc -c log.c

clean:
	rm server client $(serverObj) client.o
