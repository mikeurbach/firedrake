from pybindgen import *
import sys

# create a module object
mod = Module('firedrake')
mod.add_include('"fd.h"')

# test function
mod.add_function('fd_test', None, [])

# bind fd_socket_t
# fd_socket_t = mod.add_struct('_fd_socket_t')
# fd_socket_t.add_instance_attribute('data_cb', 
#                                 'void (*) (struct _fd_socket_t *, char *)')
# fd_socket_t.add_instance_attribute('end_cb', 'EndCallback')

# bind fd_run
# mod.add_function('fd_run', retval('int'), 
#                  [param('int', 'port'), 
#                   param('AcceptCallback', 'callback')])

# spit out the bindings
mod.generate(sys.stdout)
