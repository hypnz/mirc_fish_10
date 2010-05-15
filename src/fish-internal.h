#include "targetver.h"
#include <windows.h>
#include <string>

class CBlowIni
{
protected:
	std::wstring m_iniPath;
	std::string m_iniBlowKey;

public:
	CBlowIni(std::wstring a_iniPath = L"");
	void SetIniPath(std::wstring a_iniPath) { m_iniPath = a_iniPath; }

	/* general purpose INI tools */
	std::wstring GetStringW(const wchar_t* a_key, const wchar_t* a_default = NULL);
	std::string GetString(const wchar_t* a_key, const wchar_t* a_default = NULL);
	bool GetBool(const wchar_t* a_key, bool a_default);
	int GetInt(const wchar_t* a_key, int a_default = 0);

	/* blow.ini-specific methods */
	static std::string FixContactName(const std::string& a_name);
	std::string GetBlowKey(const std::string& a_name);
	bool DeleteBlowKey(const std::string& a_name);
	bool WriteBlowKey(const std::string& a_name, const std::string& a_value);
	bool GetSectionBool(const std::string& a_name, const wchar_t* a_key, bool a_default);
};

/* from util.cpp */
std::string UnicodeToCp(UINT a_codePage, const std::wstring& a_wstr);
std::wstring UnicodeFromCp(UINT a_codePage, const std::string& a_str);
void StrTrimRight(std::string& a_str);
std::string HttpDownloadTextFile(const std::wstring& a_url);
std::string Base64_Encode(const std::string& a_input);
std::string Base64_Decode(const std::string& a_input);

/* from blowfish.cpp */
void blowfish_encrypt(const std::string& ain, std::string &out, const std::string &key);
int blowfish_decrypt(const std::string& ain, std::string &out, const std::string &key);

/* from dh1080.cpp */
bool DH1080_Generate(std::string& ar_priv, std::string& ar_pub);
std::string DH1080_Compute(const std::string& a_priv, const std::string& a_pub);

/* for fish-main.cpp */
#define EXPORT_SIG(RET_TYPE) extern "C" RET_TYPE __stdcall