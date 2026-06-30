#include "SlangCompiler.h"

#include "../ServiceLocator.h"
#include "slang/slang-com-ptr.h"
#include "slang/slang.h"

#include <array>
#include <cstdint>
#include <string>

void slangDiagnostic(const std::string& id, SlangResult result,
                     Slang::ComPtr<slang::IBlob> diagnosticsBlob, const std::string& stage,
                     mr::Messenger* messenger);

void mr::SlangCompiler::init() {
  mr::Messenger* messenger = mr::ServiceLocator::getMessenger();
  if (!(slang::createGlobalSession(m_globalSesion.writeRef()) >= 0)) {
    messenger->sendMessage("ERROR", "SlangCompiler::init", "The session was not created");
    return;
  }
}

void mr::SlangCompiler::shutdown() {
  m_globalSesion = nullptr;
}

void mr::SlangCompiler::compileVk(const std::string& id, const std::string& path,
                                  const std::string& entryPoint, std::vector<char>& outSpirv) {
  mr::Messenger* messenger = mr::ServiceLocator::getMessenger();
  if (messenger == nullptr) {
    return;
  }

  slang::TargetDesc targetDesc{};
  targetDesc.format = SLANG_SPIRV;
  targetDesc.profile = m_globalSesion->findProfile("sm_6_5");
  targetDesc.flags = SLANG_TARGET_FLAG_GENERATE_SPIRV_DIRECTLY;

  // Options
  std::array<slang::CompilerOptionEntry, 3> options;
  // Compile with out hlsl step
  options[0].name = slang::CompilerOptionName::EmitSpirvDirectly;
  options[0].value =
    slang::CompilerOptionValue{slang::CompilerOptionValueKind::Int, 1, 0, nullptr, nullptr};
  // Optimization level 0-3
  options[1].name = slang::CompilerOptionName::Optimization;
  options[1].value =
    slang::CompilerOptionValue{slang::CompilerOptionValueKind::Int, 3, 0, nullptr, nullptr};
  // Debug info
  options[2].name = slang::CompilerOptionName::DebugInformation;
  options[2].value =
    slang::CompilerOptionValue{slang::CompilerOptionValueKind::Int, 2, 0, nullptr, nullptr};

  // Session creation
  slang::SessionDesc sessionDesc{};
  sessionDesc.targets = &targetDesc;
  sessionDesc.targetCount = 1;
  sessionDesc.compilerOptionEntries = options.data();
  sessionDesc.compilerOptionEntryCount = options.size();
  sessionDesc.searchPaths = mr::ServiceLocator::getSettings()->systemPaths.shadersFolderPath.data();
  sessionDesc.searchPathCount =
    static_cast<uint32_t>(mr::ServiceLocator::getSettings()->systemPaths.shadersFolderPath.size());

  Slang::ComPtr<slang::ISession> session;
  m_globalSesion->createSession(sessionDesc, session.writeRef());

  // Slang Module
  Slang::ComPtr<slang::IModule> shaderModule;
  {
    Slang::ComPtr<slang::IBlob> diagnosticsBlob;
    shaderModule = session->loadModule(id.data(), diagnosticsBlob.writeRef());
    slangDiagnostic(id, shaderModule ? SLANG_OK : SLANG_FAIL, diagnosticsBlob, "loadModule",
                    messenger);
    if (!shaderModule)
      return;
  }

  // Entry point
  Slang::ComPtr<slang::IEntryPoint> ePoint;
  {
    Slang::ComPtr<slang::IBlob> diagnosticsBlob;
    shaderModule->findEntryPointByName(entryPoint.data(), ePoint.writeRef());
    if (!ePoint) {
      messenger->sendMessage("ERROR", "SlangCompiler::compileVk",
                             "Failed to find a entry point " + id);
      return;
    }
  }

  // Composition
  std::array<slang::IComponentType*, 2> componentTypes{};
  componentTypes[0] = shaderModule;
  componentTypes[1] = ePoint;

  Slang::ComPtr<slang::IComponentType> composedProgram;
  {
    Slang::ComPtr<slang::IBlob> diagnosticsBlob;
    SlangResult result =
      session->createCompositeComponentType(componentTypes.data(), componentTypes.size(),
                                            composedProgram.writeRef(), diagnosticsBlob.writeRef());
    slangDiagnostic(id, result, diagnosticsBlob, "composition", messenger);
    if (SLANG_FAILED(result))
      return;
  }

  // Link
  Slang::ComPtr<slang::IComponentType> linkedProgram;
  {
    Slang::ComPtr<slang::IBlob> diagnosticsBlob;
    SlangResult result =
      composedProgram->link(linkedProgram.writeRef(), diagnosticsBlob.writeRef());
    slangDiagnostic(id, result, diagnosticsBlob, "link", messenger);
    if (SLANG_FAILED(result))
      return;
  }

  // SPIR-V
  Slang::ComPtr<slang::IBlob> spirvCode;
  {
    Slang::ComPtr<slang::IBlob> diagnosticsBlob;
    SlangResult result =
      linkedProgram->getEntryPointCode(0, 0, spirvCode.writeRef(), diagnosticsBlob.writeRef());
    slangDiagnostic(id, result, diagnosticsBlob, "getEntryPointCode", messenger);
    if (SLANG_FAILED(result))
      return;
  }

  outSpirv.resize(spirvCode->getBufferSize());
  memcpy(outSpirv.data(), spirvCode->getBufferPointer(), spirvCode->getBufferSize());
}

void slangDiagnostic(const std::string& id, SlangResult result,
                     Slang::ComPtr<slang::IBlob> diagnosticsBlob, const std::string& stage,
                     mr::Messenger* messenger) {
  if (!diagnosticsBlob)
    return;

  const char* severity = SLANG_FAILED(result) ? "ERROR" : "WARNING";
  const char* msg = static_cast<const char*>(diagnosticsBlob->getBufferPointer());

  std::string fullMessage = stage + ": " + msg + " (" + id + ")";

  messenger->sendMessage(severity, "SlangCompiler::compileVk", fullMessage);
}
