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

    //
    // determine windows' level. 0=desktop, 1=toplevel, 2+ = child
    //
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

int main() {
//    std::ofstream logfile("log.txt"); // создаем объект для записи в файл log.txt
//    std::streambuf* oldCoutStreamBuf = std::cout.rdbuf();
//
//    std::string log = "0";
//    if (argc >= 2)
//        log = argv[2];
//    std::cout << "Log option = " << log << std::endl;
//
//    if (log == "1") {
//        if (logfile.is_open())  // проверяем, успешно ли открыт файл
//            // Сохраняем стандартный вывод в переменную
//            std::cout.rdbuf(logfile.rdbuf()); // перенаправляем стандартный вывод в файл
//        else {
//            std::cerr << "Unable to open file log.txt" << std::endl;
//        }
//    }
    int explorerCount = 0;
    bool dontPrint = FALSE;
    HWND desktop = GetDesktopWindow();

    HWND hwnd = FindWindow(NULL, NULL);
    while (hwnd) {
        DWORD pid;
        GetWindowThreadProcessId(hwnd, &pid);
        HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION, FALSE, pid);

        TCHAR szExe[MAX_PATH + 1];
        DWORD lpdwSize = MAX_PATH;

        [[maybe_unused]] BOOL bFile = QueryFullProcessImageName(hProcess, 0, szExe, &lpdwSize);

        if (endsWith(szExe, "explorer.exe"))
            explorerCount++;

        dontPrint = beginsWith(szExe, "C:\\Windows\\");

        if (IsTaskbarWindow(hwnd, desktop) && !dontPrint || explorerCount == 1) {
            if (explorerCount == 1)
                explorerCount++;

            CFileVersionInfo data;
            data.Create(szExe);

            std::cout <<
                      hwnd << " ## " <<
                      szExe << " ## " <<
                      data.GetFileVersion().c_str() << " ## " <<
                      data.GetProductVersion().c_str() << " ## " <<
                      data.GetFileDescription().c_str() <<
                      std::endl;

            CloseHandle(hProcess);
        }
        hwnd = FindWindowEx(NULL, hwnd, NULL, NULL);
    }

//    if (log == "1") {
//        std::cout.rdbuf(oldCoutStreamBuf);
//        logfile.close();
//    }

    return 0;
}
