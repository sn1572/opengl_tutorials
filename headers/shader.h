#ifndef SHADER_H
	#define SHADER_H

	#include <glad/glad.h>
	#include <stdio.h>
	#include <stdlib.h>

	struct Shader{
		unsigned int ID;
		struct Shader * self;
		void (*load)(struct Shader * self, char * vertexPath, char * fragmentPath);
		void (*use)(struct Shader * self );
		void (*setBool)(struct Shader * self, const char * name, int value);
		void (*setInt)(struct Shader * self, const char * name, int value);
		void (*setFloat)(struct Shader * self, const char * name, float value);
		void (*setVec3)( struct Shader * self, const char * name,
				 float x, float y, float z );
	};

	void readFile( const char * fname, char ** buffer );
	void load( struct Shader * self, char * vertexPath, char * fragmentPath );
	void use( struct Shader * self );
	void setBool( struct Shader * self, const char * name, int value );
	void setInt( struct Shader * self, const char * name, int value );
	void setFloat( struct Shader * self, const char * name, float value );
	void setVec3( struct Shader * self, const char * name, float x, float y, float z );
	struct Shader * shaderInit();
	void checkCompileErrors( unsigned int shader, char * type );
	void shader_introspection( struct Shader * shaders );

#endif
