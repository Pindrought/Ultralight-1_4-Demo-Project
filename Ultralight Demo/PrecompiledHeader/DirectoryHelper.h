#pragma once
#include <string>
#include <vector>

class DirectoryHelper
{
public:
	static bool DirectoryExists(std::string path);
	static std::string GetParentDirectory(std::string path);
	static std::vector<std::string> GetListOfDrives();
	static std::string GetAssetsDirectoryA();
	static std::wstring GetAssetsDirectory();
	static std::string GetExecutableDirectoryA();
	static std::wstring GetExecutableDirectory();
	static std::string NormalizePathA(std::string path);
	static std::wstring NormalizePath(std::wstring path);
	static std::string GetDirectoryFromPath(const std::string& filepath);
	static std::string GetFileFromPath(const std::string& filepath);

	static std::string GetUltralightResourcesDirectoryA(); //for icudt67l.dat / cacert.pem
	static std::string GetWebDirectoryA();
	static std::wstring GetWebDirectory();
private:
	static std::wstring s_AssetsDirectory;
	static std::string s_AssetsDirectoryA;
	static std::wstring s_ExecutableDirectory;
	static std::string s_ExecutableDirectoryA;
	static std::wstring s_WebDirectory;
	static std::string s_WebDirectoryA;
};