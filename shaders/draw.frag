varying vec2 position;

void main() {
	gl_FragColor = vec4(position, (1.0 - position.x)*(1.0 - position.y), 1.0);
}
