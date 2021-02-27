// Copyright(C) 2020 ISHIN.
#ifndef SRC_CLI_COMMON_H_
#define SRC_CLI_COMMON_H_

#if __GNUG__ <= 7
#include <experimental/filesystem>
namespace fs = std::experimental::filesystem;
#define FS_EXPERIMENTAL
#else
#include <filesystem>
namespace fs = std::filesystem;
#endif

#ifdef WINDOWS
#include <windows.h>
#define TO_PATH(x) charsToWchars((x))
#define TO_STR(x) wcharsToChars((x))
#else
#define TO_PATH(x) (x)
#define TO_STR(x) (x)
#endif

#ifdef WINDOWS
std::wstring charsToWchars(const std::string& in) {
    int len = MultiByteToWideChar(CP_ACP, 0, in.c_str(), -1, nullptr, 0);
    wchar_t* buf = new wchar_t[len+1];
    std::wstring output;
    memset(buf, 0, sizeof(wchar_t) * (len+1));
    if (MultiByteToWideChar(CP_ACP, 0, in.c_str(), -1, buf, len)) {
        output = buf;
    } else {
        std::cerr << "code convert error." << std::endl;
    }
    delete[] buf;

    return output;
}

std::string wcharsToChars(const std::wstring& in) {
    int len = WideCharToMultiByte(
        CP_ACP, 0, in.c_str(), -1, nullptr, 0, nullptr, nullptr);
    char* buf = new char[(len+1) * 2];
    std::string output;
    memset(buf, 0, sizeof(char) * (len+1) * 2);
    if (WideCharToMultiByte(
        CP_ACP, 0, in.c_str(), -1, buf, len, nullptr, nullptr)) {
        output = buf;
    } else {
        std::cerr << "code convert error." << std::endl;
    }
    delete[] buf;

    return output;
}
#endif

#endif  // SRC_CLI_COMMON_H_
