#pragma once

#include "Base.h"

NS_BEGIN(Engine)

class CLight_Manager final : public CBase
{
public:
	enum class LEVEL { STATIC, LOADING, LOGO, KRAT_CENTERAL_STATION, YG, CY, GL, DH, YW, JW, END };

private:
	CLight_Manager();
	virtual ~CLight_Manager() = default;

public:
	const LIGHT_DESC* Get_Light(_uint iIndex);

public:
	HRESULT Add_Light(const LIGHT_DESC& LightDesc);
	HRESULT Add_LevelLightData(_uint iLevelIndex, const LIGHT_DESC& LightDesc);
	HRESULT Render_Lights(class CShader* pShader, class CVIBuffer_Rect* pVIBuffer);
	HRESULT Render_PBR_Lights(class CShader* pShader, class CVIBuffer_Rect* pVIBuffer, _uint Level);

private:
	list<class CLight*>	m_Lights;
	unordered_map<_uint, list<class CLight*>> m_LevelLights;
	

public:
	static CLight_Manager* Create();
	virtual void Free() override;
};

NS_END