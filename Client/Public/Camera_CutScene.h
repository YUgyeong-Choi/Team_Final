#pragma once

#include "Client_Defines.h"
#include "Camera.h"


NS_BEGIN(Engine)
class CGameObject;
NS_END


NS_BEGIN(Client)

class CCamera_CutScene final : public CCamera
{
public:
	typedef struct tagCameraCutSceneDesc : public CCamera::CAMERA_DESC
	{
		_float			fMouseSensor = {};
	}CAMERA_CutScene_DESC;

private:
	CCamera_CutScene(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CCamera_CutScene(const CCamera_CutScene& Prototype);
	virtual ~CCamera_CutScene() = default;

public:
	virtual HRESULT Initialize_Prototype() override;
	virtual HRESULT Initialize(void* pArg) override;
	virtual void Priority_Update(_float fTimeDelta) override;
	virtual void Update(_float fTimeDelta) override;
	virtual void Late_Update(_float fTimeDelta) override;
	virtual HRESULT Render() override;

	void Set_CameraFrame(const CAMERA_FRAMEDATA CameraFrameData)
	{
		m_CameraDatas = CameraFrameData;
	}

	_int Get_CurrentFrame() { return m_iCurrentFrame; }
	void Set_CutSceneData(CUTSCENE_TYPE cutSceneType);

	void PlayCutScene() { m_bActive = true; }
private:
	void Interp_WorldMatrixOnly(_int curFrame);
	void Interp_Fov(_int curFrame);
	void Interp_OffsetRot(_int curFrame);
	HRESULT InitDatas();
	CAMERA_FRAMEDATA LoadCameraFrameData(const json& j);
public:
	void	Set_FOV(_float FOV) { m_fFov = FOV; }

private:
	_float			m_fMouseSensor = { };
	_float			m_fPanSpeed = { };

	CGameObject* m_pPlayer = { nullptr };


	CAMERA_FRAMEDATA m_CameraDatas;
	_bool m_bActive = false;
	_float m_fElapsedTime = 0.f;
	_int   m_iCurrentFrame = 0;
	_float m_fFrameSpeed = 60.f; // 1초에 60프레임 기준

	unordered_map<CUTSCENE_TYPE, CAMERA_FRAMEDATA> m_CutSceneDatas;

	_bool m_bReadyCutScene = false;
public:
	static CCamera_CutScene* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg) override;
	virtual void Free() override;

};

NS_END

