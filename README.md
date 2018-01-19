# Client-ServerFTP
An FTP Project that performs basic FTP commands, sending and receiving messages between client and server.

FTP Networking protocol for both client and server side.
Includes:
- Client FTP DataConnection with UNIX System calls as messages sent from
client.
- Server FTP recieves messages from client, interpretting UNIX command
and argument (if applicable). Responds to message sent with outgoing
response sending appropriate three-digit code, determining whether the
command failed (wasn't recognized or invalid) and carrying out
applicable action to the dir/file specified as the argument (where
applicable).

*Entirety of FTP Project for client/server processing was implemented in
the C-language.*
