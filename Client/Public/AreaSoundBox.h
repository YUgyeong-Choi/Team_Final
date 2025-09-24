#pragma once

#include "Client_Defines.h"
#include "TriggerBox.h"

NS_BEGIN(Engine)
class CSoundController;
NS_END

NS_BEGIN(Client)

class CAreaSoundBox : public CTriggerBox
{
public:
	typedef struct tagAreaSoundBoxDesc : public CTriggerBox::TRIGGERBOX_DESC
	{
		TRIGGERSOUND_TYPE	eTriggerBoxType;
		const _tchar*		szSoundID;
		_float3				vPosition;
		string				strSoundName;
		_float2 			fMinMax = { 1.f , 10.f };
		float				fVolume = 1.f;

	}AREASOUNDBOX_DESC;

protected:
	CAreaSoundBox(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CAreaSoundBox(const CAreaSoundBox& Prototype);
	virtual ~CAreaSoundBox() = default;

public:
	virtual HRESULT Initialize_Prototype() override;
	virtual HRESULT Initialize(void* pArg) override;
	virtual void Priority_Update(_float fTimeDelta) override;
	virtual void Update(_float fTimeDelta) override;
	virtual void Late_Update(_float fTimeDelta) override;
	virtual HRESULT Render() override;

public:
	void SoundPlay(const string szSoundID, _float fVolume = 1.f);
	void SoundVolume(const string szSoundID, _float fVolume);
	void SoundPosition(_float3 vPosition, _float fMin = 1.f, _float fMax = 10.f);
	void SoundStop();
	void SoundVolumeToZero();

private:
	HRESULT Ready_Components();
	void Update_SoundToZero(_float fTimeDelta);
private:
	const _tchar*		m_szSoundID = { nullptr };
	CSoundController*	m_pSoundCom = { nullptr };
	TRIGGERSOUND_TYPE	m_eTriggerSoundType;

	string m_strSoundName = {};

	_float m_fSoundVolume = {};
	_bool m_bSoundToZero = false;
public:
	static CAreaSoundBox* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg) override;
	virtual void Free() override;
};

NS_END