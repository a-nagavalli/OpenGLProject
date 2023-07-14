#ifndef MESH_H
#define MESH_H

#include <glm/glm/glm.hpp>
#include <string>
#include <vector>
#include <glad/glad.h>
#include "ShaderProgram.h"

struct Vertex {
	glm::vec3 Position;
	glm::vec3 Normal;
	glm::vec2 TexCoords;
};

struct Texture {
	unsigned int id;
	std::string type;
	std::string path;
};

class Mesh {
	private:
		unsigned int VAO, VBO, EBO;		// each mesh should have its own vertex array, vertex buffer, and element buffer objects
	
	public:
		std::vector<Vertex> vertices;
		std::vector<unsigned int> indices;
		std::vector<Texture> textures;

		Mesh(std::vector<Vertex> argVertices, std::vector<unsigned int> argIndices, std::vector<Texture> argTextures) : vertices{ argVertices }, indices{ argIndices }, textures{ argTextures }
		{
			glGenBuffers(1, &VBO);
			glGenBuffers(1, &EBO);
			glGenVertexArrays(1, &VAO);

			glBindVertexArray(VAO);

			glBindBuffer(GL_ARRAY_BUFFER, VBO);
			glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), &vertices[0], GL_STATIC_DRAW);

			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
			glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), &indices[0], GL_STATIC_DRAW);

			// position data	(layout = 0)
			glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);	// using sizeof(Vertex) for the stride
			glEnableVertexAttribArray(0);

			// normal data	(layout = 1)
			glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Normal));
			glEnableVertexAttribArray(1);

			// texture coordinate data	 (layout = 2)
			glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, TexCoords));
			glEnableVertexAttribArray(2);
		}

		void Draw(const ShaderProgram& program) const
		{
			unsigned int diffuseNr = 1;
			unsigned int specularNr = 1;
			for (int i = 0; i < textures.size(); ++i) {
				glActiveTexture(GL_TEXTURE0 + i);	// activate texture unit before binding
				std::string name = textures[i].type;
				std::string number;
				if (name == "texture_diffuse")
					number = std::to_string(diffuseNr++);
				else if (name == "texture_specular")
					number = std::to_string(specularNr++);

				program.setInt((name + number).c_str(), i);		// set sampler2D uniform to the corresponding texture unit
				glBindTexture(GL_TEXTURE_2D, textures[i].id);	// bind the texture before drawing (done for each texture)
			}
			glActiveTexture(GL_TEXTURE0);	// set texture unit back to default

			// draw mesh
			glBindVertexArray(VAO);
			glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);
			
			// unbind vertex array
			glBindVertexArray(0);
		}
};

#endif
