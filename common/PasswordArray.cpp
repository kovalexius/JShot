#include "PasswordArray.h"

#ifdef _WIN32

CPasswordArray::CPasswordArray(void)
{
}

CPasswordArray::~CPasswordArray(void)
{
	for (size_t i = 0; i < m_vecPasswords.size(); i++) delete m_vecPasswords[i];
}

void CPasswordArray::AddPassword(const TCHAR* szLogin, const TCHAR* szPassword /*= NULL*/)
{
	AddPassword(CPasswordKeeper(szLogin, szPassword));
}

void CPasswordArray::AddPassword(const CPasswordKeeper& rPwd)
{
	CPasswordKeeper* pPwd = new CPasswordKeeper(rPwd);
	m_vecPasswords.push_back(pPwd);
}

const CPasswordKeeper* CPasswordArray::GetByIndex(size_t nIndex) const
{
	return (nIndex < m_vecPasswords.size()) ? m_vecPasswords[nIndex] : NULL;
}

bool CPasswordArray::SaveToRegistry(const TCHAR* szCryptKey, const CRegistry& rReg, const TCHAR* szSection, const TCHAR* szKeyName)
{
	bool bResult = false;
	// ��������� ������
	size_t nBufSize = 0;
	for (size_t i = 0; i < m_vecPasswords.size(); i++)
	{
		const CPasswordKeeper& rPwd = *m_vecPasswords[i];
		nBufSize += rPwd.GetLoginSize() + rPwd.GetPasswordSize() + sizeof(size_t) * 2;
	}
	if (nBufSize)
	{// ���������� � ���� PasswordKeeper ��� ������ � ������ �� ���������
		
		CPasswordKeeper rPwdArr;
		rPwdArr.m_sPassword.m_pData = new unsigned char[nBufSize];
		rPwdArr.m_sPassword.m_nSize = nBufSize;
		unsigned char* pCurPtr = static_cast<unsigned char*>(rPwdArr.m_sPassword.m_pData);
		for (size_t i = 0; i < m_vecPasswords.size(); i++)
		{	// ����� ��������� ����� � ������
			CPasswordKeeper rPwd(*m_vecPasswords[i]);
			for (int j = 0; j < 2; j++)
			{	// ���������� ������ ������, �����, ������ ������, ������ � �����
				void* pData = j ? rPwd.m_sPassword.m_pData : rPwd.m_sLogin.m_pData;
				size_t nSize = j ? rPwd.m_sPassword.m_nSize : rPwd.m_sLogin.m_nSize;

				rPwd.Decrypt(pData, nSize);
				memcpy(pCurPtr, &nSize, sizeof(nSize));
				pCurPtr += sizeof(nSize);
				memcpy(pCurPtr, pData, nSize);
				pCurPtr += nSize;
			}
		}
		// ��������� ������ ���� ����������, �.�. �� ����� ��� ������ � ������ ����� 
		// �������������� ������������
		rPwdArr.m_sKey.m_pData = NULL;
		rPwdArr.m_sKey.m_nSize = 0;
		// ��������� ��������� ����� � ������
		rPwdArr.SaveToRegistry(szCryptKey, rReg, szSection, szKeyName);
		bResult = true;
	}
	return bResult;
}

bool CPasswordArray::LoadFromRegistry(const TCHAR* szCryptKey, const CRegistry& rReg, const TCHAR* szSection, const TCHAR* szKeyName)
{
	// ������� ��� ����� ����������� ������
	m_vecPasswords.clear();

	// ��������� ������������� ������ �� ����
	CPasswordKeeper rPwdArr;
	bool bResult = rPwdArr.LoadFromRegistry(szCryptKey, rReg, szSection, szKeyName);
	
	if (bResult)
	{	// ��������� ����������� ������ 
		rPwdArr.Decrypt(rPwdArr.m_sPassword.m_pData, rPwdArr.m_sPassword.m_nSize);
		// ������
		size_t nCurPos = 0;
		while (nCurPos < rPwdArr.m_sPassword.m_nSize)
		{
			TCHAR *szLogin = NULL, *szPassword = NULL;
			size_t nLoginSize, nPasswordSize;
			for (int i = 0; i < 2; i++)
			{
				TCHAR** ppData = i ? &szPassword : &szLogin;
				size_t* pnSize = i ? &nPasswordSize : &nLoginSize;
				// ��������� ��������� ������
				*pnSize = *(size_t*)(static_cast<TCHAR*>(rPwdArr.m_sPassword.m_pData) + nCurPos);
				if ((nCurPos += sizeof(*pnSize)) > rPwdArr.m_sPassword.m_nSize) break;
				if (*pnSize)
				{
					*ppData = new TCHAR[*pnSize];
					memcpy(*ppData, static_cast<TCHAR*>(rPwdArr.m_sPassword.m_pData) + nCurPos, *pnSize);
				}
			}

//			CPasswordKeeper rPwd(
		//unsigned char* pCurPtr = rPwdArr.m_sPassword.m_pData;
		}
	}
	return bResult;
}

#endif