#include "SlangCompiler.h"

#include "../ServiceLocator.h"
#include "slang/slang.h"

bool mr::SlangCompiler::init() {
  return SLANG_SUCCEEDED(slang::createGlobalSession(m_globalSesion.writeRef()));
}

void mr::SlangCompiler::shutdown() {
  m_globalSesion = nullptr;
}

bool mr::SlangCompiler::compile(const std::string& path, const std::string& entryPoint,
                                std::vector<char>& outSpirv) {
  mr::Messenger* messenger = mr::ServiceLocator::getMessenger();

  using namespace Slang;

  // 1. Налаштування target
  slang::TargetDesc targetDesc{};
  targetDesc.format = SLANG_SPIRV;
  targetDesc.profile = m_globalSesion->findProfile("spirv_1_5");

  // 2. Створення session для цієї компіляції
  slang::SessionDesc sessionDesc{};
  sessionDesc.targets = &targetDesc;
  sessionDesc.targetCount = 1;

  ComPtr<slang::ISession> session;
  if (SLANG_FAILED(m_globalSesion->createSession(sessionDesc, session.writeRef()))) {
    messenger->sendMessage("ERROR", "SlangCompiler::compile", "Failed to create Slang session");
    return false;
  }

  // 3. Завантаження модуля
  ComPtr<slang::IBlob> diagnostics;
  slang::IModule* module = session->loadModule(path.c_str(), diagnostics.writeRef());
  if (!module) {
    if (diagnostics)
      std::cerr << (const char*)diagnostics->getBufferPointer() << "\n";
    return false;
  }

  // 4. Пошук entry point
  ComPtr<slang::IEntryPoint> entryPointPtr;
  if (SLANG_FAILED(module->findEntryPointByName(entryPoint.c_str(), entryPointPtr.writeRef()))) {
    std::cerr << "Entry point not found: " << entryPoint << "\n";
    return false;
  }

  // 5. Компонування програми
  std::vector<slang::IComponentType*> components = {module, entryPointPtr};
  ComPtr<slang::IComponentType> program;
  if (SLANG_FAILED(m_globalSesion->createCompositeComponentType(components.data(),
                                                                components.size(),
                                                                program.writeRef()))) {
    return false;
  }

  // 6. Лінкування
  ComPtr<slang::IComponentType> linkedProgram;
  if (SLANG_FAILED(program->link(linkedProgram.writeRef(), diagnostics.writeRef()))) {
    if (diagnostics)
      std::cerr << (const char*)diagnostics->getBufferPointer() << "\n";
    return false;
  }

  // 7. Отримання SPIR-V коду
  ComPtr<slang::IBlob> spirvBlob;
  if (SLANG_FAILED(
        linkedProgram->getEntryPointCode(0, 0, spirvBlob.writeRef(), diagnostics.writeRef()))) {
    if (diagnostics)
      std::cerr << (const char*)diagnostics->getBufferPointer() << "\n";
    return false;
  }

  // 8. Копіювання в outSpirv
  outSpirv.resize(spirvBlob->getBufferSize());
  memcpy(outSpirv.data(), spirvBlob->getBufferPointer(), spirvBlob->getBufferSize());

  return true;
}
