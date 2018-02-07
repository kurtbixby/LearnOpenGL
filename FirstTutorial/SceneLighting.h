//
//  SceneLighting.h
//  FirstTutorial
//
//  Created by Kurt Bixby on 2/4/18.
//  Copyright © 2018 Bixby Productions. All rights reserved.
//

#ifndef SceneLighting_h
#define SceneLighting_h

#include <vector>

#include <glm/glm.hpp>

#include "IBufferable.h"
#include "Lights.h"

class SceneLighting : public IBufferable {
public:
    size_t DataSize();
    size_t BufferData(GLuint offset);
private:
    std::vector<Light> lights_;
    std::vector<PointLight> pointLights_;
    std::vector<SpotLight> spotLights_;
};

#endif /* SceneLighting_h */
