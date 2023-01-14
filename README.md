# UDP-chat-server
server that stores user address info and returns it to client upon request for they can intitiate a conversation with another user

---------------PROJECT SETUP-----------------
-program was developed/tested for use on google cloud platform, also works in UNIX environments.
-tcp port that clients listen on is hard-coded on line 37 of chat_client.c, if it needs to be changed for use with emunix
-both chat_server.c and chat_client.c must be compiled with DieWithError.c; when I compiled on GCP platform I compiled:

server program: gcc -o chatServer chat_server.c DieWithError.c
client program:  gcc -o chatClient chat_client.c DieWithError.c


***************STARTING THE APPLICATION****************
-using the above example, server is started with ./chatServer < SERVER PORT NUMBER>, personally I used port 20000

-client started with ./chatClient <IP ADDRESS> <PORT NUMBER>, again I used 20000

-on GCP platform, the IP address used was the "internal ip address" listed with the respective virtual machines, NOT the external

-to test the application I utilized three virtual machines, one for the server, one for each client


**************USING THE CLIENT APPLICATION***********************
upon starting, enter a four-digit login ID (eg 0001, 0002, 1111, 9234, etc)

server will take client's address info and login ID, and store it in an array of active users (capacity of 20)

option 1 will allow you to view the current active users on the application

option 2 will allow you to chat with one of the users
	-receiving client is given option to accept or decline the message. if declined, both users are returned to the menu
	-if accepted, the requesting client messages first, and the users can message each other
	-users must take turns sending messages, max size 200 characters (199 plus null terminator)
	-to end the chat, either user sends an end of line character (backslash followed by zero): \0
		-must be only character for that final message
	-after chat ends both users are returned to the menu
	
option 3 will log a user out. The server deletes that user from the user record, and the directory is updated


*************USING THE SERVER APPLICATION***************************
client_server.c provides logs to the console regarding user requests and the server's responses to them, including what addresses
it sends in response to a talk request, the directory sent in response to a who request, and confirmation of what user ID is being searched
for and deleted when a user logs out. 
