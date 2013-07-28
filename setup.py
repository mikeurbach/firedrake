from distutils.core import setup, Extension

firedrake_module = Extension(
    'firedrake',
    define_macros = [('PYTHON_MODE', '1')],
    include_dirs = ['./libev'],
    libraries = ['pthread', 'ssl', 'crypto'],
    library_dirs = ['/usr/include'],
    sources = ['fd_run.c', 'fd_send.c', 'fd_recv.c', 'fd_channels.c', 'fd_util.c', 'base64.c', 'queue.c', 'firedrakemodule.c'],
    extra_compile_args = ['-std=gnu99']
    )

setup(
    name = 'firedrake',
    version = '0.1',
    description = 'fast websocket server library',
    author = 'Mike Urbach',
    author_email = 'mikeurbach@gmail.com',
    url = 'www.cs.dartmouth.edu/~urbach/firedrake',
    ext_modules = [firedrake_module]
    )
