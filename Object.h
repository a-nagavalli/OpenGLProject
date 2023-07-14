#ifndef OBJECT_H
#define OBJECT_H

#include "model.h"
#include <glm/glm/glm.hpp>
#include <glm/glm/gtc/type_ptr.hpp>

enum Axis {
	axisX,
	axisY,
	axisZ
};

class Object
{
private:
	Model model;
	glm::mat4 matrix;

public:
	Object(const char* path)
	{
		model = Model(path);
		model.load();
		matrix = glm::mat4(1.0f);
	}

	void Draw(ShaderProgram& program)
	{
		program.use();
		program.setUniformMatrix("model", matrix);

		model.Draw(program);
	}

	/// <summary>
	/// NOTE: First translate, then rotate, and then scale
	/// </summary>
	void Translate(float x, float y, float z)
	{
		matrix = glm::translate(matrix, glm::vec3(x, y, z));
	}

	/// <summary>
	/// NOTE: First translate, then rotate, and then scale
	/// </summary>
	void Rotate(float degrees, Axis axis)
	{
		glm::vec3 vec(0.0f);

		switch (axis) {
			case axisX:
				vec.x = 1.0f;
				break;
			case axisY:
				vec.y = 1.0f;
				break;
			case axisZ:
				vec.z = 1.0f;
				break;
		}

		matrix = glm::rotate(matrix, glm::radians(degrees), vec);
	}

	/// <summary>
	/// NOTE: First translate, then rotate, and then scale
	/// Don't call in render loop!
	/// </summary>
	void Scale(float x, float y, float z)
	{
		matrix = glm::scale(matrix, glm::vec3(x, y, z));
	}
};

#endif
