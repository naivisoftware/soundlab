// fragment shader
#version 150

// these are based on the vertex shader
in vec2 varyingtexcoord;
in vec4 vertcolor;

// output color
out vec4 outputColor;

/*
Main Compute
*/
void main()
{
	// Set output color
	outputColor = vertcolor;
}