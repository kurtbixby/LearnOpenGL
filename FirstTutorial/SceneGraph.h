#ifndef SCENEGRAPH_H
#define SCENEGRAPH_H

#include <vector>
#include <iostream>

#include <glm/glm.hpp>

#include "Camera.h"
#include "Lights.h"
#include "Object.h"
#include "SceneLighting.h"

class SceneGraph
{
public:
	SceneGraph();
	// SceneGraph(ifstream istream);

	void UseCamera(const Camera& camera);
	std::vector<Object> RelevantObjects();
    
    SceneLighting RelevantLighting();
	std::vector<Light> RelevantLights();
	std::vector<PointLight> RelevantPointLights();
	std::vector<SpotLight> RelevantSpotLights();

private:
	std::vector<Object> objects_; // Replace this with a legit scene object graph

	std::vector<Light> lights_;
	std::vector<PointLight> pointLights_;
	std::vector<SpotLight> spotLights_;

	glm::vec3 camPosition_;
	glm::vec3 camDirection_;

	void SortObjects();
	bool ObjectComparison(const Object& objA, const Object& objB);
};

#endif
