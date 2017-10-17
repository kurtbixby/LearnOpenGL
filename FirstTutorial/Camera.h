#ifndef CAMERA_H
#define CAMERA_H

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

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

	void MoveForward();
	void MoveBack();
	void MoveRight();
	void MoveLeft();
	void RotateRight();
	void RotateLeft();
	void LookUp();
	void LookDown();

	glm::mat4 MakeViewMat();
private:
	glm::vec3 position_;
	glm::vec3 direction_;
	glm::vec3 up_;
	glm::vec3 right_;

	float pitch_;
	float yaw_;

	float pitchSpeed_;
	float yawSpeed_;
	float transSpeed_;

	void UpdateDirection();
};

#endif
