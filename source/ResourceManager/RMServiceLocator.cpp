#include "RMServiceLocator.h"

#include "SlangCompiler.h"

mr::SlangCompiler* mr::RMServiceLocator::m_slangCompiler = nullptr;

void mr::RMServiceLocator::provideSlangCompiler(mr::SlangCompiler* slangCompiler) {
  if (slangCompiler != nullptr)
    m_slangCompiler = slangCompiler;
}

mr::SlangCompiler* mr::RMServiceLocator::getSlangCompiler() {
  return m_slangCompiler;
}
