#pragma once

#include <GL/glew.h>
#include <vector>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
//#include <GLFW/glfw3.h>

struct Vertex
{
	glm::vec3 position;
	glm::vec3 color;
	glm::vec3 normal;
	float padding;
};

typedef std::vector<unsigned short> IndexBuffer;
typedef std::vector<Vertex> VertexBuffer;

class Mesh
{
public:
	Mesh(GLuint program);
	~Mesh();

	virtual void Draw(const float time);
private:
	void BindBuffers();

	GLuint m_gl_program;
	GLuint m_gl_vertex;
	GLuint m_gl_indices;
	GLuint m_gl_vao;
	const int m_stride = sizeof(Vertex);

protected:
	void UploadData();
	void SetupGlBuffers();
	VertexBuffer m_vertices;
	IndexBuffer m_indices;
};

