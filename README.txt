Project 2 for Computing and Software Systems, COMP30023 Semester 1 2016

We were tasked with implementing the game of Mastermind through client server
interaction. The server would the accept the connection requests of up 20 
concurrent clients (an arbitrary limit), perform all game based calculations
sever side and respond to the clients with information to show the game state.
The pthread library was used to create threads to handle each individual 
client. Mutex locks were employed to share common resources and the
proc virtual filesystem was accessed to retrieve performance statistics.