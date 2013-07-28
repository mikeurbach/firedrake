from ctypes import *

# load libfiredrake
fd = cdll.LoadLibrary('libfiredrake.so')

# define the fd_channel_name
class fd_channel_name(Structure):
    pass

fd_channel_name._fields_ = [('key', c_char_p),
                            ('next', POINTER(fd_channel_name))]

# define the ev_io type
class ev_io(Structure):
    _fields_ = []

# define the fd_socket_t
class fd_socket_t(Structure):
    pass

# set the callback types
ACCEPT_CB_TYPE = CFUNCTYPE(None, POINTER(fd_socket_t))
DATA_CB_TYPE = CFUNCTYPE(None, POINTER(fd_socket_t), c_char_p)
END_CB_TYPE = CFUNCTYPE(None, POINTER(fd_socket_t))

fd_socket_t._fields_ = [('read_w', ev_io),
                        ('write_w', ev_io),
                        ('tcp_sock', c_int),
                        ('bytes_expected', c_ulonglong),
                        ('bytes_received', c_ulonglong),
                        ('bytes_outgoing', c_ulonglong),
                        ('bytes_sent', c_ulonglong),
                        ('header_len', c_int),
                        ('fin', c_int),
                        ('opcode', c_int),
                        ('mask_key', c_uint),
                        ('buffer', c_char_p),
                        ('out_buffer', c_char_p),
                        ('last_recv_opcode', c_uint),
                        ('is_open', c_bool),
                        ('just_opened', c_int),
                        ('recvs', c_int),
                        ('sends', c_int),
                        ('event', c_int),
                        ('data', c_void_p),
                        ('accept_cb', ACCEPT_CB_TYPE),
                        ('data_cb', DATA_CB_TYPE),
                        ('next', POINTER(fd_socket_t)),
                        ('channel_list', POINTER(fd_channel_name)),
                        ('end_cb', END_CB_TYPE)]

# define the data callback
def ondata(socket, buff):
    print 'ondata invoked: ' + buff

# define the end callback
def onend(socket):
    print 'onend invoked'

# set the data and end callback types
data_cb = DATA_CB_TYPE(ondata)
end_cb = END_CB_TYPE(onend)

# define the accept callback
def onconnection(socket):
    print 'onconnection invoked'
    socket.contents.data_cb = data_cb
    socket.contents.end_cb = end_cb

# set the accept callback type
accept_cb = ACCEPT_CB_TYPE(onconnection)

# run it
fd.fd_run(8080, accept_cb)
