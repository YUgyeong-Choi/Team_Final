#pragma once

#include "Client_Defines.h"
#include "DynamicMesh.h"

NS_BEGIN(Engine)
class CModel;
class CShader;
class CAnimator;
class CSoundController;
NS_END

NS_BEGIN(Client)

class CBreakableMesh : public CGameObject
{
public:
	typedef struct tagBreakableMeshDesc : public CGameObject::GAMEOBJECT_DESC
	{
		//�� �޽�
		wstring ModelName = {};
		//��Ʈ �޽� ����
		_uint iPartModelCount = { 0 };
		//��Ʈ �޽� �̸���
		vector<wstring> PartModelNames = {};
		vector<_float3> vOffsets = {};

		//������ �� �׺�޽� �̸�
		wstring wsNavName = {};

		//Ǫ���� ���� ��ո� ��Ŀ������ �� �޶� �̷��� ó���ع����߰ڴ�. ���ο� Ŭ���� �ı� �ʹ� ���ŷο� ��
		_bool bFireEaterBossPipe = { false }
		;
	}BREAKABLEMESH_DESC;

protected:
	CBreakableMesh(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CBreakableMesh(const CBreakableMesh& Prototype);
	virtual ~CBreakableMesh() = default;

public:
	virtual HRESULT Initialize_Prototype() override;
	virtual HRESULT Initialize(void* pArg) override;
	virtual void Priority_Update(_float fTimeDelta) override;
	virtual void Update(_float fTimeDelta) override;
	virtual void Late_Update(_float fTimeDelta) override;
	virtual HRESULT Render() override;
	virtual void Reset() override;

public:
	virtual void On_CollisionEnter(CGameObject* pOther, COLLIDERTYPE eColliderType, _vector HitPos, _vector HitNormal) override;
public:
	virtual void On_TriggerEnter(CGameObject* pOther, COLLIDERTYPE eColliderType) override;
	//virtual void On_TriggerStay(CGameObject* pOther, COLLIDERTYPE eColliderType) {}
	//virtual void On_TriggerExit(CGameObject* pOther, COLLIDERTYPE eColliderType) {}
private:
	void Break();

private:
	HRESULT Render_Model();
	HRESULT Render_PartModels();

	HRESULT Find_Player();
	void IgnorePlayerCollider(CPhysXDynamicActor* pActor);

private:
	void Invisible();


private:
	//�� ���� xz ���󿡼� AABB�� ���ԵǾ��ִ���
	bool IsPointInsideXZ(const PxVec3& point, const PxBounds3& bounds);

	//�ڽ��� aabb ��ŭ�� �׺� �ε����� �����´�.
	void Store_NavIndices();

	void Set_Active_StoreCells(_bool bActive);

private:
	//�� �̸�
	wstring m_wsModelName = {};

private:
	//�ڽ��� ������ �ִ� �׺� �� �ε�����, �ڽ��� �μ����� Ȱ��ȭ ��Ų��.
	vector<_int> m_NavIndices = {};

private:
	//������ �ʱ���� ����
	vector<_float4x4> m_PartInitWorldMatrixs = {};

private:
	//��Ʈ �� ����
	_uint m_iPartModelCount = 0;

private:
	_bool m_bBreakTriggered = { false }; //�������ٴ� Ʈ����
	_bool m_bIsBroken = { false }; //�̹� ������ �������� Ȯ���ϴ� �ڵ�

private:
	//���ʵ� ��������, �ݶ��̴��� ������
	const _float m_fTime_Invisible = { 8.f };
	_float m_fTimeAcc = { 0.f };
	_bool m_bInvisible = { false };

private:
	class CPlayer* m_pPlayer = { nullptr };

private:
	//Ǫ���� ���� ��ո� ��Ŀ������ �� �޶� �̷��� ó���ع����߰ڴ�. ���ο� Ŭ���� �ı� �ʹ� ���ŷο� ��
	_bool m_bFireEaterBossPipe = { false };

private:    

	/* [ ������Ʈ ] */
	vector<CModel*> m_pPartModelComs = {};
	vector<CPhysXDynamicActor*> m_pPartPhysXActorComs = {};
	vector<CTransform*> m_pPartTransformComs = {};

	CModel* m_pModelCom = { nullptr };
	CShader* m_pShaderCom = { nullptr };

	//������ �� �׺���̼�
	CNavigation* m_pNaviCom = { nullptr };

	//����
	CSoundController* m_pSoundCom = { nullptr };

protected:
	HRESULT Ready_Components(void* pArg);
	HRESULT Bind_ShaderResources();
	HRESULT Ready_Collider();
	HRESULT Ready_PartColliders();

public:
	static CBreakableMesh* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg) override;
	virtual void Free() override;
};

NS_END