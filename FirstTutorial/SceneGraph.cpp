#include "SceneGraph.h"

#include <vector>
#include <algorithm>

#include <glm/glm.hpp>

#include "Camera.h"
#include "Lights.h"

SceneGraph::SceneGraph()
{
    camPosition_ = glm::vec3(0.0f);
    camDirection_ = glm::vec3(0.0f, 0.0f, -1.0f);

    objects_ = std::vector<Object>();
    objects_.push_back(Object(glm::vec3(0.0f), 1, 1.0f, false, true));
    objects_.push_back(Object(glm::vec3(-1.0f, 0.0f, -1.0f), 0, 1.0f, false));
    objects_.push_back(Object(glm::vec3(2.0f, 0.0f, 0.0f), 0, 1.0f, false));
    objects_.push_back(Object(glm::vec3(-1.5f,  0.0f, -0.48f), 2, 1.0f, false, true, true));
    objects_.push_back(Object(glm::vec3( 1.5f,  0.0f,  0.51f), 2, 1.0f, false, true, true));
    objects_.push_back(Object(glm::vec3( 0.0f,  0.0f,  0.7f), 2, 1.0f, false, true, true));
    objects_.push_back(Object(glm::vec3(-0.3f,  0.0f, -2.3f), 2, 1.0f, false, true, true));
    objects_.push_back(Object(glm::vec3( 0.5f,  0.0f, -0.6f), 2, 1.0f, false, true, true));
	objects_.push_back(Object(glm::vec3(0.0f, 0.0f, -3.0f), 3, 1.0f, false));

	lights_ = std::vector<Light>();
	Light dirLight;
	dirLight.direction = glm::vec3(-12.0f, -10.0f, 20.0f);
	dirLight.ambient = ColorConstants::WhiteLight() * 0.5f;
	dirLight.diffuse = ColorConstants::WhiteLight() * 0.2f;
	dirLight.specular = glm::vec3(1.0f);
	lights_.push_back(dirLight);

	pointLights_ = std::vector<PointLight>();
	PointLight pLight;
    pLight.position = glm::vec3(0.0f);
    pLight.ambient = ColorConstants::GreenLight() * 0.5f;
    pLight.diffuse = ColorConstants::GreenLight() * 0.2f;
    pLight.specular = glm::vec3(1.0f);
    pLight.constant = 1.0f;
    pLight.linear = 0.09f;
    pLight.quadratic = 0.032f;
    //pointLights_.push_back(pLight);

    // pLight2 is the same as pLight1, but in a different location
    pLight.position = glm::vec3(-5.0f);
    //pointLights_.push_back(pLight);

	spotLights_ = std::vector<SpotLight>();
	SpotLight spLight;
    spLight.position = glm::vec3(camPosition_.x, camPosition_.y, camPosition_.z);
    spLight.direction = glm::vec3(camDirection_.x, camDirection_.y, camDirection_.z);
    spLight.ambient = ColorConstants::BlueLight() * 0.5f;
    spLight.diffuse = ColorConstants::BlueLight() * 0.2f;
    spLight.specular = glm::vec3(1.0f);
    spLight.innerCutoff = cos(glm::radians(10.0f));
    spLight.outerCutoff = cos(glm::radians(15.0f));

    //spotLights_.push_back(spLight);
}

void SceneGraph::UseCamera(const Camera& camera)
{
    camPosition_ = camera.GetPosition();
    camDirection_ = camera.GetDirection();
    // Hack
    // Makes first spotlight paired to the camera
    if (spotLights_.size() > 0)
    {
        spotLights_[0].position = camPosition_;
        spotLights_[0].direction = camDirection_;
    }
    SortObjects();
}

std::vector<Object> SceneGraph::RelevantObjects()
{
	return objects_;
}

std::vector<Light> SceneGraph::RelevantLights()
{
	return lights_;
}

std::vector<PointLight> SceneGraph::RelevantPointLights()
{
    return pointLights_;
}

std::vector<SpotLight> SceneGraph::RelevantSpotLights()
{
    return spotLights_;
}

void SceneGraph::SortObjects()
{
    // Sort requires a static, 2 argument function
    // Lambdas are static, but this one captures the proper "this" variable
    // Member functions are called used an implicit "this"
    std::sort(objects_.begin(), objects_.end(), [this](const Object& a, const Object& b)
        {
            return ObjectComparison(a, b);
        }
    );
}

bool SceneGraph::ObjectComparison(const Object& objA, const Object& objB)
{
    glm::vec3 aRelative = objA.Transform_ - camPosition_;
    glm::vec3 bRelative = objB.Transform_ - camPosition_;

    float aDot = glm::dot(aRelative, camDirection_);
    float bDot = glm::dot(bRelative, camDirection_);

    return aDot > bDot;
}
