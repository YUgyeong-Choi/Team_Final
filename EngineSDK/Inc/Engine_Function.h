#pragma once

namespace Engine
{
	template<typename T>
	void Safe_Delete(T& Pointer)
	{
		if (nullptr != Pointer)
		{
			delete Pointer;
			Pointer = nullptr;
		}
	}

	template<typename T>
	void Safe_Delete_Array(T& Pointer)
	{
		if (nullptr != Pointer)
		{
			delete[] Pointer;
			Pointer = nullptr;
		}
	}

	template<typename T>
	unsigned int Safe_AddRef(T& Instance)
	{
		unsigned int iRefCnt = { 0 };

		if (nullptr != Instance)
			iRefCnt = Instance->AddRef();

		return iRefCnt;
	}

	template<typename T>
	unsigned int Safe_Release(T& Instance)
	{
		unsigned int iRefCnt = { 0 };

		if (nullptr != Instance)
		{
			iRefCnt = Instance->Release();
			if (0 == iRefCnt)
				Instance = nullptr;
		}
		return iRefCnt;
	}

	static string WStringToString(const _wstring& wstr)
	{
		if (wstr.empty()) return {};
		int sizeNeeded = WideCharToMultiByte(CP_ACP, 0, wstr.c_str(), (int)wstr.size(), nullptr, 0, nullptr, nullptr);
		string result(sizeNeeded, 0);
		WideCharToMultiByte(CP_ACP, 0, wstr.c_str(), (int)wstr.size(), &result[0], sizeNeeded, nullptr, nullptr);
		return result;
	}

	static wstring StringToWString(const string& str)
	{
		if (str.empty()) return {};

		int sizeNeeded = MultiByteToWideChar(CP_ACP, 0, str.c_str(), (int)str.size(), nullptr, 0);
		wstring result(sizeNeeded, 0);
		MultiByteToWideChar(CP_ACP, 0, str.c_str(), (int)str.size(), &result[0], sizeNeeded);
		return result;
	}

