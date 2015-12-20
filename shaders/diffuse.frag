uniform sampler2D u_source;
uniform ivec2 u_area_size;

varying vec2 v_uni_coord;

ivec2 uni_to_pix(vec2 uni) {
	return ivec2(uni*vec2(u_area_size));
}

vec2 pix_to_uni(ivec2 pix) {
	return (vec2(pix) + vec2(0.5, 0.5))/vec2(u_area_size);
}

void main(void) {
	ivec2 pix = uni_to_pix(v_uni_coord);
	vec4 
	  c = texture2D(u_source, pix_to_uni(pix)), 
	  t = texture2D(u_source, pix_to_uni(pix + ivec2( 0, 1))), 
	  b = texture2D(u_source, pix_to_uni(pix + ivec2( 0,-1))), 
	  l = texture2D(u_source, pix_to_uni(pix + ivec2( 1, 0))), 
	  r = texture2D(u_source, pix_to_uni(pix + ivec2(-1, 0)));
	float div = 4.0*c.x - (t.x + b.x + l.x + r.x);
	float deriv = c.y*div;
	float dt = 1e-1;
    gl_FragColor = c - vec4(deriv*dt, 0.0, 0.0, 0.0);
}
