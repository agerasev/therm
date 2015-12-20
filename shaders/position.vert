attribute vec2 a_vertex;
uniform mat2 u_map;
uniform vec2 u_offset;

varying vec2 v_uni_coord;

void main() {
	gl_Position = vec4(u_offset + u_map*a_vertex, 0.0, 1.0);
	v_uni_coord = a_vertex;
}
