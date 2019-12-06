# Protocoale de comunicatii:
# Laborator 8: Multiplexare
# Makefile

CFLAGS = -Wall -g

# Portul pe care asculta serverul 
PORT = 8008

# Portul pentru client UDP
PORTUDP = 8016

# Adresa IP a serverului si id
IP_SERVER = 127.0.0.1 
ID = JohnSnow

all: server client

# Compileaza server.c
server: server.c -lm

# Compileaza client.c
client: client.c -lm 

.PHONY: clean run_server run_client

# Ruleaza serverul
run_server:
	./server ${PORT}

# Ruleaza clientul
run_client:
	./client ${ID} ${IP_SERVER} ${PORT}

# Ruleaza client UDP
run_udp:
	python3 udp_client.py ${IP_SERVER} ${PORTUDP}

clean:
	rm -f server client
