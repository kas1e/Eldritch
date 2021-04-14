#include "core.h"
#include "eldritchtargetmanager.h"
#include "3d.h"
#include "irenderer.h"
#include "irendertarget.h"
#include "eldritchgame.h"
#include "Screens/uiscreen-eldmirror.h"
#include "configmanager.h"

EldritchTargetManager::EldritchTargetManager(IRenderer* const pRenderer)
    : m_Renderer(pRenderer),
      m_OriginalRenderTarget(nullptr),
#ifndef NO_POST
      m_PrimaryRenderTarget(nullptr),
#endif
      m_MirrorRenderTarget(nullptr),
      m_MinimapRenderTarget(nullptr) {}

EldritchTargetManager::~EldritchTargetManager() { ReleaseTargets(); }

void EldritchTargetManager::CreateTargets(const uint DisplayWidth,
                                          const uint DisplayHeight) {
  ReleaseTargets();

  m_OriginalRenderTarget = m_Renderer->GetDefaultRenderTarget();
#ifndef NO_POST
  m_PrimaryRenderTarget = m_Renderer->CreateRenderTarget(SRenderTargetParams(
      DisplayWidth, DisplayHeight, ERTF_A16B16G16R16F, ERTF_UseDefault));
#endif
  // HACK: Grabbing mirror UI screen to get desired RT dimensions.
  UIScreenEldMirror* const pMirrorScreen = EldritchGame::GetMirrorScreen();
  const uint MirrorRTWidth = pMirrorScreen->GetMirrorRTWidth();
  const uint MirrorRTHeight = pMirrorScreen->GetMirrorRTHeight();

  m_MirrorRenderTarget = m_Renderer->CreateRenderTarget(SRenderTargetParams(
      MirrorRTWidth, MirrorRTHeight, ERTF_A16B16G16R16F, ERTF_UseDefault));

  STATICHASH(EldMinimap);
  STATICHASH(MinimapRTWidth);
  STATICHASH(MinimapRTHeight);
  const uint MinimapRTWidth =
      ConfigManager::GetInt(sMinimapRTWidth, 0, sEldMinimap);
  const uint MinimapRTHeight =
      ConfigManager::GetInt(sMinimapRTHeight, 0, sEldMinimap);
  m_MinimapRenderTarget = m_Renderer->CreateRenderTarget(SRenderTargetParams(
      MinimapRTWidth, MinimapRTHeight, ERTF_A16B16G16R16F, ERTF_UseDefault));
}

void EldritchTargetManager::ReleaseTargets() {
  m_Renderer->FreeRenderTargets();

  // Don't free the original render target! The renderer owns that.
#ifndef NO_POST
  SafeDelete(m_PrimaryRenderTarget);
#endif
  SafeDelete(m_MirrorRenderTarget);
  SafeDelete(m_MinimapRenderTarget);
}