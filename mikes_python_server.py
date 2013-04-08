#!/usr/bin/python
"USAGE: server.py <port>"
from socket import *
import sys, sha, cgi

# constants
BUFFER = 1024
MAGIC_STRING = "258EAFA5-E914-47DA-95CA-C5AB0DC85B11"

# utility functions
def handshake(sock):
    data = sock.recv(BUFFER).splitlines()
    for header in data:
        if "Sec-WebSocket-Key" in header:
            keyheader = header.partition(':')
            key = keyheader[2].strip() # get key
            sha1 = sha.new(key.strip('\r\n') + MAGIC_STRING) # concat magic and hash
            accept = sha1.digest().encode('base64') # base64 encode
            response = """HTTP/1.1 101 Switching Protocols\r\nUpgrade: websocket\r\nConnection: Upgrade\r\nSec-WebSocket-Accept: """ + accept + "\r\n"
            sock.send(response)
    print "handshake completed"

# borrowed from Hunter F on
# http://stackoverflow.com/questions/8125507/how-can-i-send-and-receive-websocket-messages-on-the-server-side
def decodestream(stringStreamIn):
    #turn string values into opererable numeric byte values
    byteArray = [ord(character) for character in stringStreamIn]
    datalength = byteArray[1] & 127
    indexFirstMask = 2 
    if datalength == 126:
        indexFirstMask = 4
    elif datalength == 127:
        indexFirstMask = 10
    masks = [m for m in byteArray[indexFirstMask : indexFirstMask+4]]
    indexFirstDataByte = indexFirstMask + 4
    decodedChars = []
    i = indexFirstDataByte
    j = 0
    while i < len(byteArray):
        decodedChars.append( chr(byteArray[i] ^ masks[j % 4]) )
        i += 1
        j += 1
    return decodedChars

def encodestream(stringStreamIn):
    # find string length
    header = 1 & 0xFF
    byteArray = [ord(character) for character in stringStreamIn]
    datalength = len(byteArray)

    # figure out header
    if datalength <= 125:
        header += datalength
    elif datalength <= 32767:
        header = header << 24
        header += (126 << 16) + (datalength & 0xFFFF)
    else:
        header = header << 72
        header += (127 << 64) + (datalength & 0xFFFFFFFFFFFFFFFF)

    # turn header into chars and prepend to stringStreamIn
    stringStreamOut = stringStreamIn
    while header > 0:
        char = chr(header & 0xFF)
        stringStreamOut = char + stringStreamOut
        header = header >> 8

    return stringStreamOut
    

def handleClient(sock):
    handshake(sock)
    data = decodestream(sock.recv(BUFFER))
    while data:
        output = ''.join(data)
        try:
            sock.sendall(output)
            print "echoing:", output
        except:
            print "couldn't call sendall:", sys.exc_info()[0]
        data = decodestream(sock.recv(BUFFER))
    print "closing socket"
    sock.close()

if __name__ == '__main__':
    # serve it
    sockets = []
    if len(sys.argv) != 2:
        print __doc__
    else:
        sock = socket(AF_INET, SOCK_STREAM)
        sock.bind(('',int(sys.argv[1])))
        sock.listen(10)
        while 1:
            print "listening for clients"
            newsock, client_addr = sock.accept()
            print "client connected:", client_addr 
            handleClient(newsock)
