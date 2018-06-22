#include "common.h"
#include "format.h"
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netdb.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <ctype.h>



char **parse_args(int argc, char **argv);

verb check_args(char **args);

static volatile int serverSocket;

struct addrinfo *result;
int sok=-2; //default value of socket descriptor set to -2 which is changed after creating the socket



// The following function closes the socket connection
/***************************************************************************************/
void close_server_connection() {
  if(result != NULL)
  freeaddrinfo(result);
  close(sok);   	
  exit(0);      	
}
/***************************************************************************************/
//The following function creates the socket and then makes the connection to the socket 
/***************************************************************************************/
int connect_to_server(const char *host, const char *port) {
  int err;
  int sock_fd= socket(AF_INET, SOCK_STREAM,0);
  	if(sock_fd == -1){
  	perror(NULL);
  	exit(1);
  	}
  sok=sock_fd;
  struct addrinfo hints;
  memset(&hints,0, sizeof(hints));
  hints.ai_family= AF_INET;
  hints.ai_socktype= SOCK_STREAM;
  err= getaddrinfo(host,port,&hints,&result);
  	if( err != 0){
  	fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(err));
          exit(1);
  	}

  int ok= connect(sock_fd,result->ai_addr,result->ai_addrlen);
  	if(ok == -1){
  	perror(NULL);
  	exit(1);
  	}

    return sock_fd;
}
/***************************************************************************************/
//The following function is responsible for creating the get request from the client side and then processing the feeback from the server after making the get request to the server
// the text protcol for the get function is explained in the Readme 
/***************************************************************************************/
void getfunc(char** args){


  char* gt= "GET ";
  char* nl= "\n";
  char* filename=args[3];

  //Intial set up for the get message which is formatted by following the text protocol:
  // "GET filename \n" 
  char* msg= malloc(strlen(gt) + strlen(filename)+ strlen(nl)+1);
  strcpy(msg,gt);
  strcat(msg,filename);
  strcat(msg,nl);

  size_t count= strlen(msg);

  // Send get request over to socket so the server can read the message and process accordingly
  write_all_to_socket(serverSocket,msg,count);
  free(msg);
  // No need to write anything to the socket after writing the get request to stop client from writing any other messages over the socket
  shutdown(serverSocket, SHUT_WR);

     
  ssize_t ret;
  char* buff=calloc(1024,sizeof(char));
  int i= 0;
  int count1=0;



  // read from socket one byte at a time until we hit a newline indicating the end of the response from the server
  // have to read one byte at a time since our reads from the socket are non-blocking
  	while(1){
      ret=read_all_from_socket(serverSocket,&(buff[i]),1);

  		if(ret == 0){
        print_invalid_response();
        free(buff);
        shutdown(serverSocket, SHUT_RD);
        return;
      }
  		
  		count1++;

      if(buff[i] == '\n')
        break;
      
      i++;
      }

      // make sure we read a valid response from the server into our buffer
    if(count1 < 2){
      print_invalid_response();
      free(buff);
      shutdown(serverSocket, SHUT_RD);
      return;
    }


  char c1=buff[0];
  char c2=buff[1];

    // check if the server sent an error message as a response
  	if( c1 == 'E' && c2 == 'R'){
    	int idx=0;
    	memset(buff,0,1024);

    	        while(1){
              //Need to read all of the error message until we hit a newline
            	read_all_from_socket(serverSocket,&(buff[idx]),1);
                  if(buff[idx] == '\n')
                    break;
                  idx++;
            	}

    	print_error_message(buff);
    	free(buff);
    	shutdown(serverSocket, SHUT_RD);
    	return;
  	}

     // make sure we have an OK response if we did not get an error response
  	else if( c1 != 'O' && c2 != 'K'){
  	print_invalid_response();
  	free(buff);
  	shutdown(serverSocket, SHUT_RD);
  	return;
  	// malformed
  	}


  //Open a file descriptor to create file name locally that is being retrieved from the server
  int fd=open(args[4],O_CREAT|O_TRUNC|O_RDWR,S_IRUSR|S_IWUSR );

  // get the file size which is specified in the server response to a get request
  size_t msglen= get_message_size(serverSocket);

  memset(buff,0,1024);
  ssize_t rval;

  	if(msglen <= 1024){
  	rval=read_all_from_socket(serverSocket,buff,msglen);

      // if we cannot read the message from the socket we exit out and report error with reading the server's response
  		if( rval == 0){
  		remove(args[4]);	
  	        print_too_little_data();		
  		close(fd);
          	shutdown(serverSocket, SHUT_RD);
          	free(buff);
          	return;
  		}	

  	write(fd,buff,msglen);
  	close(fd);

  		char ch;
  		if(recv(serverSocket,&ch,1,MSG_PEEK)){
  		print_recieved_too_much_data();
  		}


  	shutdown(serverSocket, SHUT_RD);
  	free(buff);
  	return;
  	} 


  // following code deals with reading the message and accounting for the non-blocking read so we may have no have read all of the message on our first read to the socket
  size_t totalread=msglen;
  ssize_t rval1;
  ssize_t rval2;


  while(totalread > 0){

  	if(totalread < 1024){
  	rval1=read_all_from_socket(serverSocket,buff,totalread);

      if(rval1 == 0){
  	     remove(args[4]);
         print_too_little_data();
  	     close(fd);
         shutdown(serverSocket, SHUT_RD);
         free(buff);
         return;// anything else?
        }
          

  	write(fd,buff,rval1);
  	totalread-=rval1;
  	memset(buff,0,1024);
  	}


  	else{
  	rval2=read_all_from_socket(serverSocket,buff,1024);
         
  	if(rval2 == 0){
          remove(args[4]);
          print_too_little_data();
          close(fd);
          shutdown(serverSocket, SHUT_RD);
          free(buff);
          return;
  	}


  	write(fd,buff,rval2);
  	totalread-=rval2;
  	memset(buff,0,1024);
  	}

  }


  char ch;
  if(recv(serverSocket,&ch,1,MSG_PEEK)){
  print_recieved_too_much_data();
  }


   shutdown(serverSocket, SHUT_RD);
   free(buff);
   return;
  //clean up 

}
/***************************************************************************************/
//The following function is responsible for creating the put request from the client side and then processing the feeback from the server after making the put request to the server
// the text protcol for the put function is explained in the Readme 
/***************************************************************************************/
void putfunc(char** args){

  // Set up put message that is to be written to the socket
  // PUT filename \n
  // [File Size ][File contents]  
  char* pt= "PUT ";
  char* nl= "\n";
  char* filename=args[3];
  size_t l1=strlen(pt) + strlen(filename)+ strlen(nl)+1;
  char* msg= calloc(1,l1);
  strcpy(msg,pt);
  strcat(msg,filename);
  strcat(msg,nl);
  size_t count= strlen(msg);   
  write_all_to_socket(serverSocket,msg,count);
  free(msg);

  	

  //opening a descriptor to the file that is being uploaded to the socket	
  int fd=open(args[4],O_RDWR,S_IRUSR|S_IWUSR );
   	if(fd< 0){
          shutdown(serverSocket, SHUT_RD); 
          return;
          }


  // find the out the file size that is to be written to the socket for the server to upload
  struct stat st;
  stat(args[4],&st);
  size_t len=(size_t )st.st_size;

  // write the message size that follows the newline in the Put Request and then afterwards the content of the file
  write_message_size(len,serverSocket);

  size_t msglen=len;
  char* buff= calloc(1024,sizeof(char));
  memset(buff,0,1024);

  	if(msglen <= 1024){

          read(fd,buff,msglen);
          write_all_to_socket(serverSocket,buff,msglen);
  	      shutdown(serverSocket, SHUT_WR);
          close(fd);
  	}


    // if message length is greater than 1024 bytes then we must account for the fact that the writes to the socket are non-blocking and must update totalread as the number of bytes from the message that have been written to the socket
  	else{
  	size_t totalread=msglen;
  	ssize_t rval1;
  	ssize_t rval2;
  		while(totalread > 0){

          		if(totalread < 1024){
          		rval1=read(fd,buff,totalread);
          		write_all_to_socket(serverSocket,buff,rval1);
          		totalread-=rval1;
          		memset(buff,0,1024);
          		}


  			else{
  			rval2=read(fd,buff,1024);
  			write_all_to_socket(serverSocket,buff,rval2);
  			totalread-=rval2;
  			memset(buff,0,1024);
  			}

  		}

     	shutdown(serverSocket, SHUT_WR);
          close(fd);
  	}


  // read the response from the server out of the socket to see if we have correctly executed a put request the server
  int idx= 0;
  int count1=0;
  ssize_t ret2;
  memset(buff,0,1024);

         while(1){
                  ret2=read_all_from_socket(serverSocket,&(buff[idx]),1);
  						
                  if(ret2 == 0){ 
  		print_invalid_response();
          	free(buff);
          	shutdown(serverSocket, SHUT_RD);
                  }
                 
                  count1++;
                  if(buff[idx] == '\n')
                  break;
                  idx++;

          }

          // if response is less than two characters we have an invalid response
  	if(count1 < 2){
  	print_invalid_response();
  	free(buff);
  	shutdown(serverSocket, SHUT_RD);
  	return;
  	}


  char c1=buff[0];
  char c2=buff[1];

    // check if server sent an error response
  	if( c1 == 'E' && c2 == 'R'){

  	int idx2=0;
  	int count2=0;
  	memset(buff,0,1024);

          	while(1){
                 		read_all_from_socket(serverSocket,&(buff[idx]),1);
                 		count2++;
                 		if(buff[idx] == '\n')
                 		break;
                 		idx2++;
          		}

  	print_error_message(buff);
  	free(buff);
  	shutdown(serverSocket, SHUT_RD);
  	return;
  	}


    // make sure we have an OK response if we did not get an error response
  	else if( c1 != 'O' && c2 != 'K'){
  	print_invalid_response();
  	free(buff);
  	shutdown(serverSocket, SHUT_RD);
  	return;
  	}



  free(buff);
  shutdown(serverSocket, SHUT_RD);
  return;
}
/***************************************************************************************/
//The following function is responsible for creating the list request from the client side and then processing the feeback from the server after making the list request to the server
// the text protcol for the list function is explained in the Readme 
/***************************************************************************************/
void list(){
  // Prepare a List request to the server
  // LIST\n
  char* pt= "LIST";
  char* nl= "\n";
  size_t l1=strlen(pt) +  strlen(nl)+1;
  char* msg= calloc(1,l1);
  strcpy(msg,pt);
  strcat(msg,nl);

  size_t count= strlen(msg); 
  write_all_to_socket(serverSocket,msg,count);

  free(msg);
  shutdown(serverSocket, SHUT_WR);


  // get response from the server by reading the socket
  ssize_t ret;
  char* buff=calloc(1024,sizeof(char));
  int i= 0;
  int count1=0;

          while(1){
                  ret=read_all_from_socket(serverSocket,&(buff[i]),1);

                  if(ret == 0){
                  print_invalid_response();
                  free(buff);
                  shutdown(serverSocket, SHUT_RD);
                  return;
                  }

                  count1++;
                  if(buff[i] == '\n')
                  break;
                  i++;
          }


          if(count1 < 2){
          print_invalid_response();
          free(buff);
          shutdown(serverSocket, SHUT_RD);
          return;
          }

  // Check if the response from the server is an error message
  char c1=buff[0];
  char c2=buff[1];

          if( c1 == 'E' && c2 == 'R'){
          int idx=0;
          memset(buff,0,1024);

                  while(1){
                  read_all_from_socket(serverSocket,&(buff[idx]),1);
                  if(buff[idx] == '\n')
                  break;
                  idx++;
                  }

          print_error_message(buff);
          free(buff);
          shutdown(serverSocket, SHUT_RD);
          return;
          }

           // make sure we have an OK response if we did not get an error response
          else if( c1 != 'O' && c2 != 'K'){
          print_invalid_response();
          free(buff);
          shutdown(serverSocket, SHUT_RD);
          return;
          // malformed
          }



  size_t msglen= get_message_size(serverSocket);
  memset(buff,0,msglen);
  ssize_t rval;

          
          rval=read_all_from_socket(serverSocket,buff,msglen);

                  if( rval == 0){
                  print_too_little_data();
                  shutdown(serverSocket, SHUT_RD);
                  free(buff);
                  return;
                  }

          write(1,buff,msglen);

                  char ch;
                  if(recv(serverSocket,&ch,1,MSG_PEEK)){
                  print_recieved_too_much_data();
                  }


  shutdown(serverSocket, SHUT_RD);
  free(buff);
  return;
}

/****************************************************************************************/
//The following function is responsible for creating the delete request from the client side and then processing the feeback from the server after making the get request to the server
// the text protcol for the delete function is explained in the Readme 
/***************************************************************************************/
void delete(char** args){
  // setup Delete request to remove a file from the server
  char* gt= "DELETE ";
  char* nl= "\n";
  char* filename=args[3];
  char* msg= malloc(strlen(gt) + strlen(filename)+ strlen(nl)+1);
  strcpy(msg,gt);
  strcat(msg,filename);
  strcat(msg,nl);

  size_t count= strlen(msg);
  write_all_to_socket(serverSocket,msg,count);
  free(msg);
  shutdown(serverSocket, SHUT_WR);



  // read the response from the server
  ssize_t ret;
  char* buff=calloc(1024,sizeof(char));
  int i= 0;
  int count1=0;

          while(1){
                  ret=read_all_from_socket(serverSocket,&(buff[i]),1);

                  if(ret == 0){
                  print_invalid_response();
                  free(buff);
                  shutdown(serverSocket, SHUT_RD);
                  return;
                  }

                  count1++;
                  if(buff[i] == '\n')
                  break;
                  i++;
          }


          if(count1 < 2){
          print_invalid_response();
          free(buff);
          shutdown(serverSocket, SHUT_RD);
          return;
          }

  // check if the response from the server is an error message
  char c1=buff[0];
  char c2=buff[1];

          if( c1 == 'E' && c2 == 'R'){
          int idx=0;
          memset(buff,0,1024);

                  while(1){
                  read_all_from_socket(serverSocket,&(buff[idx]),1);
                  if(buff[idx] == '\n')
                  break;
                  idx++;
                  }

          print_error_message(buff);
          free(buff);
          shutdown(serverSocket, SHUT_RD);
          return;
          }

           // make sure we have an OK response if we did not get an error response
          else if( c1 != 'O' && c2 != 'K'){
          print_invalid_response();
          free(buff);
          shutdown(serverSocket, SHUT_RD);
          return;
          // malformed
          }


   shutdown(serverSocket, SHUT_RD);
   free(buff);
   return;
}

/******************************************************************************************/
// The following is the main for the client code
/******************************************************************************************/

int main(int argc, char **argv) {

  //Parse the arguments given since either arguments from a GET, PUT, LIST or DELETE will be given depending on the request and its protocol which is explained in the Readme
  char** par=parse_args(argc,argv);
  if (par == NULL){
  return 0;
  }


  // connect to the server using the connect_to_server helper function
  serverSocket = connect_to_server(par[0], par[1]);
  sok=serverSocket;
  verb var;
  // parse the verb which relates to the type of request the user wants to make with the client
  var=check_args(par);




  if(var == GET){
  getfunc(par);
  }

  else if(var== PUT){
  putfunc(par);
  }


  else if(var== LIST){
  list();
  }


  else if(var== DELETE){
  delete(par);
  }

  close_server_connection();
  free(par);
  return 0;

}

/**
 * Given commandline argc and argv, parses argv.
 *
 * argc argc from main()
 * argv argv from main()
 *
 * Returns char* array in form of {host, port, method, remote, local, NULL}
 * where `method` is ALL CAPS
 */
char **parse_args(int argc, char **argv) {
  if (argc < 3) {
    return NULL;
  }

  char *host = strtok(argv[1], ":");
  char *port = strtok(NULL, ":");
  if (port == NULL) {
    return NULL;
  }

  char **args = calloc(1, (argc + 1) * sizeof(char *));
  args[0] = host;
  args[1] = port;
  args[2] = argv[2];
  char *temp = args[2];
  while (*temp) {
    *temp = toupper((unsigned char)*temp);
    temp++;
  }
  if (argc > 3) {
    args[3] = argv[3];
  } else {
    args[3] = NULL;
  }
  if (argc > 4) {
    args[4] = argv[4];
  } else {
    args[4] = NULL;
  }

  return args;
}

/**
 * Validates args to program.  If `args` are not valid, help information for the
 * program is printed.
 *
 * args     arguments to parse
 *
 * Returns a verb which corresponds to the request method
 */
verb check_args(char **args) {
  if (args == NULL) {
    print_client_usage();
    exit(1);
  }

  char *command = args[2];

  if (strcmp(command, "LIST") == 0) {
    return LIST;
  }

  if (strcmp(command, "GET") == 0) {
    if (args[3] != NULL && args[4] != NULL) {
      return GET;
    }
    print_client_help();
    exit(1);
  }

  if (strcmp(command, "DELETE") == 0) {
    if (args[3] != NULL) {
      return DELETE;
    }
    print_client_help();
    exit(1);
  }

  if (strcmp(command, "PUT") == 0) {
    if (args[3] == NULL || args[4] == NULL) {
      print_client_help();
      exit(1);
    }
    return PUT;
  }

  // Not a valid Method
  print_client_help();
  exit(1);
}





