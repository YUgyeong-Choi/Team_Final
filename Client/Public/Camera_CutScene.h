#pragma once

#include "Client_Defines.h"
#include "Camera.h"
#include "UI_Manager.h"


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

	// ======================================
	// 오비탈 초기 위치 : 플레이어 등 뒤에 위치하며 플레이어랑 같은 Look을 가진 위치

	_int Get_CurrentFrame() { return m_iCurrentFrame; }

	/* [ 컷씬 - E눌러서 실행할 때 ] */
	void Set_CutSceneData(CUTSCENE_TYPE cutSceneType);
	/* [ 컷씬 - Tool에서 실행할 때 ] */
	void Set_CameraFrame(CUTSCENE_TYPE cutSceneType, const CAMERA_FRAMEDATA CameraFrameData, _int start, _int end);

	/* [ 컷씬 활성화 ] */
	void PlayCutScene() { m_bActive = true; CUI_Manager::Get_Instance()->Off_Panel(); }

	/* [ 오비탈 초기 위치 ] */
	void Set_InitOrbitalWorldMatrix(_matrix initOrbitalPos) { m_initOrbitalMatrix = initOrbitalPos; }

	void Set_StartBlend(_bool bStartBlend) { m_bOrbitalToSetOrbital = bStartBlend; }
	void Set_EndBlend(_bool bEndBlend) { m_bReadyCutSceneOrbital = bEndBlend; }

	HRESULT InitDatas();
private:
	/* [ 카메라 위치, 회전 보간 ] */
	void Interp_WorldMatrixOnly(_int curFrame);
	/* [ 카메라 Fov 보간 ] */
	void Interp_Fov(_int curFrame);
	/* [ 카메라 OffsetRot 보간 ] */
	void Interp_OffsetRot(_int curFrame);
	/* [ 카메라 OffsetPos 보간 ] */
	void Interp_OffsetPos(_int curFrame);
	/* [ 카메라 Target 보간 ] */
	void Interp_Target(_int curFrame);

	/* [ 위치값만 추출하는 함수 - 보간할때 쓰기 좋아서 추가해둔 것 ] */
	XMVECTOR XMMatrixDecompose_T(const _matrix& m);

	/* [ 오비탈 카메라 (어떤)위치 ->  오비탈 초기 위치 (오비탈 카메라의 Pitch와 Yaw도 설정)] */
	_bool ReadyToOrbitalWorldMatrix(_float fTimeDelta);

	/* [ 오비탈 초기 위치 ->  컷씬 초기 위치] || [ 컷씬 끝 위치 ->  오비탈 초기 위치] */
	_bool Camera_Blending(_float fTimeDelta, _matrix targetMat, _matrix currentMat);

	/* [ 컷씬 데이터 로드 ] */
	CAMERA_FRAMEDATA LoadCameraFrameData(const json& j);

	/* [ 이벤트 ] */
	void Event();
public:
	void	Set_FOV(_float FOV) { m_fFov = FOV; }

private:
	_float			m_fMouseSensor = { };
	_float			m_fPanSpeed = { };

	CGameObject* m_pPlayer = { nullptr };

	CAMERA_FRAMEDATA m_CameraDatas;
	CUTSCENE_TYPE m_eCurrentCutScene;

	_bool m_bActive = false;
	_float m_fElapsedTime = 0.f;
	_int   m_iCurrentFrame = -1;
	_float m_fFrameSpeed = 60.f; // 1초에 60프레임 기준

	unordered_map<CUTSCENE_TYPE, CAMERA_FRAMEDATA> m_CutSceneDatas;

	_matrix m_initOrbitalMatrix = {};
	_bool m_bOrbitalToSetOrbital = false;
	_bool m_bReadyCutScene = false;
	_bool m_bReadyCutSceneOrbital = false;

	_bool m_bStopCamera = false;

	// Tool용
	_int m_iStaratFrame;
	_int m_iEndFrame;
	_bool m_bShowSpecial = false;

	_double m_Accumulate = 0.0;
public:
	static CCamera_CutScene* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg) override;
	virtual void Free() override;

};

NS_END

