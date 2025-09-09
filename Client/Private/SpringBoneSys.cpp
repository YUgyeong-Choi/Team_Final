#include "SpringBoneSys.h"

#include "Bone.h"
#include "Model.h"
#include "Client_Calculation.h"

HRESULT CSpringBoneSys::InitializeSpringBones(CModel* pModel, const vector<string>& vecSpringBoneNames, const SpringInitParams& initParam)
{
	if (!pModel || vecSpringBoneNames.empty())
		return E_FAIL;

	m_pModelCom = pModel;
	m_SpringBoneNames = vecSpringBoneNames;
	m_InitParams = initParam;
	const auto& pBones = m_pModelCom->Get_Bones();

	for (const auto& child : pBones)
	{
		string boneName = child->Get_Name();

		if (IsCorrectBoneName(boneName, m_SpringBoneNames) == false)
			continue;

		_int iParentIdx = child->Get_ParentBoneIndex();
		if (iParentIdx < 0)
			continue;
		auto* pParent = m_pModelCom->Get_Bones()[iParentIdx];
		SpringBone vSpringBone{};
		vSpringBone.pBone = child;
		vSpringBone.pParent = pParent;
		vSpringBone.parentIdx = iParentIdx;
		vSpringBone.childIdx = child->Get_BoneIndex();

		_matrix childL = XMLoadFloat4x4(child->Get_TransformationMatrix());
		vSpringBone.restLocalPos = childL.r[3];
		vSpringBone.length = XMVectorGetX(XMVector3Length(vSpringBone.restLocalPos));
		vSpringBone.restDirLocal = XMVector3Normalize(vSpringBone.restLocalPos);
		vSpringBone.part = SetBonePart(boneName);

		_vector correctedRestDir = vSpringBone.restDirLocal;

		if (m_InitParams.restDirBiasEnable) // 재조정할건지
		{
			_float fY = XMVectorGetY(vSpringBone.restDirLocal);
			_vector idealDir = XMVectorSet(0.f, -1.f, 0., 0.f);
			idealDir = XMVector3Normalize(idealDir);

			// 현재 방향과 이상적 방향의 차이 확인
			_float fAngleFromIdeal = acosf(clamp(XMVectorGetX(XMVector3Dot(vSpringBone.restDirLocal, idealDir)), -1.f, 1.f));
			_float fMaxAngle = XMConvertToRadians(m_InitParams.restDirMaxAngleDeg);

			if (fAngleFromIdeal > fMaxAngle || fY > 0.1f) // Y가 양수면 위쪽을 향함
			{
				_float fBlendFactor = m_InitParams.restDirBlend; //60%는 내가 쓸 방향, 30% 원래 방향
				correctedRestDir = XMVector3Normalize(
					vSpringBone.restDirLocal * (1.f - fBlendFactor) + idealDir * fBlendFactor
				);
			}
		}

		vSpringBone.restDirLocal = correctedRestDir;

		vSpringBone.restUpLocal = XMVector3Normalize(childL.r[1]);
		vSpringBone.restRotQ = XMQuaternionRotationMatrix(childL);

		// 부위별 처짐 정도 설정
		_float fDownBiasAmount = m_InitParams.downBiasOther;
		_vector gravityDir = XMVectorSet(0.f, -1.f, 0.f, 0.f);

		if (vSpringBone.part == CSpringBoneSys::SpringBonePart::Front)
		{
			fDownBiasAmount = m_InitParams.downBiasFront;
		}
		else if (vSpringBone.part == CSpringBoneSys::SpringBonePart::Back)
		{
			fDownBiasAmount = m_InitParams.downBiasBack;
		}

		_vector restBiasDir = XMVector3Normalize(
			vSpringBone.restDirLocal * (1.f - fDownBiasAmount) + gravityDir * fDownBiasAmount
		);

		vSpringBone.curTipLocal = restBiasDir * vSpringBone.length;
		vSpringBone.prevTipLocal = vSpringBone.curTipLocal;

		m_SpringBones.push_back(vSpringBone);
	}
	Build_SpringBoneHierarchy();

	SetupSpringBoneParameters();

	return S_OK;
}

void CSpringBoneSys::Update(_float fTimeDelta)
{
	constexpr _float fTargetTimeDelta = 1.f / 60.f; // 시간은 너무 튀거나 이상해지지 않게 고정값으로

	_float fTimeScale = fTimeDelta / fTargetTimeDelta;

	for (_int layer = 0; layer < static_cast<_int>(m_SBLayers.size()); layer++)
	{
		for (_int i : m_SBLayers[layer])
		{
			auto& vSpringBone = m_SpringBones[i];
			auto& vParam = m_SpringBones[i].param;
			_int iParentIdx = m_SBParentIdx[i];

			// 부모나 본이 없으면 패스
			if (!vSpringBone.pBone || !vSpringBone.pParent)
				continue;

			_vector S, Rq, T;
			_matrix parentC;

			if (iParentIdx >= 0)
			{
				parentC = XMLoadFloat4x4(m_SpringBones[iParentIdx].pBone->Get_CombinedTransformationMatrix());
			}
			else
			{
				parentC = XMLoadFloat4x4(vSpringBone.pParent->Get_CombinedTransformationMatrix());
			}


			XMMatrixDecompose(&S, &Rq, &T, parentC);
			Rq = XMQuaternionNormalize(Rq);
			_matrix parentR = XMMatrixRotationQuaternion(Rq);
			_matrix parentRInv = XMMatrixTranspose(parentR);

			// 부모 회전 변화에 따른 관성 처리
			_matrix toCurr = parentRInv * vSpringBone.parentPrevRotC;
			_vector qToCurr = XMQuaternionRotationMatrix(toCurr);
			_vector qBlend = XMQuaternionSlerp(XMQuaternionIdentity(), qToCurr, vParam.follow); // 부모를 따라가는 정도로 블렌드
			_matrix toCurrSoft = XMMatrixRotationQuaternion(qBlend);

			_vector oldTipLocal = vSpringBone.curTipLocal;

			// Verlet 적분 (관성)
			_vector velocity = (vSpringBone.curTipLocal - vSpringBone.prevTipLocal) * vParam.damping;
			vSpringBone.curTipLocal = vSpringBone.curTipLocal + velocity;
			vSpringBone.curTipLocal = XMVector3TransformNormal(vSpringBone.curTipLocal, toCurrSoft);

			// 중력 처리
			_vector gDir = XMVectorSet(0.f, -1.f, 0.f, 0.f);
			_vector gGravityDir = XMVector3TransformNormal(gDir, parentRInv);

			_vector restBiasDir = XMVector3Normalize(
				vSpringBone.restDirLocal * (1.f - vParam.downBias) + gGravityDir * vParam.downBias);

			// 중력
			_vector gravityForce = gGravityDir * (vParam.gravity * vParam.gScale * fTargetTimeDelta * fTargetTimeDelta * fTimeScale);
			vSpringBone.curTipLocal += gravityForce;


			// 복원력
			_vector targetPos = restBiasDir * vSpringBone.length;
			_vector restoreForce = (targetPos - vSpringBone.curTipLocal) * vParam.stiffness;
			vSpringBone.curTipLocal += restoreForce;


			// 길이 
			_vector dirL = XMVector3Normalize(vSpringBone.curTipLocal);
			vSpringBone.curTipLocal = dirL * vSpringBone.length;

			//  콘 제한
			_float fMaxRad = XMConvertToRadians(vParam.maxDeg);
			_float fAng = acosf(std::clamp(XMVectorGetX(XMVector3Dot(restBiasDir, dirL)), -1.f, 1.f));

			if (fAng > fMaxRad)
			{
				_float falloffStart = fMaxRad * 0.8f; // 너무 딱 잘라서 각도 제한 안하게 80프로부터

				if (fAng > falloffStart)
				{
					_float fExcess = fAng - falloffStart;
					_float fMaxExcess = fMaxRad - falloffStart;
					_float t = fExcess / fMaxExcess;
					_float fEasedT = 1.f - powf(1.f - t, 3.f); // Cubic ease-out
					_float targetAngle = falloffStart + fMaxExcess * fEasedT;

					_vector vAxis = XMVector3Cross(restBiasDir, dirL);
					_float fAxisLen = XMVectorGetX(XMVector3Length(vAxis));

					if (fAxisLen > 0.001f) // 유요한 길이인지
					{
						vAxis = XMVector3Normalize(vAxis);
						_matrix rot = XMMatrixRotationAxis(vAxis, targetAngle);
						dirL = XMVector3TransformNormal(restBiasDir, rot);
						vSpringBone.curTipLocal = dirL * vSpringBone.length;
					}
				}
			}

			// 회전 계산
			_vector dq = FromToQ(vSpringBone.restDirLocal, dirL);
			_vector newRotQ = XMQuaternionMultiply(dq, vSpringBone.restRotQ);
			newRotQ = XMQuaternionNormalize(newRotQ);

			_matrix R = XMMatrixRotationQuaternion(newRotQ);

			// 최종 변환 행렬
			_matrix L;
			L.r[0] = R.r[0];
			L.r[1] = R.r[1];
			L.r[2] = R.r[2];
			L.r[3] = XMVectorSetW(vSpringBone.restLocalPos, 1.f);

			vSpringBone.pBone->Set_TransformationMatrix(L);

			// 상태 업데이트
			vSpringBone.prevTipLocal = oldTipLocal;
			vSpringBone.parentPrevRotC = parentR;

		}
	}
	m_pModelCom->Update_Bones();
}

void CSpringBoneSys::Build_SpringBoneHierarchy()
{
	_int iMaxIdx = 0;
	for (auto& sb : m_SpringBones)
	{
		iMaxIdx = max(iMaxIdx, sb.childIdx);
		iMaxIdx = max(iMaxIdx, sb.parentIdx);
	}

	vector<_int> boneIdxToSpringIdx(iMaxIdx + 1, -1);
	for (_int i = 0; i < static_cast<_int>(m_SpringBones.size()); i++)
		boneIdxToSpringIdx[m_SpringBones[i].childIdx] = i;

	m_SBChildren.assign(m_SpringBones.size(), {});
	m_SBParentIdx.assign(m_SpringBones.size(), -1);
	m_SBRoots.clear();

	for (_int i = 0; i < static_cast<_int>(m_SpringBones.size()); i++)
	{
		_int iParentBoneIdx = m_SpringBones[i].parentIdx;
		_int pSpr = (iParentBoneIdx >= 0 ? boneIdxToSpringIdx[iParentBoneIdx] : -1);
		if (pSpr < 0)
			m_SBRoots.emplace_back(i);
		else
		{
			m_SBParentIdx[i] = pSpr;
			m_SBChildren[pSpr].emplace_back(i);
		}
	}

	// depth 채우기, 레이어 구성
	vector<_int> depth(m_SpringBones.size(), -1);
	queue<_int> q;
	for (_int r : m_SBRoots)
	{
		depth[r] = 0;
		q.push(r);
	}

	_int iMaxDepth = 0;
	while (!q.empty())
	{
		_int u = q.front();
		q.pop();
		for (_int v : m_SBChildren[u]) // 현재 뼈의 자식들
		{
			if (depth[v] != -1)
				continue;
			depth[v] = depth[u] + 1;
			if (depth[v] > iMaxDepth) // 깊이 갱신
				iMaxDepth = depth[v];
			q.push(v);
		}
	}

	m_SBLayers.assign(iMaxDepth + 1, {});
	for (_int i = 0; i < static_cast<_int>(m_SpringBones.size()); i++)
	{
		m_SpringBones[i].depth = depth[i];
		m_SBLayers[depth[i]].emplace_back(i);
	}
}

void CSpringBoneSys::SetupSpringBoneParameters()
{
	CHAR exeFullPath[MAX_PATH] = {};
	GetModuleFileNameA(nullptr, exeFullPath, MAX_PATH);
	filesystem::path exeDir = filesystem::path(exeFullPath).parent_path();
	auto fileName = m_pModelCom->Get_ModelName() + "_SpringBoneProfile.json";
	auto loadDir = exeDir / "Save" / "SpringBoneProfiles" / fileName;

	if (!filesystem::exists(loadDir))
	{
		MSG_BOX("SpringBone profile file not found!");
		return;
	}

	// JSON 읽기
	ifstream inFile(loadDir);
	if (!inFile.is_open())
	{
		MSG_BOX("Failed to open SpringBone profile file!");
		return;
	}

	json j;
	inFile >> j;
	inFile.close();

	Deserialize(j);

	for (_int layer = 0; layer < static_cast<_int>(m_SBLayers.size()); layer++)
	{
		for (int i : m_SBLayers[layer])
		{
			auto& sb = m_SpringBones[i];
			SpringBoneProfile profile = m_Profiles[ReturnPartString(sb.part)];

			_float t = (sb.chainLen > 1) ? static_cast<_float>(sb.depth) / (sb.chainLen - 1) : 0.f;
			_float t2 = powf(t, profile.fExp);
			auto stiffnessRange = profile.stiffnessRange;
			auto gScaleRange = profile.gScaleRange;
			auto downBiasRange = profile.downBiasRange;
			auto followRange = profile.followRange;
			auto maxDegRange = profile.maxDegRange;
			auto dampingRange = profile.dampingRange;

			sb.param.stiffness = LerpFloat(stiffnessRange.first, stiffnessRange.second, t2);
			sb.param.gScale = LerpFloat(gScaleRange.first, gScaleRange.second, t2);
			sb.param.downBias = LerpFloat(downBiasRange.first, downBiasRange.second, t2);
			sb.param.follow = LerpFloat(followRange.first, followRange.second, t2);
			sb.param.maxDeg = LerpFloat(maxDegRange.first, maxDegRange.second, t2);
			sb.param.damping = LerpFloat(dampingRange.first, dampingRange.second, t2);
			sb.param.maxDeg = min(sb.param.maxDeg, 178.f);

			_matrix parentLocal = XMMatrixIdentity();
			if (sb.pParent)
			{
				_vector S, Rq, T;
				parentLocal = XMLoadFloat4x4(sb.pParent->Get_CombinedTransformationMatrix());
				XMMatrixDecompose(&S, &Rq, &T, parentLocal);
				Rq = XMQuaternionNormalize(Rq);
				parentLocal = XMMatrixRotationQuaternion(Rq); // 순수 회전만
			}
			sb.parentPrevRotC = parentLocal;
		}
	}
}

_bool CSpringBoneSys::IsCorrectBoneName(const string& boneName, const vector<string>& vecSpringBoneNames)
{
	for (const auto& name : vecSpringBoneNames)
	{
		if (boneName.find(name) != string::npos)
			return true;
	}
	return false;
}

CSpringBoneSys::SpringBonePart CSpringBoneSys::SetBonePart(const string& boneName)
{
	if((boneName.find("FL_") != string::npos || boneName.find("FR_") != string::npos))
		return SpringBonePart::Front;
	else if (boneName.find("BL_") != string::npos || boneName.find("BR_") != string::npos)
		return SpringBonePart::Back;
	else if (boneName.find("Cloth") != string::npos)
		return SpringBonePart::Cloth;
	else
		return SpringBonePart::Other;
}

json CSpringBoneSys::Serialize()
{
	return json();
}

void CSpringBoneSys::Deserialize(const json& j)
{
	_float exp = 1.35f;
	pair<_float, _float> follow{ 0.1f, 0.5f };
	pair<_float, _float> maxDeg{ 90.f, 140.f };
	pair<_float, _float> damping{ 0.7f, 1.f };

	if (j.contains("exp"))
		exp = j["exp"].get<_float>();

	if (j.contains("follow") && j["follow"].is_array() && j["follow"].size() == 2)
		follow = { j["follow"][0].get<_float>(), j["follow"][1].get<_float>() };

	if (j.contains("maxDeg") && j["maxDeg"].is_array() && j["maxDeg"].size() == 2)
		maxDeg = { j["maxDeg"][0].get<_float>(), j["maxDeg"][1].get<_float>() };

	if (j.contains("damping") && j["damping"].is_array() && j["damping"].size() == 2)
		damping = { j["damping"][0].get<_float>(), j["damping"][1].get<_float>() };

	// parts 처리
	if (j.contains("parts"))
	{
		for (auto& [key, val] : j["parts"].items()) // "Front", "Back", "Other"
		{
			SpringBoneProfile profile;

			profile.fExp = exp;
			profile.followRange = follow;
			profile.maxDegRange = maxDeg;
			profile.dampingRange = damping;

			if (val.contains("stiffness") && val["stiffness"].is_array() && val["stiffness"].size() == 2)
				profile.stiffnessRange = { val["stiffness"][0].get<_float>(), val["stiffness"][1].get<_float>() };

			if (val.contains("gScale") && val["gScale"].is_array() && val["gScale"].size() == 2)
				profile.gScaleRange = { val["gScale"][0].get<_float>(), val["gScale"][1].get<_float>() };

			if (val.contains("downBias") && val["downBias"].is_array() && val["downBias"].size() == 2)
				profile.downBiasRange = { val["downBias"][0].get<_float>(), val["downBias"][1].get<_float>() };

			m_Profiles[key] = profile;
		}
	}
}

string CSpringBoneSys::ReturnPartString(SpringBonePart part)
{
	switch (part)
	{
	case SpringBonePart::Front:
		return "Front";
	case SpringBonePart::Back:
		return "Back";
	case SpringBonePart::Other:
		return "Other";
	case SpringBonePart::Cloth: 
		return "Cloth";
	default:
		return "Other";
	}
}

CSpringBoneSys* CSpringBoneSys::Create(CModel* pModel, const vector<string>& vecSpringBoneNames, const SpringInitParams& initParam)
{
	CSpringBoneSys* pInstance = new CSpringBoneSys();
	if (FAILED(pInstance->InitializeSpringBones(pModel, vecSpringBoneNames, initParam)))
	{
		MSG_BOX("Failed to create CSpringBoneSys");
		Safe_Release(pInstance);
	}
	return pInstance;
}

void CSpringBoneSys::Free()
{
	__super::Free();
}


