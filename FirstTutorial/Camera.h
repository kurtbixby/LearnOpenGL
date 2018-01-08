#ifndef CAMERA_H
#define CAMERA_H

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "Structs.h"

// Don't mess with roll or y-axis translation
class Camera
{
public:
	// Creates camera
	// 		pos @ 0,0,0
	// 		target direction @ 0,0,-1
	// 		up @ 0,1,0
	Camera();
	Camera(const glm::vec3 position, const glm::vec3 target, const glm::vec3 up);

	void TakeInput(CameraInput input);

	void SetAspectRatio(const float ratio);
	glm::mat4 GetProjection() const;
	glm::mat4 MakeViewMat() const;
	glm::vec3 GetPosition() const;
	glm::vec3 GetDirection() const;

private:
	glm::mat4 projection_;

	glm::vec3 position_;
	glm::vec3 direction_;
	glm::vec3 xzDirection_;
	glm::vec3 up_;
	glm::vec3 right_;

	float pitch_;
	float yaw_;
	float fov_;
	float aspectRatio_;

	float pitchSpeed_;
	float yawSpeed_;
	float transSpeed_;
	float pitchSensitivity_;
	float yawSensitivity_;

	void UpdateProjection();
	void UpdateDirection();
	void MoveForward();
	void MoveBack();
	void MoveRight();
	void MoveLeft();
	void RotateRight();
	void RotateLeft();
	void LookUp();
	void LookDown();
};

#endif
