#pragma once
#include "GameObject.h"

#include "Client_Defines.h"
#include "CameraSequence.h"
NS_BEGIN(Client)

class CYGTool final : public CGameObject
{
	struct CAMERA_KEYFRAME
	{
		_int keyFrame;

		XMFLOAT3 position;
		XMFLOAT3 rotation;   // Euler or quaternion
		_float fFov;

		INTERPOLATION_CAMERA interpPosition;
		INTERPOLATION_CAMERA interpRotation;
		INTERPOLATION_CAMERA interpFov;

		//_bool bLookAt = false;
		//_vector lookAtTarget;
		//INTERPOLATION_CAMERA interpLookAt;
	};
private:
	CYGTool(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CYGTool(const CYGTool& Prototype);
	virtual ~CYGTool() = default;

public:
	virtual HRESULT Initialize_Prototype();
	virtual HRESULT Initialize(void* pArg);
	virtual void Priority_Update(_float fTimeDelta);
	virtual void Update(_float fTimeDelta);
	virtual void Late_Update(_float fTimeDelta);
	virtual HRESULT Render();

private:
	HRESULT Render_CameraTool();
	HRESULT Render_CameraFrame();
	HRESULT Render_CameraSequence();
	void Render_SetInfos();

	CCameraSequence* m_CameraSequence;
	_int m_iCurrentFrame = 0;
	_bool m_bExpanded = true;
	_int m_iSelected = -1;
	_int m_iFirstFrame = 0;

	_int m_iEndFrame = {};

	vector<CAMERA_KEYFRAME> m_vecCameraKeyFrame;
	CCameraSequence::CAMERA_KEY* m_pSelectedKey = nullptr;

	_int m_iEditKey = -1;
	CAMERA_KEYFRAME* m_pEditKey = nullptr;
public:
	static CYGTool* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, void* pArg = nullptr);
	virtual CGameObject* Clone(void* pArg) override;
	virtual void Free() override;

};

NS_END