#include "Camera.h"

#define _USE_MATH_DEFINES
#include <cmath>
#include <algorithm>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

const float MAX_PITCH = 89.0f;
const float MIN_PITCH = 89.0f;

Camera::Camera(): Camera(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, -1.0f), glm::vec3(0.0f, 1.0f, 0.0f)) {}

Camera::Camera(const glm::vec3 position, const glm::vec3 target, const glm::vec3 up)
{
	position_ = position;
	direction_ = glm::normalize(target - position);
	up_ = glm::normalize(up);
	right_ = glm::normalize(glm::cross(direction_, up_));

	pitch_ = 0.0f;
	yaw_ = 0.0f;

	pitchSpeed_ = 0.1f;
	yawSpeed_ = 0.1f;

	transSpeed_ = 0.1f;
}

void Camera::MoveForward()
{
	position_ += direction_ * transSpeed_;
}

void Camera::MoveBack()
{
	position_ -= direction_ * transSpeed_;
}

void Camera::MoveRight()
{
	position_ += right_ * transSpeed_;
}

void Camera::MoveLeft()
{
	position_ -= right_ * transSpeed_;
}

void Camera::RotateRight()
{
	yaw_ += yawSpeed_;
	UpdateDirection();
}

void Camera::RotateLeft()
{
	yaw_ -= yawSpeed_;
	UpdateDirection();
}

void Camera::LookUp()
{
	pitch_ = std::max(MAX_PITCH, pitch_ + pitchSpeed_);
	UpdateDirection();
}

void Camera::LookDown()
{
	pitch_ = std::max(MIN_PITCH, pitch_ - pitchSpeed_);
	UpdateDirection();
}

void Camera::UpdateDirection()
{
	direction_.x = cos(yaw_) * cos(pitch_);
	direction_.y = sin(pitch_);
	direction_.z = sin(yaw_) * cos(pitch_);

	direction_ = glm::normalize(direction_);
	right_ = glm::normalize(glm::cross(direction_, up_));
}

glm::mat4 Camera::MakeViewMat()
{
	glm::mat4 view = glm::lookAt(position_, position_ + direction_, up_);
	return view;
}
