from distutils.core import setup, Extension

extension = Extension("_topylogic", ["topylogic/_topylogic_module.cc", 
"src/edge.c", 
"src/graph.c", 
"src/request.c", 
"src/topologic_json.c", 
"src/topologic.c",
"src/vertex.c",
"parse/topologic_parser.tab.c"])

setup(name = "topylogic", ext_modules=[extension])