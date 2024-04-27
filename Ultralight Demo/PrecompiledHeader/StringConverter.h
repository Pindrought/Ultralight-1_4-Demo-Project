#pragma once
#include <string>
#include <memory>
#include <stdexcept>

//This string format solution is from https://stackoverflow.com/questions/2342162/stdstring-formatting-like-sprintf
template<typename ... Args>
std::string strfmt(const std::string& format, Args ... args)
{
	int size_s = std::snprintf(nullptr, 0, format.c_str(), args ...) + 1; // Extra space for '\0'
	if (size_s <= 0) { throw std::runtime_error("Error during formatting."); }
	auto size = static_cast<size_t>(size_s);
	auto buf = std::make_unique<char[]>(size);
	std::snprintf(buf.get(), size, format.c_str(), args ...);
	std::string result = std::string(buf.get(), buf.get() + size - 1);
	return result; // We don't want the '\0' inside
}

class StringConverter
{
public:
    //Solution from https://stackoverflow.com/a/3999597
    static std::wstring s2ws(const std::string& str);

    static std::string ws2s(const std::wstring& wstr);
};