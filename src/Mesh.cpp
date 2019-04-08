#include "Mesh.h"

Mesh::Mesh(GLuint program) : m_vertices(), m_indices(), m_gl_program(program), m_gl_vertex(0), m_gl_vao(0), m_gl_indices(0)
{
}

Mesh::~Mesh()
{
}
void Mesh::SetupGlBuffers()
{
	glGenVertexArrays(1, &m_gl_vao);
	glBindVertexArray(m_gl_vao);

	glGenBuffers(1, &m_gl_vertex);
	glBindBuffer(GL_ARRAY_BUFFER, m_gl_vertex);
	glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex) * m_vertices.size(), m_vertices.data(), GL_STATIC_DRAW);

	glGenBuffers(1, &m_gl_indices);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_gl_indices);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned short) * m_indices.size(), m_indices.data(), GL_STATIC_DRAW);

	//glBindVertexArray(m_gl_vao);
}

void Mesh::Draw(const float time)
{
	if (m_indices.size() == 0)
	{
		// nothing to draw
		return;
	}

	glm::mat4 model = glm::mat4(1.0f);
	model[0] = glm::vec4(1, 0, 0, 0);
	model[1] = glm::vec4(0, 1, 0, 0);
	model[2] = glm::vec4(0, 0, 1, 0);
	model[3] = glm::vec4(0, 0, 0, 1);

	model = glm::translate(model, glm::vec3(-5, -5, -5));

	GLint modelUniform = glGetUniformLocation(m_gl_program, "Model");
	glm::mat4 rotate = glm::rotate(
		model,
		time * glm::radians(60.0f),
		glm::vec3(0.0f, 1.0f, 0.0f));
	glUniformMatrix4fv(modelUniform, 1, GL_FALSE, glm::value_ptr(rotate));

	// draw it
	BindBuffers();
	glDrawElements(GL_TRIANGLES, m_indices.size(), GL_UNSIGNED_SHORT, 0);
}

// TODO: figure out which parts are unnecessary to do on each frame
void Mesh::BindBuffers()
{
	glBindVertexArray(m_gl_vao);

	GLint posAttrib = glGetAttribLocation(m_gl_program, "position");
	glEnableVertexAttribArray(posAttrib);
	glBindBuffer(GL_ARRAY_BUFFER, m_gl_vertex);
	glVertexAttribPointer(
		posAttrib, // attribute
		3,                 // number of elements per vertex, here (x,y,z)
		GL_FLOAT,          // the type of each element
		GL_FALSE,          // take our values as-is
		m_stride,                 // no extra data between each position
		0                  // offset of first element
	);

	//GLint colAttrib = glGetAttribLocation(m_gl_program, "inColor");
	//glEnableVertexAttribArray(colAttrib);
	//glVertexAttribPointer(
	//	colAttrib, // attribute
	//	3,                 // number of elements per vertex, here (R,G,B)
	//	GL_FLOAT,          // the type of each element
	//	GL_FALSE,          // take our values as-is
	//	m_stride,                 // no extra data between each position
	//	(void*)(3 * sizeof(float))                  // offset of first element
	//);

	GLint normAttrib = glGetAttribLocation(m_gl_program, "normal");
	glEnableVertexAttribArray(normAttrib);
	glVertexAttribPointer(
		normAttrib, // attribute
		3,                 // number of elements per vertex, here (R,G,B)
		GL_FLOAT,          // the type of each element
		GL_FALSE,          // take our values as-is
		m_stride,                 // no extra data between each position
		(void*)(6 * sizeof(float))                  // offset of first element
	);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_gl_indices);
}
