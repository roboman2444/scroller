#include <stdio.h>
#include <stdlib.h>
#include <GL/glew.h>
#include <GL/gl.h>
#include <GLFW/glfw3.h>
#include <math.h>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#define TRUE 1
#define FALSE 0


const char * lineshadervert = "\
	attribute vec2 tcattrib;\n\
	attribute vec2 posattrib;\n\
	varying vec2 tc;\n\
	uniform float time;\n\
	uniform float aspect;\n\
	void main(){\n\
		tc = tcattrib*vec2(aspect, 1.0) + vec2(1.,0.) * time;\n\
		gl_Position = vec4(posattrib, 0., 1.);\n\
	}\n\
";
const char * lineshaderfrag = "\
	varying vec2 tc;\n\
	uniform sampler2D tex;\n\
	void main(){\n\
		gl_FragColor = texture2D(tex,tc);\n\
		gl_FragColor.rgb *= vec3(0.1,1.0,0.1);\n\
	}\n\
";

int shader_printProgramLogStatus(const int id){
	GLint blen = 0;
	glGetProgramiv(id, GL_INFO_LOG_LENGTH, &blen);
	if(blen > 1){
		GLchar * log = (GLchar *) malloc(blen);
		glGetProgramInfoLog(id, blen, 0, log);
		printf("program log: \n%s\n", log);
		free(log);
	}
	return blen;
}
int shader_printShaderLogStatus(const int id){
	GLint blen = 0;
	glGetShaderiv(id, GL_INFO_LOG_LENGTH, &blen);
	if(blen > 1){
		GLchar * log = (GLchar *) malloc(blen);
		glGetShaderInfoLog(id, blen, 0, log);
		printf("shader log: \n%s\n", log);
		free(log);
	}
	return blen;
}

GLuint shaderid = 0;
#define POSATTRIBLOC 0
#define TCATTRIBLOC 1

int winwidth = 800;
int winheight = 600;
float aspect = 800.0/600.0;

double timestretch = 30.0;

int main(const int argc, const char ** argv){

	GLFWwindow * window;
	if(!glfwInit()) return -1;
	window 	= glfwCreateWindow(800, 600, "Linedraw", NULL, NULL);
	if (!window){
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);

	GLenum glewError = glewInit();
	if(glewError != GLEW_OK){
		printf( "Error initializing GLEW! %s\n", glewGetErrorString( glewError ) );
		return FALSE;
	}
	//gl and glew are good 2 go
	GLuint vertid;
	GLuint fragid;

	vertid = glCreateShader(GL_VERTEX_SHADER);
	fragid = glCreateShader(GL_FRAGMENT_SHADER);

	glShaderSource(vertid, 1, (const GLchar **)&lineshadervert, 0);
	glShaderSource(fragid, 1, (const GLchar **)&lineshaderfrag, 0);

	glCompileShader(vertid);
	glCompileShader(fragid);
	shader_printShaderLogStatus(vertid);
	shader_printShaderLogStatus(fragid);


	shaderid = glCreateProgram();
	if(!shaderid) printf("unable to greate program\n");
	glAttachShader(shaderid, vertid);
	glAttachShader(shaderid, fragid);

	glBindAttribLocation(shaderid, POSATTRIBLOC, "posattrib");
	glBindAttribLocation(shaderid, TCATTRIBLOC, "tcattrib");

	glLinkProgram(shaderid);
	glDeleteShader(vertid);
	glDeleteShader(fragid);




	shader_printProgramLogStatus(shaderid);
	glUseProgram(shaderid);

	GLuint texloc = glGetUniformLocation(shaderid, "tex");
	glUniform1i(texloc, 0);
	GLuint aspectloc = glGetUniformLocation(shaderid, "aspect");
	glUniform1f(aspectloc, aspect);

	GLuint timeloc = glGetUniformLocation(shaderid, "time");




	//texture

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);


	int width, height, nrChannels;
	unsigned char *data = stbi_load(argv[1], &width, &height, &nrChannels, 0);
	float imgaspect = (float)width/(float)height;

	unsigned int texture;
	glGenTextures(1, &texture);

	glBindTexture(GL_TEXTURE_2D, texture);
	switch(nrChannels){
	case 1:
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, width, height, 0, GL_RED, GL_UNSIGNED_BYTE, data);
		break;
	case 3:
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
		break;
	case 4:
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
		break;
	}

	glGenerateMipmap(GL_TEXTURE_2D);

	stbi_image_free(data);




	GLfloat vertices[] = { -1.0, -1.0,
				1.0, -1.0,
				1.0, 1.0,
				-1.0, -1.0,
				1.0, 1.0,
				-1.0, 1.0};
	GLfloat texcoords[] = { 0.0, 1.0,
				1.0, 1.0,
				1.0, 0.0,
				0.0, 1.0,
				1.0, 0.0,
				0.0, 0.0};



	glEnableVertexAttribArray(POSATTRIBLOC);
	glVertexAttribPointer(POSATTRIBLOC, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(GLfloat), vertices);
	glEnableVertexAttribArray(TCATTRIBLOC);
	glVertexAttribPointer(TCATTRIBLOC, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(GLfloat), texcoords);

	while(!glfwWindowShouldClose(window)){
		glfwPollEvents();
		glClear(GL_COLOR_BUFFER_BIT);
		double seconds = fmod(glfwGetTime()/timestretch, 1.00);
		glUniform1f(timeloc, seconds);
		glDrawArrays(GL_TRIANGLES, 0, 6);
		glfwSwapBuffers(window);
		if(glfwWindowShouldClose(window)){
			glfwTerminate();
			exit(0);
		}
		int iw, ih;
		glfwGetWindowSize(window, &iw, &ih);
		if(iw != winwidth || ih != winheight){
			winwidth = iw; winheight = ih;
			aspect = (float)winwidth/(float)winheight;
			glfwSetWindowSize(window, winwidth, winheight);
			glViewport(0,0, winwidth, winheight);
			glUniform1f(aspectloc, aspect /imgaspect);
		}

	}
}
