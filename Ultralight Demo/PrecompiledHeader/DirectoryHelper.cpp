#include "PCH.h"
#include "DirectoryHelper.h"
#include <sys/stat.h>

std::wstring DirectoryHelper::s_AssetsDirectory = L"";
std::string DirectoryHelper::s_AssetsDirectoryA = "";
std::wstring DirectoryHelper::s_ExecutableDirectory = L"";
std::string DirectoryHelper::s_ExecutableDirectoryA = "";
std::wstring DirectoryHelper::s_WebDirectory = L"";
std::string DirectoryHelper::s_WebDirectoryA = "";

bool DirectoryHelper::DirectoryExists(std::string path)
{
	std::filesystem::path filepath = path;
	bool filepathExists = std::filesystem::is_directory(filepath.parent_path());
	return filepathExists;
}

std::string DirectoryHelper::GetAssetsDirectoryA()
{
	if (s_AssetsDirectoryA != "")
		return s_AssetsDirectoryA;

	s_AssetsDirectoryA =  GetExecutableDirectoryA() + "Assets/";
	if (!DirectoryExists(s_AssetsDirectoryA))
	{
		s_AssetsDirectoryA = GetExecutableDirectoryA() + "../Assets/";
		if (!DirectoryExists(s_AssetsDirectoryA))
		{
			FatalError("Missing assets directory. The executable directory was checked as well as the parent directory to the executable.");
		}
	}

	return s_AssetsDirectoryA;
}

std::wstring DirectoryHelper::GetAssetsDirectory()
{
	if (s_AssetsDirectory != L"")
		return s_AssetsDirectory;

	s_AssetsDirectory = StringConverter::s2ws(GetAssetsDirectoryA());
	return s_AssetsDirectory;
}

std::string DirectoryHelper::GetExecutableDirectoryA()
{
	if (s_ExecutableDirectoryA != "")
		return s_ExecutableDirectoryA;

	char szExecutablePath[MAX_PATH];
	GetModuleFileNameA(NULL, szExecutablePath, MAX_PATH);

	std::string executablePath(szExecutablePath);

	s_ExecutableDirectoryA = executablePath.substr(0, executablePath.find_last_of("/\\")) + "/";
	s_ExecutableDirectoryA = NormalizePathA(s_ExecutableDirectoryA); //Replace \\ with /
	return s_ExecutableDirectoryA;
}

std::wstring DirectoryHelper::GetExecutableDirectory()
{
	if (s_ExecutableDirectory != L"")
		return s_ExecutableDirectory;

	s_ExecutableDirectory = StringConverter::s2ws(GetExecutableDirectoryA());
	return s_ExecutableDirectory;
}

std::string DirectoryHelper::NormalizePathA(std::string path)
{
	if (!path.empty())
	{
		for (size_t i = 0; i < path.length(); ++i)
		{
			if (path[i] == '\\')
				path[i] = '/';
		}
	}
	return path;
}

std::wstring DirectoryHelper::NormalizePath(std::wstring path)
{
	if (!path.empty())
	{
		for (size_t i = 0; i < path.length(); ++i)
		{
			if (path[i] == L'\\')
				path[i] = L'/';
		}
	}
	return path;
}

std::string DirectoryHelper::GetDirectoryFromPath(const std::string& filepath)
{
	size_t off1 = filepath.find_last_of('\\');
	size_t off2 = filepath.find_last_of('/');
	if (off1 == std::string::npos && off2 == std::string::npos) //If no slash or backslash in path?
	{
		return "";
	}
	if (off1 == std::string::npos)
	{
		return filepath.substr(0, off2);
	}
	if (off2 == std::string::npos)
	{
		return filepath.substr(0, off1);
	}
	//If both exists, need to use the greater offset
	return filepath.substr(0, std::max(off1, off2));
}

std::string DirectoryHelper::GetFileFromPath(const std::string& filepath)
{
	size_t off1 = filepath.find_last_of('\\');
	size_t off2 = filepath.find_last_of('/');
	if (off1 == std::string::npos && off2 == std::string::npos) //If no slash or backslash in path?
	{
		return filepath;
	}

	return filepath.substr(filepath.find_last_of("/\\") + 1);
}

std::string DirectoryHelper::GetUltralightResourcesDirectoryA()
{
	return DirectoryHelper::GetExecutableDirectoryA() + "resources/";
}

std::string DirectoryHelper::GetWebDirectoryA()
{
	if (s_WebDirectoryA != "")
		return s_WebDirectoryA;

	s_WebDirectoryA = GetExecutableDirectoryA() + "web/";
	if (!DirectoryExists(s_WebDirectoryA))
	{
		s_WebDirectoryA = GetExecutableDirectoryA() + "../web/";
		if (!DirectoryExists(s_WebDirectoryA))
		{
			FatalError("Missing web directory. The executable directory was checked as well as the parent directory to the executable.");
		}
	}

	return s_WebDirectoryA;
}

std::wstring DirectoryHelper::GetWebDirectory()
{
	if (s_WebDirectory != L"")
		return s_WebDirectory;

	s_WebDirectory = StringConverter::s2ws(GetWebDirectoryA());
	return s_WebDirectory;
}
