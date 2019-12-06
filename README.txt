The archive contains a Makefile file, source files server.c, client.c, and auxiliaries such as helper.h, etc.

Makefile rules: make, make run_server, make run_client (open client with JohnSnow id), make run_udp, make clean.

Server:
Used UDP port 8016.
The "value" structure, which represents the type of data used in communication through the UDP protocol (contains data, field in which we find the received message and length, message length).
The server will maintain a database with subscribed clients (a vector with "client" elements) and a database with all the received topics (a vector with "topic" elements).
The "client" structure will have fields corresponding to the following properties: the socket the client is on (-1 if this client is not active), on - whether this client is active or not, a database with all the topics it is subscriber (a vector with "subscription" type elements (which contains a topic name, how many messages the client missed during the inactivity period, the messages to be received by the client and the SF flag for that topic), the number of topics to which his ID is also subscribed.
The "topic" structure will have a name field and the number of messages for that topic.
The server will check 3 cases:
- the case in which it receives from the corresponding UDP data socket; will check incoming messages, create new topics, if needed, and send all subscribers who are active, the appropriate messages;

- the case in which it receives commands from the keyboard: at the "exit" command, the program will close, otherwise, the typed command will be displayed next to an error message (these displays are commented, as it has been requested to deactivate them in the request) ;

- the case in which a client connects; he will be asked for the id, depending on which one will check if he is a client already existing in the database or to be added; then check if it has received messages from its inactive period, or if it subscribes or unsubscribes to a topic that may exist in the database or not;

Customer:
The "decrypt" function that handles the proper display of the message received from the server (corresponding display INT, SHORT_REALT, etc.).