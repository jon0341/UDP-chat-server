# chat-server
server that stores user address info and returns it to client upon request for they can intitiate a conversation with another user

-program was developed for/tested on linux/macOS, gcc used to compile.
-to compile: gcc -o chatServer chat_server.c DieWithError.c	
 starting the application from the CLI: ./chatServer < SERVER PORT NUMBER >  (personally I used port 20000)
 
client_server.c provides logs to the console regarding user requests and the server's responses to them, including what addresses
it sends in response to a talk request, the directory sent in response to a who request, and confirmation of what user ID is being searched for and deleted when a user logs out. 

This server program listens via UDP socket for client requests. It receives a client's ip address username, and tcp port they are using for chat purposes. Clients can then perform a "lookup", and the server will return a list of logged in users. If a user requests to talk to a specific client, the following occurs:
	
	1) server receives request
	2) server sends requested user's address info to the initiating client
	3) that client can then intitiate a chat request with the client they looked up
	
If a client chooses to log out, then their information is removed from the directory the server is maintaining, and a confirmation message is sent to them. 
