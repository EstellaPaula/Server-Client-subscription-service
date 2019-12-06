PROTOCOALE DE COMUNICATIE
	Tema #2 

In arhiva se afla un fisier Makefile, fisierele sursa server.c, client.c, si auxiliare precum helper.h, etc.

Makefile rules :  make, make run_server, make run_client (deschide client cu id JohnSnow), make run_udp,  make clean.

Server :
	Port UDP folosit 8016.
	Structura "value", care reprezinta tipul de date folosit in comunicare prin protocolul UDP(contine data,camp in care gasim mesajul primit si length, lugimea mesajului).
	Serverul va tine o baza de date cu clientii abonati (un vector cu elemente de tip "client") si o baza de date cu toate topics-urile primite (un vector cu elemente de tip "topic").
	Structura "client" va tine campuri corespunzatoare urmatoarelor proprietati: socketul pe care se afla clientul (-1 daca acest client nu este activ), on - daca acest client este activ sau nu, o baza de date cu toate topics-urile la care este abonat (un vector cu elemente de tip "abonament"(care contine un numele topic-ului, cate mesaje a ratat clientul in perioada de inactivitate, mesajele ce trebuie primite de catre client si flagul SF pentru respectivul topic), numarul de topics la care este abonat si id-ul sau. 
	Structura "topic" va avea un camp de nume si numarul de mesaje pentru acel topic.
	Serverul va verifica 3 cazuri :
		-cazul in care primeste de pe socket-ul corespunzator UDP date; va verifica mesajele primite, va crea topic-uri noi, daca este nevoie, si va trimite la toti clientii abonati care sunt activi, mesajele corespunzatoare;

		-cazul in care primeste comenzi de la tastatura : la comanda "exit" , programul se va inchide, in caz contrar, comanda tastata se va afisa alaturi de un mesaj de eroare (aceste afisari sunt comentate, intrucat s-a cerut dezactivarea lor in cerinta);

		-cazul in care se conecteaza un client; acestuia i se va interoga id-ul, in functie de care se va verifica daca acesta este un client deja existent in baza de date sau care trebuie adaugat; apoi se verifica daca acesta are de primit mesaje din perioada sa inactiva, sau daca se aboneaza sau dezaboneaza la un topic care poate sa exite in baza de date sau nu;

Client : 
	Functia "decrypt" care se ocupa cu afisarea corespunzatoare a mesajului primit de la server(afisare corespunzatoare INT, SHORT_REALT, etc).
	Clientul va verifica 2 cazuri :
		-cazul in care primeste de la server mesaje; daca acestea semnaleaza erori se va afisa mesajul specific pe ecran, in caz contrar se va afisa mesajul primit;
		-cazul in care primeste comenzi de la tastatura, ce vor fi verificate inainte de a fi trimise catre server; 

!!!De mentionat sunt cazurile de exceptie tratate si faptul ca nu am optat pentru dezactivarea algoritmului Neagle, intrucat la testare nu am intampinat intarzieri ale mesajelor. De asemenea mesajele primite care trebuie retinute in memorie, sunt retinute static (intrucat nu s-a mentionat scalabilitatea programului, deci exista un numar limitat de mesaje care pot fi retinuti in baza de date), in server, cele care nu mai sunt necesare fiind eliminate din memorie sau suprascrise, zona de memorie fiind refolosita.

Cazuri de exceptie tratate de catre aplicatie (afisarile acestora au fost comentate intrucat acest lucru se specifica in cerinta) :
	-cazul in care un client incearca se se logheze cu un id al unui client care este activ la momentul acela
	-cazul in care comenzile subscribe/unsubscribe nu au toate argumentele prezente
	-cazul in care flag-ul SF are alta valoare decat 1 sau 0
	-cazul in care un client incearca sa se aboneze la un topic inexistent
	-cazul in care un client incearca sa se dezaboneze de la un topic la care nu era abonat
	-cazul in care de la tastatura se incearca a fi date comenzi care nu exista (caz verificat atat pentru server, cat si pentru client);

	Negoita Estella-Paula 323 CD