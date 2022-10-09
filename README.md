# OrdersBelt
C program created with Threads and MUTEX on Operational Systems lab class at Unisinos

The program is based on the Producer and Consumer paradigm and it simulates the day of a store that takes orders through the app, website, and the actual store and ships them according to the type of shipping (standard or express).

![animation](https://user-images.githubusercontent.com/63256286/194774830-ff5c04d7-2fac-4c30-be82-d33b2da63545.gif)


### Implementação 
* The program was implemented with threads and MUTEX for the critical section control. 
  There are 3 threads used for the creation of the orders, each one is responsible for the creation of an order of different origin (App, website, store)
* The belt, where the orders pass, is treated as a FIFO (First in, First out), and the consumer processes treat the orders according to their shipping (express or standard)
* The program has a handler for the SIGINT interrupt signal, which displays the final statistics and terminates execution.
