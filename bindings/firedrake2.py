#! /usr/bin python

import sys

import pybindgen
from pybindgen import FileCodeSink
from pybindgen.gccxmlparser import ModuleParser

def module_gen():
    module_parser = ModuleParser('firedrake')
    module = module_parser.parse(['fd.h'])
    module.add_include('"fd.h"')

    pybindgen.write_preamble(FileCodeSink(sys.stdout))
    module.generate(FileCodeSink(sys.stdout))

if __name__ == '__main__':
    module_gen()
