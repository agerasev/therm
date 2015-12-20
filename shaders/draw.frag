uniform sampler2D u_texture;

varying vec2 v_uni_coord;

void main() {
	vec4 v = texture2D(u_texture, v_uni_coord);
	float t = 4.0*(1.0 - clamp(v.x, 0.0, 1.0));
	vec3 c = vec3(abs(t - 3.0) - 1.0, 2.0 - abs(t - 2.0), abs(t - 1.0) - 1.0);
	float m = 0.8*v.y + 0.2;
	gl_FragColor = vec4(m*clamp(c, 0.0, 1.0), 1.0);
}
