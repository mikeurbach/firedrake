from time import gmtime, strftime
import firedrake as fd

sockets = {}

def acc(socket, string):
    if not sockets[socket]:
        sockets[socket] = []

    sockets[socket].append(string);

def dump(socket):
    log = open("log.txt", "a")
    log.write(strftime("*** %a, %d %b %Y %H:%M:%S +0000", gmtime()))
    for fragment in sockets[socket]:
        log.write(fragment)
    log.close()

def callback(socket):
    socket.ondata = acc
    socket.onend = dump

fd.onconnection(8080, callback)
