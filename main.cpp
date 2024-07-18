#include <windows.h>
#include <iostream>
#include <string>
#include <Psapi.h>
#include <vector>
#include "CFileVersionInfo.h"

//get file description via its window - C++ Forum
//https://cplusplus.com/forum/windows/181615/
//and some other shit
//https://github.com/ritchielawrence/cmdow/blob/master/tlist.cpp

BOOL IsTaskbarWindow(HWND hwnd, HWND desktop) {
    DWORD styles = GetWindowLong(hwnd, GWL_STYLE);
    DWORD exstyles = GetWindowLong(hwnd, GWL_EXSTYLE);
    HWND owner = GetWindow(hwnd, GW_OWNER);
    HWND parent = GetParent(hwnd);
    int level;

    // determine windows' level. 0=desktop, 1=toplevel, 2+ = child
    if (hwnd == desktop) level = 0;
    else if ((parent == owner) || (parent == desktop)) level = 1;
    else {
        level = 1;
        while ((parent != NULL) && (parent != desktop)) {
            level++;
            parent = GetParent(parent);
        }
    }

    if ((!(styles & WS_VISIBLE)) ||
        (exstyles & WS_EX_TOOLWINDOW) ||
        owner || (level != 1))
        return FALSE;
    else return TRUE;
}

bool endsWith(const std::string &fullString, const std::string &ending) {
    if (fullString.length() >= ending.length()) {
        return (0 == fullString.compare(fullString.length() - ending.length(), ending.length(), ending));
    } else {
        return false;
    }
}

bool beginsWith(const std::string &fullString, const std::string &begining) {
    if (fullString.length() >= begining.length()) {
        return (0 == fullString.compare(0, begining.length(), begining));
    } else {
        return false;
    }
}

BOOL CALLBACK EnumWindowsProc(HWND hwnd, LPARAM lParam) {
    std::vector<HWND> *windows = reinterpret_cast<std::vector<HWND> *>(lParam);
    windows->push_back(hwnd);
    return TRUE;
}

int main() {
    int explorerCount = 0;
    bool dontPrint = FALSE;
    HWND desktop = GetDesktopWindow();

    std::vector<HWND> windows;
    EnumWindows(EnumWindowsProc, reinterpret_cast<LPARAM>(&windows));

    for (const auto &hwnd: windows) {
        char title[256];
        GetWindowTextA(hwnd, title, sizeof(title));

        DWORD pid;
        GetWindowThreadProcessId(hwnd, &pid);
        HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pid);

        TCHAR szExe[MAX_PATH + 1];
        DWORD lpdwSize = MAX_PATH;

        if (hProcess == nullptr)
            memset(szExe, 0, lpdwSize);
        else
            [[maybe_unused]] BOOL bFile = QueryFullProcessImageName(hProcess, 0, szExe, &lpdwSize);

        if (endsWith(szExe, "explorer.exe"))
            explorerCount++;

        dontPrint = beginsWith(szExe, "C:\\Windows\\");

        if (IsTaskbarWindow(hwnd, desktop) && !dontPrint || explorerCount == 1) {
            if (explorerCount == 1)
                explorerCount++;

            CFileVersionInfo data;
            data.Create(szExe);

            if (hProcess == nullptr)
                std::cout <<
                          hwnd << " ## C:\\" <<
                          title << ".exe ##  ##  ## " <<
                          title <<
                          std::endl;
            else
                std::cout <<
                          hwnd << " ## " <<
                          szExe << " ## " <<
                          data.GetFileVersion().c_str() << " ## " <<
                          data.GetProductVersion().c_str() << " ## " <<
                          data.GetFileDescription().c_str() <<
                          std::endl;

            CloseHandle(hProcess);
        }
    }
    return 0;
}