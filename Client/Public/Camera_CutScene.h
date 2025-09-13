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
	// ����Ż �ʱ� ��ġ : �÷��̾� �� �ڿ� ��ġ�ϸ� �÷��̾�� ���� Look�� ���� ��ġ

	_int Get_CurrentFrame() { return m_iCurrentFrame; }

	/* [ �ƾ� - E������ ������ �� ] */
	void Set_CutSceneData(CUTSCENE_TYPE cutSceneType);
	/* [ �ƾ� - Tool���� ������ �� ] */
	void Set_CameraFrame(CUTSCENE_TYPE cutSceneType, const CAMERA_FRAMEDATA CameraFrameData, _int start, _int end);

	/* [ �ƾ� Ȱ��ȭ ] */
	void PlayCutScene() { m_bActive = true; CUI_Manager::Get_Instance()->Off_Panel(); }

	/* [ ����Ż �ʱ� ��ġ ] */
	void Set_InitOrbitalWorldMatrix(_matrix initOrbitalPos) { m_initOrbitalMatrix = initOrbitalPos; }

	void Set_StartBlend(_bool bStartBlend) { m_bOrbitalToSetOrbital = bStartBlend; }
	void Set_EndBlend(_bool bEndBlend) { m_bReadyCutSceneOrbital = bEndBlend; }

	HRESULT InitDatas();
private:
	/* [ ī�޶� ��ġ, ȸ�� ���� ] */
	void Interp_WorldMatrixOnly(_int curFrame);
	/* [ ī�޶� Fov ���� ] */
	void Interp_Fov(_int curFrame);
	/* [ ī�޶� OffsetRot ���� ] */
	void Interp_OffsetRot(_int curFrame);
	/* [ ī�޶� OffsetPos ���� ] */
	void Interp_OffsetPos(_int curFrame);
	/* [ ī�޶� Target ���� ] */
	void Interp_Target(_int curFrame);

	/* [ ��ġ���� �����ϴ� �Լ� - �����Ҷ� ���� ���Ƽ� �߰��ص� �� ] */
	XMVECTOR XMMatrixDecompose_T(const _matrix& m);

	/* [ ����Ż ī�޶� (�)��ġ ->  ����Ż �ʱ� ��ġ (����Ż ī�޶��� Pitch�� Yaw�� ����)] */
	_bool ReadyToOrbitalWorldMatrix(_float fTimeDelta);

	/* [ ����Ż �ʱ� ��ġ ->  �ƾ� �ʱ� ��ġ] || [ �ƾ� �� ��ġ ->  ����Ż �ʱ� ��ġ] */
	_bool Camera_Blending(_float fTimeDelta, _matrix targetMat, _matrix currentMat);

	/* [ �ƾ� ������ �ε� ] */
	CAMERA_FRAMEDATA LoadCameraFrameData(const json& j);

	/* [ �̺�Ʈ ] */
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
	_float m_fFrameSpeed = 60.f; // 1�ʿ� 60������ ����

	unordered_map<CUTSCENE_TYPE, CAMERA_FRAMEDATA> m_CutSceneDatas;

	_matrix m_initOrbitalMatrix = {};
	_bool m_bOrbitalToSetOrbital = false;
	_bool m_bReadyCutScene = false;
	_bool m_bReadyCutSceneOrbital = false;

	_bool m_bStopCamera = false;

	// Tool��
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

