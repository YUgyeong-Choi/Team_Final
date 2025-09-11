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
		_bool  restDirBiasEnable = false;   // 보정 사용 여부
		_float restDirMaxAngleDeg = 70.f;   // 이상적인 -Y와의 최대 허용 각(초과 시 보정)
		_float restDirBlend = 0.6f;   // 보정 블렌딩 비율(0~1), 0.6=60% 이상적 방향

		_float downBiasFront = 0.5f;   // (Front)
		_float downBiasBack = 0.3f;   // (Back)
		_float downBiasOther = 0.4f;   // (Other)
	};
private:
	enum class SpringBonePart // 머리카락 앞,뒤, 다른 부위, 옷
	{
		Front, Back, Other, Cloth, Item
	};
	struct SpringBoneParm
	{
		_float follow = 0.7f;
		_float stiffness = 0.015f;
		_float maxDeg = 160.f;
		_float gravity = 20.f; //과하게 주기
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
		// 레스트 로컬 기준들
		_float3 restLocalPos = { 0.f,0.f,0.f };     // 로컬 위치
		_float3 restDirLocal = { 0.f,0.f,0.f };     // 로컬 방향
		_float3 restUpLocal = { 0.f,0.f,0.f };      // 로컬 업 벡터
		_float4 restRotQ = { 0.f,0.f,0.f,1.f };         // child 로컬의 레스트 회전(quat)
		_float  length{};         // 부모랑의 길이
		// 부모 로컬에서 끝점 위치를 적분
		_float3 curTipLocal = { 0.f,0.f,0.f };
		_float3 prevTipLocal = { 0.f,0.f,0.f };
		// 부모의 직전 로컬 회전
		_float4x4  parentPrevRotC = {};
		_float3 prevParentPos = { 0.f,0.f,0.f };    // 부모의 이전 프레임 위치
		// 파라미터 (강성, 최대 각도, 중력)
		_int   depth = 0;
		_int   chainLen = 0;
		SpringBoneParm param{};
		SpringBonePart part = SpringBonePart::Other;
	};

	struct SpringBoneProfile
	{
		_float fExp = 1.35f; // 깊이 기반 보간 지수
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
	_bool IsCorrectBoneName(CBone* pBone, const vector<string>& vecSpringBoneNames);
	SpringBonePart SetBonePart(const string& boneName);
	string ReturnPartString(SpringBonePart part);

	json Serialize() override;
	void Deserialize(const json& j) override;
private:
	vector<SpringBone> m_SpringBones;
	vector<_int>           m_SBRoots;        // 루트 인덱스들
	vector<vector<_int>>   m_SBChildren;     // 뼈들의 자식 인덱스들
	vector<_int>           m_SBParentIdx;    // 각 노드의 부모 스프링본 인덱스
	vector<vector<_int>>   m_SBLayers;       // depth별 노드 리스트(순회용)

	CModel* m_pModelCom = nullptr;
	vector<string> m_SpringBoneNames;
	SpringInitParams m_InitParams;
	unordered_map<string, SpringBoneProfile> m_Profiles;
public:
	static CSpringBoneSys* Create(CModel* pModel,const vector<string>& vecSpringBoneNames, const SpringInitParams& initParam);
	virtual void Free() override;
};
NS_END

