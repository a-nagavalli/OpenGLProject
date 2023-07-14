#ifndef MODEL_H
#define MODEL_H

#include "mesh.h"
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include "stb_image.h"

class Model
{
	private:
		bool is_loaded;
		std::string path;
		std::vector<Mesh> meshes;
		std::string directory;
		std::vector<Texture> textures_loaded;	// stores textures that have already been loaded in (optimization to avoid loading the same textures repeatedly)

		void loadModel(std::string path)
		{
			Assimp::Importer importer;
			const aiScene* scene = importer.ReadFile(path, aiProcess_Triangulate | aiProcess_FlipUVs);
			
			if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
				std::cout << "ERROR: Assimp: " << importer.GetErrorString() << std::endl;
				return;
			}
			directory = path.substr(0, path.find_last_of('/'));

			processNode(scene->mRootNode, scene);
		}

		void processNode(aiNode* node, const aiScene* scene)
		{
			// process this node's meshes
			for (unsigned int i = 0; i < node->mNumMeshes; ++i) {
				aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];	// scene's mMeshes array contains all the meshes in the scene
				meshes.push_back(processMesh(mesh, scene));
			}

			// then process each of the node's children
			for (unsigned int i = 0; i < node->mNumChildren; ++i) {
				processNode(node->mChildren[i], scene);
			}
		}

		Mesh processMesh(aiMesh* mesh, const aiScene* scene)
		{
			std::vector<Vertex> vertices;
			std::vector<unsigned int> indices;
			std::vector<Texture> textures;

			// process vertices (Position, Normal, and TexCoords)
			for (unsigned int i = 0; i < mesh->mNumVertices; ++i) {
				Vertex vertex;
				glm::vec3 vector;

				// position data
				vector.x = mesh->mVertices[i].x;
				vector.y = mesh->mVertices[i].y;
				vector.z = mesh->mVertices[i].z;
				vertex.Position = vector;

				// normal data
				vector.x = mesh->mNormals[i].x;
				vector.y = mesh->mNormals[i].y;
				vector.z = mesh->mNormals[i].z;
				vertex.Normal = vector;

				// texture coordinates
				if (mesh->mTextureCoords[0]) {		// first check to see if mesh has texture coordinates
					glm::vec2 vec;
					vec.x = mesh->mTextureCoords[0][i].x;	// only want first set of texture coordinates, hence mTextureCoords[0] (there can be up to 8)
					vec.y = mesh->mTextureCoords[0][i].y;
					vertex.TexCoords = vec;
				}
				else
					vertex.TexCoords = glm::vec2(0.0f, 0.0f);

				vertices.push_back(vertex);
			}

			// process indices
			for (unsigned int i = 0; i < mesh->mNumFaces; ++i) {	// each face contains data for its indices
				aiFace face = mesh->mFaces[i];
				for (unsigned int j = 0; j < face.mNumIndices; ++j)
					indices.push_back(face.mIndices[j]);
			}

			// process materials (textures in our case)
			if (mesh->mMaterialIndex >= 0) {	// if this mesh has a material
				aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];		// scene's mMaterials array contains all of its materials; mesh only contains its index for its material
				
				// put all the textures (diffuse and specular) into the textures vector (which will be passed to our mesh object)
				// diffuse textures
				std::vector<Texture> diffuseMaps = loadMaterialTextures(material, aiTextureType_DIFFUSE, "texture_diffuse");
				textures.insert(textures.end(), diffuseMaps.begin(), diffuseMaps.end());	

				// specular textures
				std::vector<Texture> specularMaps = loadMaterialTextures(material, aiTextureType_SPECULAR, "texture_specular");
				textures.insert(textures.end(), specularMaps.begin(), specularMaps.end());
			}

			return Mesh(vertices, indices, textures);

		}

		std::vector<Texture> loadMaterialTextures(aiMaterial* mat, aiTextureType type, std::string typeName)
		{
			std::vector<Texture> textures;
			for (unsigned int i = 0; i < mat->GetTextureCount(type); ++i) {
				aiString str;
				mat->GetTexture(type, i, &str);		// get the texture's file path

				bool skip = false;
				for (unsigned int j = 0; j < textures_loaded.size(); ++j) {
					if (std::strcmp(textures_loaded[j].path.data(), str.C_Str()) == 0) {
						textures.push_back(textures_loaded[j]);		// if this texture was already loaded, just push back the already-loaded texture, instead of loading it again
						skip = true;
						break;		
					}
				}

				if (!skip) {	// make and load a new texture
					Texture texture;
					texture.id = TextureFromFile(str.C_Str(), directory);	// get an ID for the texture after loading it in with stb_image.h
					texture.type = typeName;
					texture.path = str.C_Str();		// texture's path is stored to check if it has already been loaded in or not
					textures.push_back(texture);
					textures_loaded.push_back(texture);
				}
			}

			return textures;
		}

		unsigned int TextureFromFile(std::string path, std::string directory)
		{
			unsigned int texture;
			glGenTextures(1, &texture);
			glBindTexture(GL_TEXTURE_2D, texture);

			// texture wrapping and filtering options
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);	// repeat texture for wrapping
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);		// linear interpolation between mipmaps; linear interpolation within texture
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);	// linear interpolation within texture

			int width, height, nrChannels;
			unsigned char* data = stbi_load((directory + '/' + path).c_str(), &width, &height, &nrChannels, 0);
			
			GLenum format;
			if (nrChannels == 3)
				format = GL_RGB;
			else if (nrChannels == 4)
				format = GL_RGBA;
			
			//std::cout << nrChannels << std::endl;
			if (data) {
				glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, format, GL_UNSIGNED_BYTE, data);		// texture image has RGBA, but we only read in RGB values
				glGenerateMipmap(GL_TEXTURE_2D);
			}
			else
				std::cout << "ERROR: stbi_load failed to load texture" << std::endl;
			 
			stbi_image_free(data);

			return texture;
		}

	public:

		Model() 
		{
			is_loaded = false;
		}

		Model(const char* argPath) : path{ argPath } 
		{
			is_loaded = false;
		}

		void load(void)
		{
			if (path.empty()) {
				std::cout << "ERROR: No path given to model" << std::endl;
				return;
			}

			else {
				loadModel(path);
				is_loaded = true;
			}
		}

		void Draw(const ShaderProgram& program)
		{
			if (is_loaded) {
				for (const Mesh& mesh : meshes)
					mesh.Draw(program);
			}

			else 
				std::cout << "ERROR: Model not loaded" << std::endl;
		}
};


#endif
