# Non-Blocking-Client

The follow application is an implementation of a client from the client-server model. The client follows a custom text protocol that explains how a user can make a request to the server and what the format of the request needs to be in order for the server to give a correct response. Moreover, the client implementation accounts for non-blocking input and output so any writes or reads to and from the server are all non-blocking. Since the reads and writes are non-blocking, we must keep track of where we left off in our read/write when trying to communicate to the server through the socket created to connect the client and server.
<br />
TEXT PROTOCOL for the Client:
<br />
-GET request: The get request allows the client to ask the server for a specific file on the server and get the contents of the file as well. 
To issue a GET request to the server the following protocol for this request must be followed
GET “file name”\n
The response from the server can be of two types either an error message response (the format of this response is uniform for either a GET, PUT, LIST or DELETE request) or an OK response which returns the contents of the file requested
For the error message (uniform for all requests) the following format that is followed for this message is shown below
<br />
Error\n
[“a customized error message based off the request made”]

Otherwise the get request was successful and the response from the server will be as follows
<br />
OK\n
<br />
[File Size][Contents of the File]
<br />
<br />
<br />
-PUT request: The put request allows the client to upload a file to the server. 
To issue a PUT request to the server the following protocol for this request must be followed
<br />
PUT “file name”\n
[File Size][File Contents]
<br />
If the server did not reply with an error response, the put request was successful and the response from the server will be as follows
<br />
OK\n
<br />
<br />
<br />
-LIST request: The list request allows the client to get a list of file names on the server. 
To issue a LIST request to the server the following protocol for this request must be followed
<br />
LIST\n
<br />
If the server did not reply with an error response, the list request was successful and the response from the server will be as follows (message size he refers to how long the message is that contains the list of file names)
<br />
OK\n
[Message Size][List of file names on the server]
<br />
<br />
<br />
-DELETE request: The delete request allows the client to delete a file on the server. 
To issue a DELETE request to the server the following protocol for this request must be followed
<br />
DELETE “filename”\n
<br />
If the server did not reply with an error response, the delete was successful and the response from the server is as follows
<br />
OK\n























