# clientServerFTP
Basic FTP Client Server with '-l' and '-g' functionality using C and Python

To execute :
        Server and client need to be in different directories. txt files should be in server directory prior to executing below. This program is meant to be ran on OSU servers, anthough code could easily be modified to run on any client / server.

        1. Start server first
                gcc -o ftserver ftserver.c
                ./ftserver [server_port]        // say for example 13256

        2. Start client
                chmod +x ftclient.py
                python3 ftclient.py [flip1-3] [server_port] [command(s)] [data_port]
                        - server_port is same for both
                        - flip is the flip server that ftserver.c is run on
                        - commands are either [-l]  or [-g] [filename]
                        - data port is port for data transfer from server
