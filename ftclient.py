"""
Name : Alexandra Dohm
Program : Server Side TCP FTP transfer, Project 2
Description : This code creates a client using user specified inputs.
 Sends either -l or -g to server for file data transfer or directory listing
Course : CS 372
Last Modified: 01.12.19

Resources :
    Project 1 (I implemented my own python server in project 1 for testing purposes)
    https://docs.python.org/3/library/socket.html#example
"""

from socket import *
import sys
import os.path


# Basic error checking to see if user inputs correct number of args
# Preconditions:  user has included commands in
# Returns transfer type to main function
def check_input():
    if sys.argv[3] == '-l' and len(sys.argv) is 5 and '65535' > sys.argv[4] > '1024':
        transfer_type = 'list'
    elif sys.argv[3] == '-g' and len(sys.argv) is 6 and '65535' > sys.argv[5] > '1024':
        transfer_type = 'get'
    elif sys.argv[1] != 'flip1' and sys.argv[1] != 'flip2' and sys.argv[1] != 'flip3':
        print("ERROR: invalid server name, enter flip1-3")
        exit(1)
    else:
        print("ERROR : Include '-l' or '-g' with proper input arguments as stated in assignment.")
        print("<server_host> <server_port>  <'-l'> <data_port> OR")
        print("<server_host> <server_port> <'-g'> <file_name> <data_port>")
        print("Program exiting...")
        sys.exit()

    return transfer_type


# Construct string with user args to send to server
# Preconditions:  input verified
# Postconditions: return complete server command
def build_input(data_type):
    if data_type is 'list':
        server_command = sys.argv[3] + " " + sys.argv[4] + " " + sys.argv[1]
    else:
        server_command = sys.argv[3] + " " + sys.argv[4] + " " + sys.argv[5] + " " + sys.argv[1]
    return server_command


# Construct control port
# Preconditions:  server name and port input by user and verified
# Postconditions: client socket created, connected and opened with server, socket num returned
def control_port(serverName, serverPort):
    client_socket = socket(AF_INET, SOCK_STREAM)
    client_socket.connect((serverName, serverPort))
    client_socket.send(command.encode())
    return client_socket


# Get directory listing from server
# Preconditions:  data socket created and connected between server and client
# Postconditions: -
def get_list(data_socket):
    print("***  Listing of Current Directory From Server ***")
    filename = data_socket.recv(1024).decode()  # accept up to 1024 bytes of data at once
    while filename != '':
        print(filename)
        filename = data_socket.recv(1024).decode()


# Get file from server
# Preconditions:  data socket created and connected between server and client
# Postconditions: file created with file name passed to server. if file already exists, file
# is overwritten
def get_file(data_socket):
    path = sys.argv[4]
    print("***  Get File: " + sys.argv[4] + " from Server ***")

    if os.path.isfile(path):
        print("File already exists, creating new file name ")
        prefix = 1
        while os.path.exists(path):
            path = str(prefix) + sys.argv[4]
            prefix += 1

    buffer = data_socket.recv(1024).decode()

    if "File Not Found" in buffer:
        print("From Server: " + buffer)
    else:
        f = open(path, "w")
        f.write(buffer)

        print("START writing data to file")
        while "** done **" not in buffer:
            buffer = data_socket.recv(1024).decode()
            f.write(buffer)
        print("DONE writing data to file")
        f.close()


# Construct server socket to listen for data connection from server
# Preconditions:  data transfer type determined
# Postconditions: client listening for connection from server
def create_socket(data_transfer_type):
    if data_transfer_type is "list":
        port_index = 4
    else:
        port_index = 5

    server_port = int(sys.argv[port_index])
    server_socket = socket(AF_INET, SOCK_STREAM)
    server_socket.bind(('', server_port))
    server_socket.listen(1)
    data_socket, address = server_socket.accept()
    return data_socket


# Build data port and call get file or get list. close data port once finished
# Preconditions:  data transfer type determined
# Postconditions: -
def data_port(data_transfer_type):
    data_socket = create_socket(data_transfer_type)
    if data_transfer_type is "get":  # Get a file
        get_file(data_socket)
    elif data_transfer_type is "list":  # Get directory list
        get_list(data_socket)

    data_socket.close()


# Initiate connection with server and send ftp command. close connection once finished
if __name__ == "__main__":

    data_transfer_type = check_input()
    command = build_input(data_transfer_type)

    serverName = sys.argv[1] + ".engr.oregonstate.edu"
    serverPort = int(sys.argv[2])

    # create control port
    clientSocket = control_port(serverName, serverPort)

    # create data port based on user command
    data_port(data_transfer_type)

    clientSocket.close()
