#ifndef SHADER_PROGRAM_H
#define SHADER_PROGRAM_H

#include <fstream>
#include <string>
#include <iostream>
#include <glad/glad.h>

class ShaderFile
{
	private:
		std::ifstream fileStream;
		std::string shaderSrc;
		std::string shaderType;

	public:

		ShaderFile(const std::string& path, const std::string& type) : shaderType{ type }
		{
			std::string line;
			fileStream.open(path);
			if (fileStream.is_open()) {
				while (std::getline(fileStream, line))
					shaderSrc += line + '\n';
			}
			fileStream.close();
			if (shaderSrc.empty())
				std::cout << "ERROR: " + type + " shader source not loaded" << std::endl;
		}

		const char** getSource(void) const
		{
			const char* cStr = shaderSrc.c_str();
			return &cStr;
		}

		std::string getType(void) const
		{
			return shaderType;
		}
};

class ShaderProgram
{
	public:
		unsigned int ID;

		ShaderProgram(const ShaderFile& vertexShaderFile, const ShaderFile& fragmentShaderFile)
		{
			unsigned int vertexShader, fragmentShader, shaderProgram;
			int success;
			char infoLog[512];

			shaderProgram = glCreateProgram();
			vertexShader = glCreateShader(GL_VERTEX_SHADER);
			fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);

			glShaderSource(vertexShader, 1, vertexShaderFile.getSource(), NULL);
			glShaderSource(fragmentShader, 1, fragmentShaderFile.getSource(), NULL);

			// Vertex Shader compilation
			glCompileShader(vertexShader);
			// check to see if shader compilation succeeded
			glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
			if (!success) {
				glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
				std::cout << "ERROR: Vertex Shader Compilation failed\n" << infoLog << std::endl;
				ID = 0;
			}
			// if it did succeed, attach it to the shader program
			glAttachShader(shaderProgram, vertexShader);

			// Fragment Shader compilation
			glCompileShader(fragmentShader);
			glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
			if (!success) {
				glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
				std::cout << "ERROR: Fragment Shader Compilation failed\n" << infoLog << std::endl;
				ID = 0;
			}
			glAttachShader(shaderProgram, fragmentShader);

			// Link shader program
			glLinkProgram(shaderProgram);
			glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
			if (!success) {
				glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
				std::cout << "ERROR: Shader program link failed\n" << infoLog << std::endl;
				ID = 0;
			}

			// delete shaders; no longer need them
			glDeleteShader(vertexShader);
			glDeleteShader(fragmentShader);

			ID = shaderProgram;
		}

		void use(void) const
		{
			glUseProgram(ID);
		}

		void setUniformMatrix(const char* name, const glm::mat4& matrix) const
		{
			int loc = glGetUniformLocation(ID, name);
			if (loc == -1)
				std::cout << "ERROR: couldn't get matrix uniform location" << std::endl;
			glUniformMatrix4fv(loc, 1, GL_FALSE, glm::value_ptr(matrix));
		}

		void setVec3(const char* name, const glm::vec3& vec) const
		{
			float arr[] = { vec.x, vec.y, vec.z };

			int loc = glGetUniformLocation(ID, name);
			if (loc == -1)
				std::cout << "ERROR: couldn't get vec3 uniform location" << std::endl;
			glUniform3fv(loc, 1, arr);
		}

		void setFloat(const char* name, float val) const
		{
			int loc = glGetUniformLocation(ID, name);
			if (loc == -1)
				std::cout << "ERROR: couldn't get float uniform location" << std::endl;
			glUniform1f(loc, val);
		}

		void setInt(const char* name, int i) const
		{
			int loc = glGetUniformLocation(ID, name);
			if (loc == -1)
				std::cout << "ERROR: couldn't get int uniform location" << std::endl;
			glUniform1i(loc, i);
		}
};

#endif
