from pprint import pprint
import struct
triangles = []

with open("pg.obj") as file:
	vertices = []
	for line in file:
		t = line.split()
		if t[0] == 'v':
			vertex = list(map(float, t[1:]))
			vertex[1] = -vertex[1] # y = -y
			vertex[2] = 0 # z = 0
			vertices.append(vertex)
		elif t[0] == 'f':
			i, j, k = map(int, reversed(t[1:]))
			triangles.append((vertices[i-1], vertices[j-1], vertices[k-1]))


pprint(vertices, open("v.txt", 'w'))
pprint(triangles, open("t.txt", 'w'))
			
data = struct.pack("I", len(triangles))
for triangle in triangles:
	for v in triangle:
		data = data + struct.pack("fff", *v)
		
with open('resources/pg.bin', 'wb') as out_file:
	out_file.write(data)
