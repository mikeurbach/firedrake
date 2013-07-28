import firedrake as fd

def echo(socket, message):
    socket.send(message)

def accept(socket):
    socket.on('data', echo)

fd.run(8080, accept)
