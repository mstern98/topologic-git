from topylogic import graph, topylogic_function, vertex_result, vertex_result, stack
import topylogic

def v(a : vertex_result):
    print("HI")

def e(a):
    print(a)
    return 1

vf = topylogic_function(v)
ef = topylogic_function(e)
g = graph()
v1 = g.create_vertex(vf.callback_void, 1)
v2 = g.create_vertex(vf.callback_void, 2)
edge = g.create_edge(v1, v2, ef.callback_int)
g.set_start_set([1], 1)

v = vertex_result("hi", "edge")
print(v1.f)
print(v1.f(v))

g.run([v])