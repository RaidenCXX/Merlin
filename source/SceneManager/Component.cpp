#include "Component.h"

#include <utility>
mr::CName::CName(const std::string& name) {
  m_name = std::move(name);
}

void mr::CTransform::setPosition(const glm::vec3& pos) {
  m_position = pos;
}
void mr::CTransform::setRotation(const glm::quat& rot) {
  m_rotation = rot;
}
void mr::CTransform::setScale(const glm::vec3& scale) {
  m_scale = scale;
}
