**Project 2 for Computing and Software Systems, COMP30023 Semester 1 2016**

**Overview:**
  We were tasked with implementing the game of Mastermind through client server
  interaction. The server would the accept the connection requests of up 20 
  concurrent clients (an arbitrary limit), perform all game based calculations
  server side and respond to the clients with information to show the game state.
  The pthread library was used to create threads to handle each individual 
  client. Mutex locks were employed to share common resources and the
  proc virtual filesystem was accessed to retrieve performance statistics.

**Usage:**
  You will need a Unix/Linux operating system. 
  
  Clone the repository.
  
  Navigate in terminal to the directory.
  
  Run the terminal command "make", which compiles the .c files into the object files
  and executables.
  
  Execute server, passing a port number as the first argument in the terminal.
  
  Execute client, passing the ip address of the server as the first argument 
  (localhost if you have just opened up a new terminal on the same machine) 
  and the same port number as passed to the server as the second argument.
  
  You can now play mastermind, 
  see `here	<https://en.wikipedia.org/wiki/Mastermind_(board_game)>`_ for the rules.
  
  Having played until your heart's content, you may terminate the server with ctr-C,
  which will cause performance metrics to be written to the end of log.txt, which has
  been recording the client-server interactions as you've played.
  
