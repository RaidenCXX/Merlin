#pragma once

#include "glm/ext/matrix_float4x4.hpp"
#include "glm/gtc/quaternion.hpp"

#include <glm/ext/vector_float3.hpp>
#include <glm/fwd.hpp>

class Component {
  bool active = false;
};

class CTransform : public Component {
  glm::vec3 m_position = glm::vec3{0.0f};
  glm::quat m_rotation = glm::quat{1.0f, 0.0f, 0.0f, 0.0f};
  glm::vec3 m_scale = glm::vec3{1.0f};

public:
  void setPosition(const glm::vec3& pos);
  void setRotation(const glm::quat& rot);
  void setScale(const glm::vec3& scale);

  const glm::vec3& GetPosition() const { return m_position; }
  const glm::quat& GetRotation() const { return m_rotation; }
  const glm::vec3& GetScale() const { return m_scale; }
};

class CMesh : public Component {};

class CCamera : public Component {
  float fielfOfView = 45.f;
  float aspectRatio = 16.f / 9.f;
  float nearPlane = 0.1;
  float farPlane = 1000.f;

  glm::mat4 viewMatrix = glm::mat4{1.f};
  glm::mat4 projectionMatrix = glm::mat4{1.f};
  bool projDirty = true;

public:
};
