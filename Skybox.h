#ifndef SKYBOX_H
#define SKYBOX_H

#include <string>
#include <vector>
#include <glad/glad.h>
#include <iostream>
#include "stb_image.h"
#include "ShaderProgram.h"
#include <glm/glm/glm.hpp>

class Skybox
{
	private:
		unsigned int VAO, VBO, texSkybox;
		std::vector<std::string> texturePaths;
		std::string skyboxDirectory;
		std::string fileExtension;
		ShaderProgram skyboxShaderProgram;
		float skyboxVertices[36 * 3] = {		// vertices for a 1x1x1 cube; unchanging for any skybox when rendered in this fashion
			-1.0f,  1.0f, -1.0f,
			-1.0f, -1.0f, -1.0f,
			 1.0f, -1.0f, -1.0f,
			 1.0f, -1.0f, -1.0f,
			 1.0f,  1.0f, -1.0f,
			-1.0f,  1.0f, -1.0f,

			-1.0f, -1.0f,  1.0f,
			-1.0f, -1.0f, -1.0f,
			-1.0f,  1.0f, -1.0f,
			-1.0f,  1.0f, -1.0f,
			-1.0f,  1.0f,  1.0f,
			-1.0f, -1.0f,  1.0f,

			 1.0f, -1.0f, -1.0f,
			 1.0f, -1.0f,  1.0f,
			 1.0f,  1.0f,  1.0f,
			 1.0f,  1.0f,  1.0f,
			 1.0f,  1.0f, -1.0f,
			 1.0f, -1.0f, -1.0f,

			-1.0f, -1.0f,  1.0f,
			-1.0f,  1.0f,  1.0f,
			 1.0f,  1.0f,  1.0f,
			 1.0f,  1.0f,  1.0f,
			 1.0f, -1.0f,  1.0f,
			-1.0f, -1.0f,  1.0f,

			-1.0f,  1.0f, -1.0f,
			 1.0f,  1.0f, -1.0f,
			 1.0f,  1.0f,  1.0f,
			 1.0f,  1.0f,  1.0f,
			-1.0f,  1.0f,  1.0f,
			-1.0f,  1.0f, -1.0f,

			-1.0f, -1.0f, -1.0f,
			-1.0f, -1.0f,  1.0f,
			 1.0f, -1.0f, -1.0f,
			 1.0f, -1.0f, -1.0f,
			-1.0f, -1.0f,  1.0f,
			 1.0f, -1.0f,  1.0f
		};

		void fillBuffers(void)
		{
			glGenVertexArrays(1, &VAO);
			glGenBuffers(1, &VBO);

			glBindVertexArray(VAO);
			glBindBuffer(GL_ARRAY_BUFFER, VBO);
			glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), skyboxVertices, GL_STATIC_DRAW);

			// position data	(layout = 0)
			glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 3, (void*)0);
			glEnableVertexAttribArray(0);
		}

		void genTextures(void)
		{
			glGenTextures(1, &texSkybox);
			glBindTexture(GL_TEXTURE_CUBE_MAP, texSkybox);

			int width, height, nrChannels;
			for (int i = 0; i < texturePaths.size(); ++i) {

				unsigned int texture;
				glGenTextures(1, &texture);
				glBindTexture(GL_TEXTURE_2D, texture);

				unsigned char* data = stbi_load((skyboxDirectory + '/' + texturePaths[i]).c_str(), &width, &height, &nrChannels, 0);

				GLenum format;
				if (nrChannels == 3)
					format = GL_RGB;
				else if (nrChannels == 4)
					format = GL_RGBA;

				if (data) {
					glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB, width, height, 0, format, GL_UNSIGNED_BYTE, data);		// texture image has RGBA, but we only read in RGB values
					glGenerateMipmap(GL_TEXTURE_2D);
				}
				else
					std::cout << "ERROR: SKYBOX: stbi_load failed to load texture" << std::endl;

				stbi_image_free(data);
			}
			glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

			skyboxShaderProgram.use();
			int loc = glGetUniformLocation(skyboxShaderProgram.ID, "skyboxTextures");
			if (loc < 0)
				std::cout << "ERROR: Skybox: couldn't get skyboxTextures uniform location" << std::endl;
			glUniform1i(loc, 0);		// skybox texture will be at texture unit 0
		}

	public:
		/// <summary>
		/// Generates a Skybox object
		/// Rename the skybox texture files to "right", "left", "top", "down", "front", and "back"
		/// </summary>
		/// <param name="fileExt">Extension of the files containing the skybox textures (include '.')</param>
		/// <param name="dir">Directory that the skybox texture files are in</param>
		/// <param name="prog">Shader program</param>
		Skybox(const std::string& fileExt, const std::string& dir, const ShaderProgram& prog)
			: fileExtension{ fileExt }, skyboxDirectory{ dir }, skyboxShaderProgram{ prog }
		{
			texturePaths = {
				"right" + fileExt,		// ex: right.bmp
				"left" + fileExt,
				"top" + fileExt,
				"down" + fileExt,
				"front" + fileExt,
				"back" + fileExt
			};

			fillBuffers();
			genTextures();
		}

		/// <summary>
		/// NOTE: Draw the skybox last, after all other objects have been drawn
		/// </summary>
		/// <param name="view">View matrix</param>
		/// <param name="projection">Projection matrix</param>
		void Draw(const glm::mat4& view, const glm::mat4& projection)
		{
			glDepthFunc(GL_LEQUAL);		// depth buffer will be filled with values of 1.0, so set to <= to make sure the skybox's fragments pass the depth test
			
			skyboxShaderProgram.use();

			glm::mat4 skyboxViewMat = glm::mat4(glm::mat3(view));		// set view and projection matrices (remove translation part of view matrix to make sure skybox is static while player moves around)
			skyboxShaderProgram.setUniformMatrix("view", skyboxViewMat);
			skyboxShaderProgram.setUniformMatrix("projection", projection);

			glActiveTexture(GL_TEXTURE0);		// set the skybox texture and bind the VAO before drawing
			glBindTexture(GL_TEXTURE_CUBE_MAP, texSkybox);
			glBindVertexArray(VAO);
			glDrawArrays(GL_TRIANGLES, 0, 36);

			glDepthFunc(GL_LESS);		// set it back to just < for the depth test
		}
};



#endif
