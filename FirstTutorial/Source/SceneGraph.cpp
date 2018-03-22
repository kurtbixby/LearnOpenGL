#include "Headers/SceneGraph.h"

#include <vector>
#include <algorithm>
#include <unordered_map>
#include <ctime>

#include <glm/glm.hpp>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/rotate_vector.hpp>

#include "Headers/Camera.h"
#include "Headers/Lights.h"
#include "Headers/Object.h"
#include "Headers/SceneLighting.h"

#define ORBIT_OBJS 100
#define ORBIT_START 10
#define ORBIT_SIZE 40

SceneGraph::SceneGraph() : SceneGraph(0)
{}

SceneGraph::SceneGraph(uint32_t number)
{
    objects_ = std::vector<Object>();
    
    lights_ = std::vector<Light>();
    pointLights_ = std::vector<PointLight>();
    spotLights_ = std::vector<SpotLight>();
    
    camPosition_ = glm::vec3(0.0f);
    camDirection_ = glm::vec3(0.0f, 0.0f, -1.0f);
    
    switch (number)
    {
        case 1:
        {
            LightColorData green;
            green.ambient = ColorConstants::GreenLight() * 0.2f;
            green.diffuse = ColorConstants::GreenLight() * 0.5f;
            green.specular = ColorConstants::GreenLight() * 0.7f;
            
            LightColorData white;
            white.ambient = ColorConstants::WhiteLight() * 0.2f;
            white.diffuse = ColorConstants::WhiteLight() * 0.5f;
            white.specular = ColorConstants::WhiteLight() * 0.7f;
            
            LightColorData red;
            red.ambient = ColorConstants::RedLight() * 0.2f;
            red.diffuse = ColorConstants::RedLight() * 0.5f;
            red.specular = ColorConstants::RedLight() * 0.7f;
            
            Light grnLight = Light(white, glm::vec3(15.0f, 0.5f, 15.0f));
            Light redLight = Light(red, glm::vec3(0.0f, 0.0f, 0.0f));
            
            lights_.push_back(grnLight);
            lights_.push_back(redLight);
            
            Object obj = Object(glm::vec3(0.0f), 0, glm::vec3(1.0f, 1.0f, 5.0f), 0);
            objects_.push_back(obj);
            break;
        }
        case 2:
        {
            objects_.push_back(Object(glm::vec3(0.0f), 0, glm::vec3(1.0f, 1.0f, 10.0f), false));
            
            LightColorData green;
            green.ambient = ColorConstants::GreenLight() * 0.2f;
            green.diffuse = ColorConstants::GreenLight() * 0.5f;
            green.specular = ColorConstants::GreenLight() * 0.7f;
            
            LightColorData white;
            white.ambient = ColorConstants::WhiteLight() * 100.0f;
            white.diffuse = ColorConstants::WhiteLight() * 100.0f;
            white.specular = ColorConstants::WhiteLight() * 100.0f;
            
            LightColorData red;
            red.ambient = ColorConstants::RedLight() * 0.2f;
            red.diffuse = ColorConstants::RedLight() * 0.5f;
            red.specular = ColorConstants::RedLight() * 0.7f;
            
            PointLight grnLight = PointLight(green, glm::vec3(0.0f, 0.0f, 0.0f));
            PointLight redLight = PointLight(red, glm::vec3(0.0f, 0.0f, 0.0f));
            PointLight whiteLight = PointLight(white, glm::vec3(0.0f, 0.0f, 0.0f));
            
            pointLights_.push_back(whiteLight);
            pointLights_.push_back(grnLight);
            pointLights_.push_back(redLight);
            
            break;
        }
        case 0:
        default:
        {
            objects_.push_back(Object(glm::vec3(0.0f), 0, glm::vec3(1.0f), false));
            
            glm::vec3 orbit_axis = glm::vec3(0.0f, 1.0f, 0.0f);
            srand(11101991);
            uint32_t instance_model = 1;
            for (int i = 0; i < ORBIT_OBJS; i++)
            {
                float random = (rand() % 1000) / 1000.0f;
                float distance = random * ORBIT_SIZE + ORBIT_START;
                glm::vec3 position = glm::vec3(distance, 0.0f, 0.0f);
                float angle = ((rand() % 1000) * 360) / 1000.0f;
                glm::vec3 rotated_position = glm::rotate(position, angle, orbit_axis);
                Object new_obj = Object(rotated_position, instance_model, glm::vec3(1.0), false);
                objects_.push_back(new_obj);
            }
            
            Object plane = Object(glm::vec3(0.0f, -0.75f, 0.0f), 2, glm::vec3(10.0f), false, true);
            objects_.push_back(plane);
            
            /**
             *  Old Scene
             **
             objects_.push_back(Object(glm::vec3(0.0f), 1, 1.0f, false, true));
             objects_.push_back(Object(glm::vec3(-1.0f, 0.0f, -1.0f), 0, 1.0f, false));
             objects_.push_back(Object(glm::vec3(2.0f, 0.0f, 0.0f), 0, 1.0f, false));
             objects_.push_back(Object(glm::vec3(-1.5f,  0.0f, -0.48f), 2, 1.0f, false, true, true));
             objects_.push_back(Object(glm::vec3( 1.5f,  0.0f,  0.51f), 2, 1.0f, false, true, true));
             objects_.push_back(Object(glm::vec3( 0.0f,  0.0f,  0.7f), 2, 1.0f, false, true, true));
             objects_.push_back(Object(glm::vec3(-0.3f,  0.0f, -2.3f), 2, 1.0f, false, true, true));
             objects_.push_back(Object(glm::vec3( 0.5f,  0.0f, -0.6f), 2, 1.0f, false, true, true));
             objects_.push_back(Object(glm::vec3(0.0f, 0.0f, -3.0f), 3, 1.0f, false));
             */
            
            // Directional Light Creation
            LightColorData white;
            white.ambient = ColorConstants::WhiteLight() * 0.5f;
            white.diffuse = ColorConstants::WhiteLight() * 0.2f;
            white.specular = glm::vec3(1.0f);
            
            //    glm::vec3 light_direction = glm::vec3(-12.0f, -10.0f, 20.0f);
            glm::vec3 light_direction = glm::vec3(-12.0f, -10.0f, 20.0f);
            
            Light dirLight = Light(white, light_direction);
            lights_.push_back(dirLight);
            
            // Point Light Creation
            LightColorData green;
            green.ambient = ColorConstants::GreenLight() * 0.2f;
            green.diffuse = ColorConstants::GreenLight() * 0.5f;
            green.specular = ColorConstants::GreenLight() * 0.7f;
            
            LightColorData red;
            red.ambient = ColorConstants::RedLight() * 0.2f;
            red.diffuse = ColorConstants::RedLight() * 0.5f;
            red.specular = ColorConstants::RedLight() * 0.7f;
            
            glm::vec3 point_position = glm::vec3(0.0f, 2.0f, -3.0f);
            float point_constant = 1.0f;
            float point_linear = 0.09f;
            float point_quadratic = 0.032f;
            PointLight pLight = PointLight(green, point_position, point_constant, point_linear, point_quadratic);
            pointLights_.push_back(pLight);
            
            // pLight2 is the same as pLight1, but in a different location
            PointLight pLight2 = PointLight(red, point_position, point_constant, point_linear, point_quadratic);
            pLight2.ChangePosition(glm::vec3(3.0f, 2.0f, 00.0f));
            pointLights_.push_back(pLight2);
            
            // Spot Light Creation
            LightColorData blue;
            blue.ambient = ColorConstants::BlueLight() * 0.5f;
            blue.diffuse = ColorConstants::BlueLight() * 0.2f;
            blue.specular = ColorConstants::BlueLight() * 0.7f;
            
            glm::vec3 spot_position = glm::vec3(camPosition_.x, camPosition_.y, camPosition_.z);
            glm::vec3 spot_direction = glm::vec3(camDirection_.x, camDirection_.y, camDirection_.z);
            float spot_inner = std::cos(glm::radians(10.0f));
            float spot_outer = std::cos(glm::radians(15.0f));
            
            SpotLight spLight = SpotLight(blue, spot_position, spot_direction, spot_inner, spot_outer);
            spotLights_.push_back(spLight);
            break;
        }
    }
}

void SceneGraph::UseCamera(const Camera& camera)
{
    camPosition_ = camera.GetPosition();
    camDirection_ = camera.GetDirection();
    glm::vec3 right = camera.Right();
    // Hack
    // Makes first spotlight paired to the camera
    if (spotLights_.size() > 0)
    {
        spotLights_[0].ChangePosition(camPosition_ + 1.5f * right);
        spotLights_[0].ChangeDirection(camDirection_ - 0.25f * right);
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

SceneLighting SceneGraph::RelevantLighting()
{
    return SceneLighting(lights_, pointLights_, spotLights_);
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
    glm::vec3 aRelative = objA.Transform() - camPosition_;
    glm::vec3 bRelative = objB.Transform() - camPosition_;

    float aDot = glm::dot(aRelative, camDirection_);
    float bDot = glm::dot(bRelative, camDirection_);

    return aDot > bDot;
}
