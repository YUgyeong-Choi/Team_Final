#pragma once

#include "Base.h"

NS_BEGIN(Engine)

class CRenderer final : public CBase
{
private:
	CRenderer(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual ~CRenderer() = default;

public:
	HRESULT Initialize();
	HRESULT Add_RenderGroup(RENDERGROUP eRenderGroup, class CGameObject* pRenderObject);	
	HRESULT Draw();

	void ClearRenderObjects();
#ifdef _DEBUG
public:
	HRESULT Add_DebugComponent(class CComponent* pDebugCom);
	void Set_RenderTarget() { m_bRenderTarget = !m_bRenderTarget; }
	void Set_RenderCollider() { m_bRenderCollider = !m_bRenderCollider; }
	_bool Get_RenderCollider() { return m_bRenderCollider; }
	void Change_DebugRT() { ++m_eDebugRT == DEBUGRT_END ? m_eDebugRT = DEBUGRT_DH : m_eDebugRT; }
#endif

private:
	ID3D11Device*				m_pDevice = { nullptr };
	ID3D11DeviceContext*		m_pContext = { nullptr };
	class CGameInstance*		m_pGameInstance = { nullptr };

private:
	class CVIBuffer_Rect*		m_pVIBuffer = { nullptr };
	class CShader*				m_pShader = { nullptr };

	_float4x4					m_WorldMatrix{}, m_ViewMatrix{}, m_ProjMatrix{};

	ID3D11DepthStencilView*		m_pShadowDSV = { nullptr };
	ID3D11DepthStencilView*		m_pBlurDSV = { nullptr };
	_uint						m_iOriginalViewportWidth{}, m_iOriginalViewportHeight{};

private:
	_uint m_iCurrentRenderLevel = 0;

private:
	list<class CGameObject*>	m_RenderObjects[ENUM_CLASS(RENDERGROUP::RG_END)];
	std::chrono::steady_clock::time_point m_StartTime = std::chrono::steady_clock::now();

#ifdef _DEBUG
private:
	list<class CComponent*>		m_DebugComponent;
	_bool m_bRenderTarget = false;
	_bool m_bRenderCollider = false;
	enum DEBUGRT { DEBUGRT_DH, DEBUGRT_YW, DEBUGRT_CY, DEBUGRT_END };
	_uint m_eDebugRT = static_cast<_uint>(DEBUGRT_DH);
#endif

private:
	_float m_fDownscaledRatio = {};

private:
	HRESULT Render_Priority();
	HRESULT Render_Shadow();
	HRESULT Render_NonBlend();
	HRESULT Render_PBRMesh();
	HRESULT Render_Blend();
	HRESULT Render_UI();
	HRESULT Render_Lights();
	HRESULT Render_PBRLights();
	HRESULT Render_Volumetric();
	HRESULT Render_BackBuffer();
	HRESULT Render_NonLight();
	HRESULT Render_UI_Deferred();
	HRESULT Render_Effect_Blend();
	HRESULT Render_Blur(const _wstring& strTargetTag);
	HRESULT Render_Effect_Glow();
	HRESULT Render_Effect_WBGlow();
	HRESULT Render_Effect_NonLight();
	HRESULT Render_Effect_WB();
	HRESULT Render_Effect_WB_Composite();


private:
	HRESULT Ready_DepthStencilView_Shadow(_uint iWidth, _uint iHeight);
	HRESULT Ready_DepthStencilView_Blur(_uint iWidth, _uint iHeight);
	HRESULT Change_ViewportDesc(_uint iWidth, _uint iHeight);
	HRESULT Add_Component(_uint iPrototypeLevelIndex, const _wstring& strPrototypeTag, const _wstring& strComponentTag, CComponent** ppOut, void* pArg = nullptr);


#ifdef _DEBUG
private:
	HRESULT Render_Debug();
#endif

public:
	static CRenderer* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual void Free() override;
};

NS_END