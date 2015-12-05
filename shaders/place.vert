attribute vec2 vertex;
uniform mat2 transform;
uniform vec2 translate;

void main()
{
	gl_Position = vec4(translate + transform*vertex,0.0,1.0);
	gl_TexCoord[0] = vec4(vertex,0.0,1.0);
}
