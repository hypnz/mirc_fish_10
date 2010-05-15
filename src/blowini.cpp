#include "fish-internal.h"
#include <time.h>

#define INI_SECTION L"FiSH"


CBlowIni::CBlowIni(std::wstring a_iniPath)
{
	m_iniBlowKey = "blowinikey\0ADDITIONAL SPACE FOR CUSTOM BLOW.INI PASSWORD";
	// well, it's open source now, but hex editing the DLL might still
	// be easier in some cases.
}


std::wstring CBlowIni::GetStringW(const wchar_t* a_key, const wchar_t* a_default)
{
	wchar_t l_buf[4096] = {0};

	::GetPrivateProfileStringW(INI_SECTION, a_key, a_default, l_buf, 4095, m_iniPath.c_str());

	return l_buf;
}


std::string CBlowIni::GetString(const wchar_t* a_key, const wchar_t* a_default)
{
	return UnicodeToCp(CP_UTF8, GetStringW(a_key, a_default));
}


bool CBlowIni::GetBool(const wchar_t* a_key, bool a_default)
{
	const std::wstring l_val = GetStringW(a_key, (a_default ? L"1" : L"0"));

	return (l_val.empty() ? a_default : (l_val[0] != L'0' && l_val[0] != L'n' && l_val[0] != L'N'));
}


int CBlowIni::GetInt(const wchar_t* a_key, int a_default)
{
	return ::GetPrivateProfileIntW(INI_SECTION, a_key, a_default, m_iniPath.c_str());
}


std::string CBlowIni::FixContactName(const std::string& a_name)
{
	std::string l_result;

	l_result.reserve(a_name.size());

	for(size_t p = 0; p < a_name.size(); p++)
	{
		l_result += (a_name[p] == '[' || a_name[p] == ']' ? '~' : a_name[p]);
	}

	return l_result;
}


std::string CBlowIni::GetBlowKey(const std::string& a_name)
{
	const std::string l_iniSection = FixContactName(a_name);
	const std::string l_ansiFileName = UnicodeToCp(CP_ACP, m_iniPath);
	char l_buf[4096] = {0};

	// use ANSI to maintain backwards compatibility to old blow.ini / fish.dlls.
	::GetPrivateProfileStringA(l_iniSection.c_str(), "key", "", l_buf, 4095, l_ansiFileName.c_str());

	std::string l_blowKey;

	if(*l_buf == 0)
	{
		// fallback for INI filenames with non-ANSI characters:
		const std::wstring l_iniSectionW = UnicodeFromCp(CP_ACP, l_iniSection);
		l_blowKey = UnicodeToCp(CP_ACP, GetStringW(l_iniSectionW.c_str()));
	}
	else
	{
		l_blowKey = l_buf;
	}

	if(l_blowKey.find("+OK ") == 0)
	{
		// the blowfish_decrypt must not change in a way that makes it stop
		// making a copy of the first argument or this will blow up.
		if(blowfish_decrypt(l_blowKey.substr(4), l_blowKey, m_iniBlowKey) != 0)
		{
			l_blowKey = "";
		}
	}

	return l_blowKey;
}


bool CBlowIni::DeleteBlowKey(const std::string& a_name)
{
	const std::string l_iniSection = FixContactName(a_name);
	const std::string l_ansiFileName = UnicodeToCp(CP_ACP, m_iniPath);
	bool l_success = true;

	if(::WritePrivateProfileStringA(l_iniSection.c_str(), NULL, NULL, l_ansiFileName.c_str()) == 0)
	{
		// compatibility fallback, read above...
		const std::wstring l_iniSectionW = UnicodeFromCp(CP_ACP, l_iniSection);

		l_success = (::WritePrivateProfileStringW(l_iniSectionW.c_str(), NULL, NULL, m_iniPath.c_str()) != 0);
	}

	return l_success;
}


bool CBlowIni::WriteBlowKey(const std::string& a_name, const std::string& a_value)
{
	const std::string l_iniSection = FixContactName(a_name);
	const std::string l_ansiFileName = UnicodeToCp(CP_ACP, m_iniPath);
	const std::wstring l_iniSectionW = UnicodeFromCp(CP_ACP, l_iniSection);
	std::string l_keyValue;

	blowfish_encrypt(a_value, l_keyValue, m_iniBlowKey);
	l_keyValue.insert(0, "+OK ");

	bool l_success = true;

	if(::WritePrivateProfileStringA(l_iniSection.c_str(), "key", l_keyValue.c_str(), l_ansiFileName.c_str()) == 0)
	{
		const std::wstring l_keyValueW = UnicodeFromCp(CP_ACP, l_keyValue);

		l_success = (::WritePrivateProfileStringW(l_iniSectionW.c_str(), L"key", l_keyValueW.c_str(), m_iniPath.c_str()) != 0);
	}

	if(l_success)
	{
		wchar_t l_timeBuf[30] = {0};
		time_t l_time = time(NULL);
		struct tm l_today;
		_localtime64_s(&l_today, &l_time);
		if(wcsftime(l_timeBuf, 29, L"%d/%m/%Y", &l_today) > 0)
		{
			::WritePrivateProfileStringW(l_iniSectionW.c_str(), L"date", l_timeBuf, m_iniPath.c_str());
		}
	}

	return l_success;
}


bool CBlowIni::GetSectionBool(const std::string& a_name, const wchar_t* a_key, bool a_default)
{
	const std::string l_iniSection = FixContactName(a_name);
	const std::string l_iniKey = UnicodeToCp(CP_ACP, a_key);
	const std::string l_ansiFileName = UnicodeToCp(CP_ACP, m_iniPath);
	char l_buf[10] = {0};

	::GetPrivateProfileStringA(l_iniSection.c_str(), l_iniKey.c_str(),
		(a_default ? "1" : "0"), l_buf, 9, l_ansiFileName.c_str());

	if(*l_buf == 0)
	{
		wchar_t l_bufW[10] = {0};

		const std::wstring l_iniSectionW = UnicodeFromCp(CP_ACP, l_iniSection);
		const std::wstring l_iniKeyW = UnicodeFromCp(CP_ACP, l_iniKey);
		
		::GetPrivateProfileStringW(l_iniSectionW.c_str(), l_iniKeyW.c_str(),
			(a_default ? L"1" : L"0"), l_bufW, 9, m_iniPath.c_str());

		return (!*l_bufW ? a_default : (l_bufW[0] != L'0' && l_bufW[0] != L'n' && l_bufW[0] != L'N'));
	}
	else
	{
		return (l_buf[0] != L'0' && l_buf[0] != L'n' && l_buf[0] != L'N');
	}
}