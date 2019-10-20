#include "OctreeVisualization.h"
#include "Octree.h"
#include "utils.h"

#include <GL/glew.h>


OctreeVisualization::OctreeVisualization(GLuint program) : m_program(program)
{
}


OctreeVisualization::~OctreeVisualization()
{
}

void OctreeVisualization::Initialize()
{
	auto [vao, vbo] = createCubeVAO(m_cubePoints);
	m_vao = vao;
	m_vbo = vbo;

}
void OctreeVisualization::Build(const Octree* node)
{
	OctreeVisualizationData stuff = { node->m_size, node->m_min };
	m_visualizationData.push_back(stuff);

	const auto octreeChildren = node->GetChildren();
	if (!octreeChildren)
	{
		return;
	}

	auto children = octreeChildren->children;
	for (const Octree* child : children)
	{
		if (child)
		{
			Build(child);
		}
	}
}

void OctreeVisualization::DrawVisualization(const float time)
{
	glBindVertexArray(m_vao);
	glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
	GLint posAttrib = glGetAttribLocation(m_program, "position");
	glEnableVertexAttribArray(posAttrib);
	glVertexAttribPointer(
		posAttrib, // attribute
		3,                 // number of elements per vertex, here (x,y,z)
		GL_FLOAT,          
		GL_FALSE,         
		m_stride * sizeof(float),          
		0               
	);

	GLint colorAttrib = glGetAttribLocation(m_program, "inColor");
	glEnableVertexAttribArray(colorAttrib);
	glVertexAttribPointer(
		colorAttrib, // attribute
		3,                 // number of elements per vertex, here (R,G,B)
		GL_FLOAT,          // the type of each element
		GL_FALSE,          // take our values as-is
		m_stride * sizeof(float),                 // no extra data between each position
		(void*)(3 * sizeof(float))                  // offset of first element
	);

	//printf("draw visualization thing\n");
	GLint modelUniform = glGetUniformLocation(m_program, "Model");
	for (const auto& viz : m_visualizationData)
	{
		if (viz.size < 4) { continue; }
		glm::mat4 translate = glm::translate
		(
			glm::mat4(1.0f),
			viz.min
		);

		glm::mat4 rotate = glm::mat4(1.0f);
		rotate = glm::rotate(
			rotate,
			time * glm::radians(60.0f),
			glm::vec3(0.0f, 1.0f, 0.0f));

		glm::mat4 scale = glm::mat4(1.0f);
		scale[0] = glm::vec4(viz.size, 0, 0, 0);
		scale[1] = glm::vec4(0, viz.size, 0, 0);
		scale[2] = glm::vec4(0, 0, viz.size, 0);

		//this should be correct but isn't?
		//glUniformMatrix4fv(modelUniform, 1, GL_FALSE, glm::value_ptr(model * translate * rotate * scale));

		glUniformMatrix4fv(modelUniform, 1, GL_FALSE, glm::value_ptr(rotate * translate * scale));
		glBindVertexArray(m_vao);
		glDrawArrays(GL_TRIANGLES, 0, m_cubePoints.size());
	}
	glBindVertexArray(0);
}

