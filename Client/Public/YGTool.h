#pragma once
#include "GameObject.h"

#include "Client_Defines.h"
#include "CameraSequence.h"
NS_BEGIN(Client)

class CYGTool final : public CGameObject
{
	struct CAMERA_POSFRAME
	{
		_int keyFrame;
		XMFLOAT3 position;
		INTERPOLATION_CAMERA interpPosition;
	};
	struct CAMERA_ROTFRAME
	{
		_int keyFrame;
		XMFLOAT3 rotation;  
		INTERPOLATION_CAMERA interpRotation;
	};
	struct CAMERA_FOVFRAME
	{
		_int keyFrame;
		_float fFov;
		INTERPOLATION_CAMERA interpFov;
	};

	struct CAMERA_FRAMEDATA
	{
		vector<CAMERA_POSFRAME> vecPosData;
		vector<CAMERA_ROTFRAME> vecRotData;
		vector<CAMERA_FOVFRAME> vecFovData;
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

	CAMERA_FRAMEDATA m_CameraDatas;
	CCameraSequence::CAMERA_KEY* m_pSelectedKey = nullptr;

	_int m_iEditKey = -1;
	CAMERA_POSFRAME* m_pEditPosKey = nullptr;
	CAMERA_ROTFRAME* m_pEditRotKey = nullptr;
	CAMERA_FOVFRAME* m_pEditFovKey = nullptr;
public:
	static CYGTool* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, void* pArg = nullptr);
	virtual CGameObject* Clone(void* pArg) override;
	virtual void Free() override;

};

NS_END