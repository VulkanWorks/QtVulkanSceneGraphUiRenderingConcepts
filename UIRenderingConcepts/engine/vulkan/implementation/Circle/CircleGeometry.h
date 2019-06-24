#pragma once
#include "CircleShaderTypes.h"

static const CircleVertex circleFilledVertices[] =
{
    { glm::vec3(1, 0, 0),   glm::vec2(0.f, 0.f), 0 },
    { glm::vec3(0, 0, 0),   glm::vec2(1.f, 0.f), 0 },
    { glm::vec3(1, 1, 0),   glm::vec2(0.f, 1.f), 0 },
    { glm::vec3(1, 1, 0),   glm::vec2(0.f, 1.f), 0 },
    { glm::vec3(0, 0, 0),   glm::vec2(1.f, 0.f), 0 },
    { glm::vec3(0, 1, 0),   glm::vec2(1.f, 1.f), 0 },
};

static const CircleVertex rectOutlineVertices[] =
{
    { glm::vec3(0, 0, 0),   glm::vec2(0.f, 0.f), 0 },
    { glm::vec3(1, 0, 0),   glm::vec2(1.f, 0.f), 0 },
    { glm::vec3(1, 1, 0),   glm::vec2(0.f, 1.f), 0 },
    { glm::vec3(0, 1, 0),   glm::vec2(0.f, 1.f), 0 },
    { glm::vec3(0, 0, 0),   glm::vec2(0.f, 0.f), 0 },
};
