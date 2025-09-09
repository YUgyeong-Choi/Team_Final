#pragma once
#include "Base.h"
#include "Bone.h"
#include "Client_Defines.h"


NS_BEGIN(Engine)
class CModel;
NS_END

NS_BEGIN(Client)
class CSpringBoneSys : public CBase, public Engine::ISerializable
{
public:
	struct SpringInitParams
	{
		_bool  restDirBiasEnable = false;   // ���� ��� ����
		_float restDirMaxAngleDeg = 70.f;   // �̻����� -Y���� �ִ� ��� ��(�ʰ� �� ����)
		_float restDirBlend = 0.6f;   // ���� ���� ����(0~1), 0.6=60% �̻��� ����

		_float downBiasFront = 0.5f;   // (Front)
		_float downBiasBack = 0.3f;   // (Back)
		_float downBiasOther = 0.4f;   // (Other)
	};
private:
	enum class SpringBonePart // �Ӹ�ī�� ��,��, �ٸ� ����, ��
	{
		Front,Back, Other, Cloth
	};
	struct SpringBoneParm
	{
		_float follow = 0.7f;
		_float stiffness = 0.015f;
		_float maxDeg = 160.f;
		_float gravity = 20.f; //���ϰ� �ֱ�
		_float gScale = 1.0f;
		_float downBias = 0.0f;
		_float damping = 0.1f;
	};

	struct SpringBone
	{
		CBone* pBone = nullptr;
		CBone* pParent = nullptr;
		_int  parentIdx = -1;
		_int  childIdx = -1;
		// ����Ʈ ���� ���ص�
		_vector restLocalPos = XMVectorZero();     // ���� ��ġ
		_vector restDirLocal = XMVectorZero();     // ���� ����
		_vector restUpLocal = XMVectorZero();      // ���� �� ����
		_vector restRotQ = XMVectorZero();         // child ������ ����Ʈ ȸ��(quat)
		_float  length{};         // �θ���� ����
		// �θ� ���ÿ��� ���� ��ġ�� ����
		_vector curTipLocal = XMVectorZero();
		_vector prevTipLocal = XMVectorZero();
		// �θ��� ���� ���� ȸ��
		_matrix parentPrevRotC = XMMatrixIdentity();
		// �Ķ���� (����, �ִ� ����, �߷�)
		_int   depth = 0;
		_int   chainLen = 0;
		SpringBoneParm param{};
		SpringBonePart part = SpringBonePart::Other;
	};

	struct SpringBoneProfile
	{
		_float fExp = 1.35f; // ���� ��� ���� ����
		pair<_float, _float> stiffnessRange = { 0.01f, 0.02f };
		pair<_float, _float> gScaleRange = { 1.5f, 2.3f };
		pair<_float, _float> downBiasRange = { 0.25f, 0.52f };
		pair<_float, _float> followRange = { 0.1f, 0.5f };
		pair<_float, _float> maxDegRange = { 90.f, 140.f };
		pair<_float, _float> dampingRange = { 0.7f, 1.f };
	};
private:
	CSpringBoneSys() = default;
	virtual ~CSpringBoneSys() = default;

public:
	void Update(_float fTimeDelta);
private:
	HRESULT InitializeSpringBones(CModel* pModel, const vector<string>& vecSpringBoneNames, const SpringInitParams& initParam);
	void Build_SpringBoneHierarchy();
	void SetupSpringBoneParameters();
	_bool IsCorrectBoneName(const string& boneName, const vector<string>& vecSpringBoneNames);

	SpringBonePart SetBonePart(const string& boneName);
	string ReturnPartString(SpringBonePart part);

	json Serialize() override;
	void Deserialize(const json& j) override;
private:
	vector<SpringBone> m_SpringBones;
	vector<_int>           m_SBRoots;        // ��Ʈ �ε�����
	vector<vector<_int>>   m_SBChildren;     // ���� ����Ʈ
	vector<_int>           m_SBParentIdx;    // �� ����� �θ� �������� �ε���
	vector<vector<_int>>   m_SBLayers;       // depth�� ��� ����Ʈ(��ȸ��)

	CModel* m_pModelCom = nullptr;
	vector<string> m_SpringBoneNames;
	SpringInitParams m_InitParams;
	unordered_map<string, SpringBoneProfile> m_Profiles;
public:
	static CSpringBoneSys* Create(CModel* pModel,const vector<string>& vecSpringBoneNames, const SpringInitParams& initParam);
	virtual void Free() override;
};
NS_END

