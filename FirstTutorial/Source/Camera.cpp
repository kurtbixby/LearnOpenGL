#define NOMINMAX

#define _USE_MATH_DEFINES
#include <cmath>

#include "Headers/Camera.h"

#include <iostream>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

const float MAX_PITCH = M_PI_2 - .001;
const float MIN_PITCH = -1 * MAX_PITCH;

const float MIN_FOV = 0.1f;
const float MAX_FOV = M_PI_4;

Camera::Camera(): Camera(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, -1.0f), glm::vec3(0.0f, 1.0f, 0.0f)) {}

Camera::Camera(const glm::vec3 position, const glm::vec3 target, const glm::vec3 up)
{
	position_ = position;
	direction_ = glm::normalize(target - position);
	up_ = glm::normalize(up);
	right_ = glm::normalize(glm::cross(direction_, up_));
	xzDirection_ = glm::normalize(glm::vec3(direction_.x, 0.0f, direction_.z));

	pitch_ = 0.0f;
	yaw_ = 0.0f;

	pitchSpeed_ = 0.1f;
	yawSpeed_ = 0.1f;
	transSpeed_ = 0.1f;

	pitchSensitivity_ = 0.003f;
	yawSensitivity_ = 0.003f;

	aspectRatio_ = 4 / 3.0f;
	fov_ = M_PI_4;

	projection_ = glm::perspective(fov_, aspectRatio_, 0.1f, 100.0f);
}

void Camera::SetAspectRatio(const float ratio)
{
	aspectRatio_ = ratio;
}


glm::mat4 Camera::GetProjection() const
{
	return projection_;
}

glm::vec3 Camera::GetPosition() const
{
	return position_;
}

glm::vec3 Camera::GetDirection() const
{
	return direction_;
}

glm::mat4 lookAt(const glm::vec3& position, const glm::vec3& target, const glm::vec3& up)
{
	glm::vec3 zAxis = glm::normalize(position - target);
	glm::vec3 xAxis = glm::normalize(glm::cross(up, zAxis));
	glm::vec3 yAxis = glm::normalize(glm::cross(zAxis, xAxis));

	glm::mat4 translation(1.0f);
	translation[3][0] = -position.x;
	translation[3][1] = -position.y;
	translation[3][2] = -position.z;

	glm::mat4 rotation(1.0f);
	rotation[0][0] = xAxis.x;
	rotation[1][0] = xAxis.y;
	rotation[2][0] = xAxis.z;
	rotation[0][1] = yAxis.x;
	rotation[1][1] = yAxis.y;
	rotation[2][1] = yAxis.z;
	rotation[0][2] = zAxis.x;
	rotation[1][2] = zAxis.y;
	rotation[2][2] = zAxis.z;

	return rotation * translation;
}

glm::mat4 Camera::MakeViewMat() const
{
	glm::mat4 view = glm::lookAt(position_, position_ + direction_, up_);
	//glm::mat4 view = lookAt(position_, position_ + direction_, up_);
	return view;
}

void Camera::TakeInput(const CameraInput input)
{
	if (input.MoveForward)
	{
		MoveForward();
	}
	if (input.MoveBack)
	{
		MoveBack();
	}
	if (input.MoveRight)
	{
		MoveRight();
	}
	if (input.MoveLeft)
	{
		MoveLeft();
	}
	if (input.RotateRight)
	{
		RotateRight();
	}
	if (input.RotateLeft)
	{
		RotateLeft();
	}
	if (input.y_delta > 0)
	{
		pitch_ = std::min(MAX_PITCH, pitch_ + (input.y_delta * pitchSensitivity_));
	}
	else
	{
		pitch_ = std::max(MIN_PITCH, pitch_ + (input.y_delta * pitchSensitivity_));
	}

	yaw_ = yaw_ + (input.x_delta * yawSensitivity_);

	if (input.y_offset > 0)
	{
		fov_ = std::max(MIN_FOV, fov_ - glm::radians(input.y_offset));
	}
	else
	{
		fov_ = std::min(MAX_FOV, fov_ - glm::radians(input.y_offset));
	}
	std::cout << "FOV: " << fov_ << std::endl;
	std::cout << "Delta: " << input.y_offset << std::endl;

	UpdateProjection();

	UpdateDirection();
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
}

void Camera::RotateLeft()
{
	yaw_ -= yawSpeed_;
}

void Camera::LookUp()
{
	pitch_ = std::min(MAX_PITCH, pitch_ + pitchSpeed_);
}

void Camera::LookDown()
{
	pitch_ = std::max(MIN_PITCH, pitch_ - pitchSpeed_);
}

void Camera::UpdateProjection()
{
	projection_ = glm::perspective(fov_, aspectRatio_, 0.1f, 100.0f);
}

void Camera::UpdateDirection()
{
	direction_.x = cos(yaw_) * cos(pitch_);
	direction_.y = sin(pitch_);
	direction_.z = sin(yaw_) * cos(pitch_);

	xzDirection_ = glm::normalize(glm::vec3(direction_.x, 0.0f, direction_.z));

	direction_ = glm::normalize(direction_);
	right_ = glm::normalize(glm::cross(direction_, up_));
}
