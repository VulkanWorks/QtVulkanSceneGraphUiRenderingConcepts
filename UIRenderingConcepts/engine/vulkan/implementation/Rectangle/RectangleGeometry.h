#pragma once

#include "RectangleShaderTypes.h"

static const Vertex s_RectFilledVertices[] =
{
    { glm::vec3(1, 0, 0),   glm::vec3(0.f, 0.f, 0.f) },
    { glm::vec3(0, 0, 0),   glm::vec3(1.f, 0.f, 0.f) },
    { glm::vec3(1, 1, 0),   glm::vec3(0.f, 1.f, 0.f) },
    { glm::vec3(1, 1, 0),   glm::vec3(0.f, 1.f, 0.f) },
    { glm::vec3(0, 0, 0),   glm::vec3(1.f, 0.f, 0.f) },
    { glm::vec3(0, 1, 0),   glm::vec3(1.f, 1.f, 0.f) },
};

static const Vertex s_RectOutlineVertices[] =
{
    { glm::vec3(0, 0, 0),   glm::vec3(0.f, 0.f, 0.f) },
    { glm::vec3(1, 0, 0),   glm::vec3(1.f, 0.f, 0.f) },
    { glm::vec3(1, 1, 0),   glm::vec3(0.f, 1.f, 0.f) },
    { glm::vec3(0, 1, 0),   glm::vec3(0.f, 1.f, 0.f) },
    { glm::vec3(0, 0, 0),   glm::vec3(0.f, 0.f, 0.f) },
};
