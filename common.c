#include "common.h"
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
// The following function reads from the socket until a newline character is hit
// Since the client application is dealing with non-blocking input we have to return the amount of characters read 
size_t read_newline(int socket, char *buffer) {

        errno = 0;
        size_t total= 0;
        int rsent;

        while(*buffer != '\n') {
                     
                rsent = read(socket,buffer,1);
               
                if(rsent == 0){
                return total;
                }

                if(rsent == -1 && errno != EINTR)
                        return -1;

        		if(*buffer == '\n'){
        		break;		
        		}
    		


                if( rsent > 0){
                buffer+=rsent;
                total+=rsent;
                }

        }
        return total;
}
// The following is a helper function used to read a certain number of characters (limit set by 'count')
ssize_t read_all_from_socket(int socket, char *buffer, size_t count) {

        
        errno = 0;
        size_t total= 0;
        int rsent;
        size_t len=count;

        while(total < count) {	
                rsent = read(socket,buffer,len-total);

                if(rsent == 0){
                return total;
                }
                
                if(rsent == -1 && errno != EINTR)
                        return -1;
                
                if( rsent > 0){
                buffer+=rsent;
                total+=rsent;
                }
        
        }
        return total;

}
// The following is a helper function used to write a certain number of characters (limit set by 'count')
ssize_t write_all_to_socket(int socket, const char *buffer, size_t count) {

	errno = 0;
	size_t total= 0;
	int  sent;
	size_t len=count;
		
	while(total < count) {

		sent = write(socket,buffer,len-total);

		if(sent == 0){
		return total;		
		}

		if(sent == -1 && errno != EINTR)
			return -1;

		if( sent > 0){		
		buffer+=sent;
		total+=sent;
		}

	}
	return total;


}
// The following two helper functions are used to find out message size that is to be read or written to/from the socket according to the text protocol mentioned in the Readme 
size_t get_message_size(int socket) {
    size_t size;
    ssize_t read_bytes = read_all_from_socket(socket, (char *)&size, sizeof(size_t));
    if (read_bytes == 0 || read_bytes == -1)
        return read_bytes;
    return size;
}

size_t write_message_size(size_t size, int socket) {	
   size_t read_bytes = write_all_to_socket(socket, ((char *)&size), 8);	 
   if (read_bytes == 0 || read_bytes == -1)
        return read_bytes;
   return read_bytes;	

}

