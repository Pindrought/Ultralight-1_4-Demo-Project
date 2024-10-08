#pragma once
#include <PCH.h>

bool getFindData(LPCWSTR path, WIN32_FIND_DATAW& findData);

std::wstring GetMimeType(const std::wstring& szExtension);

class FileSystemWin : public ul::FileSystem {
public:
	// Construct FileSystemWin instance.
	// 
	// @note You can pass a valid baseDir here which will be prepended to
	//       all file paths. This is useful for making all File URLs relative
	//       to your HTML asset directory.
	FileSystemWin(LPCWSTR baseDir);

	virtual ~FileSystemWin();

	virtual bool FileExists(const ul::String& file_path) override;

	virtual ul::String GetFileMimeType(const ul::String& file_path) override;

	virtual ul::String GetFileCharset(const ul::String& file_path) override;

	virtual ul::RefPtr<ul::Buffer> OpenFile(const ul::String& file_path) override;

protected:
	std::unique_ptr<WCHAR[]> GetRelative(const ul::String& path);

	std::unique_ptr<WCHAR[]> m_BaseDirectory;
};