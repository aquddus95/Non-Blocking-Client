# Non-Blocking-Client

The follow application is an implementation of a client from the client-server model. The client follows a custom text protocol that explains how a user can make a request to the server and what the format of the request needs to be in order for the server to give a correct response. Moreover, the client implementation accounts for non-blocking input and output so any writes or reads to and from the server are all non-blocking. Since the reads and writes are non-blocking, we must keep track of where we left off in our read/write when trying to communicate to the server through the socket created to connect the client and server.

Text Protocol for the Client:

-GET request: The get request allows the client to ask the server for a specific file on the server and get the contents of the file as well.

To issue a GET request to the server the following protocol for this request must be followed

**GET &quot;file name&quot;\n**

The response from the server can be of two types either an error message response (the format of this response is uniform for either a GET, PUT, LIST or DELETE request) or an OK response which returns the contents of the file requested

For the error message (uniform for all requests) the following format that is followed for this message is shown below

**Error\n**

**[&quot;a customized error message based off the request made&quot;]**

Otherwise the get request was successful and the response from the server will be as follows

**OK\n**

**[File Size][Contents of the File]**



-PUT request: The put request allows the client to upload a file to the server.

To issue a PUT request to the server the following protocol for this request must be followed

**PUT &quot;file name&quot;\n**

**[File Size][File Contents]**

If the server did not reply with an error response, the put request was successful and the response from the server will be as follows

**OK\n**

-LIST request: The list request allows the client to get a list of file names on the server.

To issue a LIST request to the server the following protocol for this request must be followed

**LIST\n**

If the server did not reply with an error response, the list request was successful and the response from the server will be as follows (message size he refers to how long the message is that contains the list of file names)

**OK\n**

**[Message Size][List of file names on the server]**



-DELETE request: The delete request allows the client to delete a file on the server.

To issue a DELETE request to the server the following protocol for this request must be followed

**DELETE &quot;filename&quot;\n**

If the server did not reply with an error response, the delete was successful and the response from the server is as follows

**OK\n**


<br />
**Below are examples of how to make the requests for GET, PUT, LIST and DELETE**

./client server:port GET The.Social.Network.2010.1080p.BluRay.x265.10bit-z97.mp4 social\_network.mp4

So in this example we are getting The.Social.Network.2010.1080p.BluRay.x265.10bit-z97.mp4 from the server and saving this file locally as social\_network.mp4

./client server:port PUT prison\_break\_s05\_e01.mp4  Prison.Break.S05E01. mp4

In this example we find the file prison\_break\_s05\_e01.mp4 locally and then create the following put response to the server

PUT prison\_break\_s05\_e01.mp4\n

[File Size][File contents that may start of something like…some call it prison break others call it privilege escalation …the rest of the file contents]

and the server saves this file as Prison.Break.S05E01.mp4

./client server:port DELETE Prison.Break.S05E01.mp4

This example shows how to do a delete request which finds the file Prison.Break.S05E01.mp4 and deletes it from the server

Lastly to do a List request we simply write out the following

./client server:port LIST

Which will then get a response from the server with a list of filenames that are on the server
