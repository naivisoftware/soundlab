// fragment shader
#version 150

// texture binding
uniform sampler2D	mTexture;

// these are based on the vertex shader
in vec2 varyingtexcoord;

// Variables
uniform vec3 mColor;

// output color
out vec4 outputColor;

/*
Main Compute
*/
void main()
{
	vec4 image_clr = texture(mTexture, varyingtexcoord);
	image_clr.xyz *= mColor;

	// Set output color
	outputColor = image_clr;
}