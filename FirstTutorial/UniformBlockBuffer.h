#ifndef UNIFORMBLOCKBUFFER_H
#define UNIFORMBLOCKBUFFER_H

#include <algorithm>

#include <glad/glad.h>
#include <glm/gtc/type_ptr.hpp>

template <typename T>
class UniformBlockBuffer
{
public:
	UniformBlockBuffer<T>(size_t size);

	unsigned int Name() const;
	void BindToIndex(unsigned int index) const;
	size_t FillBuffer(T& data, unsigned int count = 1, unsigned int offset = 0);

private:
	GLuint id_;
	size_t size_;
	size_t used_;
};

template <typename T>
UniformBlockBuffer<T>::UniformBlockBuffer(size_t size)
{
	size_ = size * sizeof(T);
	used_ = 0;

	glGenBuffers(1, &id_);
	glBindBuffer(GL_UNIFORM_BUFFER, id_);
	glBufferData(GL_UNIFORM_BUFFER, size_, NULL, GL_STATIC_DRAW);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

template <typename T>
unsigned UniformBlockBuffer<T>::Name() const
{
	return id_;
}

template <typename T>
void UniformBlockBuffer<T>::BindToIndex(unsigned int bindingIndex) const
{
	glBindBufferBase(GL_UNIFORM_BUFFER, bindingIndex, id_);
}

template <typename T>
size_t UniformBlockBuffer<T>::FillBuffer(T& data, unsigned int count, unsigned int offset)
{
	if (offset == 0 && used_ != 0)
	{
		offset = used_;
	}
	size_t filled = std::min(sizeof(T) * count, size_ - used_);

	glBindBuffer(GL_UNIFORM_BUFFER, id_);
	glBufferSubData(GL_UNIFORM_BUFFER, offset, filled, glm::value_ptr(data));
	glBindBuffer(GL_UNIFORM_BUFFER, 0);

	used_ += filled;

	return filled;
}

#endif
