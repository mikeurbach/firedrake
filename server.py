import firedrake as fd

def connection(socket):
    socket.on('data', data)
    socket.on('end', end)
    socket.on('close', close)
    
    fd.channels.join('chat', socket, chat)
    fd.channels.join('streaming', socket, streaming)

    socket.data = ''

def connection():
    print 'connection called.'

def data(socket, message):
    socket.send('thanks')
    fd.channels.broadcast('streaming', message)
    socket.data += message

def end(socket):
    print socket.data
    socket.data = ''

def close():
    fd.channels.leave('chat', socket)
    fd.channels.leave('streaming', socket)

def chat(channel, socket, message):
    socket.send(message)

def streaming(channel, socket, message):
    socket.send(message)
    

fd.run(8080, connection)
