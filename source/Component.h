#pragma once

#include "Entity.h"
#include "glm/ext/matrix_float4x4.hpp"
#include "glm/gtc/quaternion.hpp"
#include "resource/Resource.h"
#include "resource/ResourceHandle.h"

#include <cstdint>
#include <glm/ext/vector_float3.hpp>
#include <glm/fwd.hpp>
#include <string>

enum class ComponentBit : uint64_t {
  CTransform = 1 << 0,
  CMesh = 1 << 1,
  CCamera = 1 << 2,
  CTag = 1 << 3,
  CName = 1 << 4,
  CMask = 1 << 5,
  CTexture = 1 << 6
};

enum class EntityTag : uint32_t { None = 0, Props, Env, Player, Character };

// 0
class CTransform {
  glm::vec3 m_position = glm::vec3{0.0f};
  glm::quat m_rotation = glm::quat{1.0f, 0.0f, 0.0f, 0.0f};
  glm::vec3 m_scale = glm::vec3{1.0f};

public:
  static constexpr uint64_t bit = static_cast<uint64_t>(ComponentBit::CTransform);

  CTransform() = default;

  void setPosition(const glm::vec3& pos);
  void setRotation(const glm::quat& rot);
  void setScale(const glm::vec3& scale);

  const glm::vec3& GetPosition() const { return m_position; }
  const glm::quat& GetRotation() const { return m_rotation; }
  const glm::vec3& GetScale() const { return m_scale; }
};

// 1
class CCamera {
  float m_fielfOfView = 45.f;
  float m_aspectRatio = 16.f / 9.f;
  float m_nearPlane = 0.1;
  float m_farPlane = 1000.f;

  glm::mat4 m_viewMatrix = glm::mat4{1.f};
  glm::mat4 m_projectionMatrix = glm::mat4{1.f};
  bool m_projDirty = true;

public:
  static constexpr uint64_t bit = static_cast<uint64_t>(ComponentBit::CCamera);
};

// 2
class CMesh {
public:
  static constexpr uint64_t bit = static_cast<uint64_t>(ComponentBit::CMesh);
};

// 3
class CTag {
public:
  static constexpr uint64_t bit = static_cast<uint64_t>(ComponentBit::CTag);

  EntityTag m_tag = EntityTag::None;

  bool operator==(EntityTag tag) { return m_tag == tag; }
};

class CTexture {
public:
  static constexpr uint64_t bit = static_cast<uint64_t>(ComponentBit::CTexture);
  ResourceHandle<Texture> m_texture;
};

// 4
class CName {
public:
  static constexpr uint64_t bit = static_cast<uint64_t>(ComponentBit::CName);

  std::string m_name = "None";
  CName(const std::string& name);
};
