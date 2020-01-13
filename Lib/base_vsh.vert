attribute vec3 qt_Vertex;
uniform mat4 Matrix;

void main(void)
{
	gl_Position = Matrix * vec4( qt_Vertex, 1.0 );
}
