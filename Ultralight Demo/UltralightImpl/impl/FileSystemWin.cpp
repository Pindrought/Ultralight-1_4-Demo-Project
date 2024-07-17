#include <PCH.h>
#include "FileSystemWin.h"
#include "MimeTypeHelper.h"
#include <io.h>
#include <shlobj.h>
#include <shlwapi.h>
#include <sys/stat.h>
#include <windows.h>
#include <limits>
#include <stdio.h>
#include <Ultralight/String.h>
#include <Ultralight/Buffer.h>
#include <string>
#include <algorithm>
#include <memory>
#include <Strsafe.h>

using namespace ultralight;

bool getFindData(LPCWSTR path, WIN32_FIND_DATAW& findData) {
    HANDLE handle = FindFirstFileW(path, &findData);
    if (handle == INVALID_HANDLE_VALUE)
        return false;
    FindClose(handle);
    return true;
}

std::wstring GetMimeType(const std::wstring& szExtension) {
    // return mime type for extension
    HKEY hKey = NULL;
    std::wstring szResult = L"application/unknown";

    // open registry key
    if (RegOpenKeyExW(HKEY_CLASSES_ROOT, szExtension.c_str(), 0, KEY_READ, &hKey) == ERROR_SUCCESS) {
        // define buffer
        wchar_t szBuffer[256] = { 0 };
        DWORD dwBuffSize = sizeof(szBuffer);

        // get content type
        if (RegQueryValueExW(hKey, L"Content Type", NULL, NULL, (LPBYTE)szBuffer, &dwBuffSize)
            == ERROR_SUCCESS) {
            // success
            szResult = szBuffer;
        }

        // close key
        RegCloseKey(hKey);
    }

    // return result
    return szResult;
}

FileSystemWin::FileSystemWin(LPCWSTR baseDir) {
    m_BaseDirectory.reset(new WCHAR[_MAX_PATH]);
    StringCchCopyW(m_BaseDirectory.get(), MAX_PATH, baseDir);
}

FileSystemWin::~FileSystemWin() { }

bool FileSystemWin::FileExists(const ul::String& path) {
    WIN32_FIND_DATAW findData;
    return getFindData(GetRelative(path).get(), findData);
}

ul::String FileSystemWin::GetFileMimeType(const ul::String& file_path) {
    ul::String16 path16 = file_path.utf16();
    LPWSTR ext = PathFindExtensionW(path16.data());
    std::wstring mimetype = GetMimeType(ext);
    return ul::String16(mimetype.c_str(), mimetype.length());
}

ul::String FileSystemWin::GetFileCharset(const ul::String& file_path) { return "utf-8"; }

struct FileSystemWin_BufferData {
    HANDLE hFile;
    HANDLE hMap;
    LPVOID lpBasePtr;
};

void FileSystemWin_DestroyFileBufferCallback(void* user_data, void* data)
{
    delete[] user_data;
}

ul::RefPtr<ul::Buffer> FileSystemWin::OpenFile(const ul::String& file_path) {
    auto pathStr = GetRelative(file_path);
    HANDLE hFile;
    HANDLE hMap;
    LPVOID lpBasePtr;
    LARGE_INTEGER liFileSize;

    hFile = CreateFile(pathStr.get(),
                       GENERIC_READ,          // dwDesiredAccess
                       FILE_SHARE_READ,       // dwShareMode
                       NULL,                  // lpSecurityAttributes
                       OPEN_EXISTING,         // dwCreationDisposition
                       FILE_ATTRIBUTE_NORMAL, // dwFlagsAndAttributes
                       0);                    // hTemplateFile

    if (hFile == INVALID_HANDLE_VALUE) {
        return nullptr;
    }

    if (!GetFileSizeEx(hFile, &liFileSize)) {
        CloseHandle(hFile);
        return nullptr;
    }

    if (liFileSize.QuadPart == 0) {
        CloseHandle(hFile);
        return nullptr;
    }

    DWORD totalFileBytes = liFileSize.QuadPart;
    char* fileBuffer = new char[totalFileBytes];
    if (fileBuffer == nullptr)
    {
        CloseHandle(hFile);
        return nullptr;
    }

    DWORD totalBytesRead = 0;
    while (totalBytesRead < totalFileBytes)
    {
        DWORD remainingBytes = totalFileBytes - totalBytesRead;
        DWORD bytesRead = 0;
        BOOL result = ReadFile(hFile, &fileBuffer[totalBytesRead], remainingBytes, &bytesRead, nullptr);
        if (result == FALSE)
        {
            delete[] fileBuffer;
            CloseHandle(hFile);
            return nullptr;
        }
        totalBytesRead += bytesRead;
    }

    CloseHandle(hFile);
    return ul::Buffer::Create(fileBuffer, totalFileBytes, fileBuffer, FileSystemWin_DestroyFileBufferCallback);
}

std::unique_ptr<WCHAR[]> FileSystemWin::GetRelative(const ul::String& path) {
    ul::String16 path16 = path.utf16();
    if (strcmp(path.utf8().data(),"resources/icudt67l.dat") == 0 ||
        strcmp(path.utf8().data(), "resources/cacert.pem") == 0)
    {
        std::unique_ptr<WCHAR[]> relPath(new WCHAR[_MAX_PATH]);
        PathCombineW(relPath.get(), DirectoryHelper::GetExecutableDirectory().c_str(), path16.data());
        return relPath;
    }
    std::unique_ptr<WCHAR[]> relPath(new WCHAR[_MAX_PATH]);
    memset(relPath.get(), 0, _MAX_PATH * sizeof(WCHAR));
    PathCombineW(relPath.get(), m_BaseDirectory.get(), path16.data());
    return relPath;
}

// Called from Platform.cpp -- Maybe never called actually?
ultralight::FileSystem* CreatePlatformFileSystem(const ul::String& baseDir) {
    std::wstring baseDirWStr(baseDir.utf16().data());

    WCHAR cur_dir[_MAX_PATH];
    GetCurrentDirectoryW(_MAX_PATH, cur_dir);
    WCHAR absolute_dir[_MAX_PATH];
    PathCombineW(absolute_dir, cur_dir, baseDirWStr.c_str());

    return new FileSystemWin(absolute_dir);
}