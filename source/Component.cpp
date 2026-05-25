#include "Component.h"

#include <utility>
CName::CName(const std::string& name) {
  m_name = std::move(name);
}

void CTransform::setPosition(const glm::vec3& pos) {
  m_position = pos;
}
void CTransform::setRotation(const glm::quat& rot) {
  m_rotation = rot;
}
void CTransform::setScale(const glm::vec3& scale) {
  m_scale = scale;
}
