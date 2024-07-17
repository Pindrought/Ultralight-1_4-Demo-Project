#include <PCH.h>
#include "CustomFileSystem.h"
#include "../../../UltralightImpl/impl/MimeTypeHelper.h"
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
#include "../../../UltralightImpl/impl/FileSystemWin.h"

using namespace ultralight;

bool IsStagingTexturePath(const ul::String& inPath, string& outPath)
{
    string path(inPath.utf8().data());
    size_t backslashIndex1 = path.find_last_of('/');
    size_t backslashIndex2 = path.find_last_of('\\');

    int backslashIndex = backslashIndex1;
    if (backslashIndex == std::string::npos)
    {
        backslashIndex = backslashIndex2;
    }
    else
    {
        if (backslashIndex2 != std::string::npos && backslashIndex2 > backslashIndex1)
        {
            backslashIndex = backslashIndex2;
        }
    }

    if (backslashIndex == std::string::npos) //No backslashes? I don't think this should be possible
    {
    }
    else
    {
        if (path.length() > backslashIndex) //This should always be true?
        {
            string extension = path.substr(path.length() - 4, 4);
            if (extension != ".png")
            {
                return false;
            }
            string fileName = path.substr(backslashIndex + 1);

            const char* stagingTextureFilePrefix = "__STAGINGTEXTURE__";

            if (fileName.length() > strlen(stagingTextureFilePrefix)) //Make sure file name is long enough to be valid before comparing
            {
                if (memcmp(stagingTextureFilePrefix, fileName.c_str(), strlen(stagingTextureFilePrefix)) == 0)
                {
                    outPath = fileName;
                    return true;
                }
            }
        }
    }
    return false;
}

CustomFileSystem::CustomFileSystem(LPCWSTR baseDir) {
    m_BaseDirectory.reset(new WCHAR[_MAX_PATH]);
    StringCchCopyW(m_BaseDirectory.get(), MAX_PATH, baseDir);
}

CustomFileSystem::~CustomFileSystem() { }

bool CustomFileSystem::FileExists(const ul::String& path) {
    string stagingFileName = "";
    if (IsStagingTexturePath(path, stagingFileName))
    {
        return true;
    }
    WIN32_FIND_DATAW findData;
    return getFindData(GetRelative(path).get(), findData);
}

ul::String CustomFileSystem::GetFileMimeType(const ul::String& file_path) {
    ul::String16 path16 = file_path.utf16();
    LPWSTR ext = PathFindExtensionW(path16.data());
    std::wstring mimetype = GetMimeType(ext);
    return ul::String16(mimetype.c_str(), mimetype.length());
}

ul::String CustomFileSystem::GetFileCharset(const ul::String& file_path) { return "utf-8"; }

struct CustomFileSystem_BufferData {
    HANDLE hFile;
    HANDLE hMap;
    LPVOID lpBasePtr;
};

void CustomFileSystem_DestroyFileBufferCallback(void* user_data, void* data)
{
    delete[] user_data;
}

ul::RefPtr<ul::Buffer> CustomFileSystem::OpenFile(const ul::String& file_path) {

    std::unique_ptr<WCHAR[]> pathStr;

    //Check if this is a staging texture path
    string stagingFileName = "";
    if (IsStagingTexturePath(file_path, stagingFileName))
    {
        pathStr = GetRelative("Samples/CPPTextureInBrowser/StagingTexture.png");

        string pixelIdStr = stagingFileName.substr(strlen("__STAGINGTEXTURE__")); //Remove the __STAGINGTEXTURE__ prefix text so we just have the pixel id & .png (ex. 1.png)
        pixelIdStr = pixelIdStr.substr(0, pixelIdStr.length() - 4); //strip the .png out
        uint32_t pixelId = std::stoul(pixelIdStr.c_str());

        uint32_t pixelUID = 4293283327;
        ul::RefPtr<ul::Bitmap> bmp = ul::Bitmap::Create(1, //width
                                                        1, //height
                                                        ul::BitmapFormat::BGRA8_UNORM_SRGB, //format
                                                        4, //row_bytes
                                                        &pixelUID, //ptr to pixels (just 4 bytes to store the uint32_t)
                                                        4, //size
                                                        true); //should copy

        auto rowbytes = bmp->size();

        auto buffer = bmp->EncodePNG(false, false); //In gpu driver getting 0 for the uin32_t when reading pixel?
        bmp->WritePNG("test.png", false, false);  //<--1 white pixel with 255r/g/b?

        {
            void* pPixels = bmp->LockPixels();
            uint32_t test = 0;
            memcpy(&test, pPixels, sizeof(uint32_t)); //this correctly shows as 50 here
            bmp->UnlockPixels();
        }

        return buffer;
    }
    else
    {
        pathStr = GetRelative(file_path);
    }

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
    return ul::Buffer::Create(fileBuffer, totalFileBytes, fileBuffer, CustomFileSystem_DestroyFileBufferCallback);
}

//#include "../../../Misc/IDPoolManager.h"
//static IDPoolManager<uint32_t> s_CustomFileSystemStagingTextureIds(1u);
//
//string CustomFileSystem::GetAvailableStagingTextureName()
//{
//    string stagingTexturePath = "__STAGINGTEXTURE__";
//    stagingTexturePath += std::to_string(s_CustomFileSystemStagingTextureIds.GetNextId()) + ".png";
//    return stagingTexturePath;
//}

std::unique_ptr<WCHAR[]> CustomFileSystem::GetRelative(const ul::String& path) {
    ul::String16 path16 = path.utf16();
    if (strcmp(path.utf8().data(), "resources/icudt67l.dat") == 0 ||
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