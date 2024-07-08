#include "CFileVersionInfo.h"
#include <tchar.h>

CFileVersionInfo::CFileVersionInfo()
{
    Reset();
}


CFileVersionInfo::~CFileVersionInfo()
{}

BOOL CFileVersionInfo::GetTranslationId(LPVOID lpData, UINT unBlockSize, WORD wLangId, DWORD &dwId, BOOL bPrimaryEnough/*= FALSE*/)
{
    LPWORD lpwData;
    for (lpwData = (LPWORD)lpData; (LPBYTE)lpwData < ((LPBYTE)lpData) + unBlockSize; lpwData += 2)
    {
        if (*lpwData == wLangId)
        {
            dwId = *((DWORD*)lpwData);
            return TRUE;
        }
    }

    if (!bPrimaryEnough)
        return FALSE;

    for (lpwData = (LPWORD)lpData; (LPBYTE)lpwData < ((LPBYTE)lpData) + unBlockSize; lpwData += 2)
    {
        if (((*lpwData) & 0x00FF) == (wLangId & 0x00FF))
        {
            dwId = *((DWORD*)lpwData);
            return TRUE;
        }
    }

    return FALSE;
}

stlString CFileVersionInfo::GetFileDescription() const
{
    return m_strFileDescription;
}

stlString CFileVersionInfo::GetFileVersion() const
{
    return m_strFileVersion;
}

stlString CFileVersionInfo::GetProductVersion() const
{
    return m_strProductVersion;
}

void CFileVersionInfo::Reset()
{
    ZeroMemory(&m_FileInfo, sizeof(m_FileInfo));
    m_strFileDescription.clear();
    m_strFileVersion.clear();
    m_strProductVersion.clear();
}

BOOL CFileVersionInfo::Create(LPCTSTR lpszFileName)
{
    Reset();

    DWORD	dwHandle;
    DWORD	dwFileVersionInfoSize = GetFileVersionInfoSize((LPTSTR)lpszFileName, &dwHandle);
    if (!dwFileVersionInfoSize)
        return FALSE;

    BYTE    *lpData = NULL;
    lpData = new BYTE[dwFileVersionInfoSize];

    try
    {
        if (!GetFileVersionInfo((LPTSTR)lpszFileName, dwHandle, dwFileVersionInfoSize, lpData))
            throw FALSE;

        // catch default information
        LPVOID	lpInfo;
        UINT	unInfoLen;
        if (VerQueryValue(lpData, _T("\\"), &lpInfo, &unInfoLen))
        {
            //ASSERT(unInfoLen == sizeof(m_FileInfo));
            if (unInfoLen == sizeof(m_FileInfo))
                memcpy(&m_FileInfo, lpInfo, unInfoLen);
        }

        // find best matching language and codepage
        VerQueryValue(lpData, _T("\\VarFileInfo\\Translation"), &lpInfo, &unInfoLen);

        DWORD	dwLangCode = 0;
        if (!GetTranslationId(lpInfo, unInfoLen, GetUserDefaultLangID(), dwLangCode, FALSE))
        {
            if (!GetTranslationId(lpInfo, unInfoLen, GetUserDefaultLangID(), dwLangCode, TRUE))
            {
                if (!GetTranslationId(lpInfo, unInfoLen, MAKELANGID(LANG_NEUTRAL, SUBLANG_NEUTRAL), dwLangCode, TRUE))
                {
                    if (!GetTranslationId(lpInfo, unInfoLen, MAKELANGID(LANG_ENGLISH, SUBLANG_NEUTRAL), dwLangCode, TRUE))
                        // use the first one we can get
                        dwLangCode = *((DWORD*)lpInfo);
                }
            }
        }

        stlString	strSubBlock;
        TCHAR buf[1024];
        _stprintf(buf, _T("\\StringFileInfo\\%04X%04X\\"), dwLangCode & 0x0000FFFF, (dwLangCode & 0xFFFF0000) >> 16);
        strSubBlock = buf;

        stlString sBuf;
        sBuf = strSubBlock;
        sBuf += _T("FileDescription");
        if (VerQueryValue(lpData, sBuf.c_str(), &lpInfo, &unInfoLen))
            m_strFileDescription = stlString((LPCTSTR)lpInfo);

        sBuf.clear();
        sBuf = strSubBlock;
        sBuf += _T("FileVersion");
        if (VerQueryValue(lpData, sBuf.c_str(), &lpInfo, &unInfoLen))
            m_strFileVersion = stlString((LPCTSTR)lpInfo);

        sBuf.clear();
        sBuf = strSubBlock;
        sBuf += _T("ProductVersion");
        if (VerQueryValue(lpData, sBuf.c_str(), &lpInfo, &unInfoLen))
            m_strProductVersion = stlString((LPCTSTR)lpInfo);

        delete[] lpData;
    }
    catch (BOOL)
    {
        delete[] lpData;
        return FALSE;
    }

    return TRUE;
}