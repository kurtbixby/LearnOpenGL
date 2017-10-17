#ifndef CAMERA_H
#define CAMERA_H

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

struct CameraInput
{
	unsigned char MoveForward : 1;
	unsigned char MoveBack : 1;
	unsigned char MoveLeft : 1;
	unsigned char MoveRight : 1;
	unsigned char RotateRight : 1;
	unsigned char RotateLeft : 1;
	unsigned char : 0;
	float x_delta;
	float y_delta;
};

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

	glm::mat4 MakeViewMat();
private:
	void Init(const glm::vec3 position, const glm::vec3 target, const glm::vec3 up);

	glm::vec3 position_;
	glm::vec3 direction_;
	glm::vec3 up_;
	glm::vec3 right_;

	float pitch_;
	float yaw_;

	float pitchSpeed_;
	float yawSpeed_;
	float transSpeed_;
	float pitchSensitivity_;
	float yawSensitivity_;

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
