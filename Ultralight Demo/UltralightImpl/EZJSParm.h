#pragma once
#include <PCH.h>

std::unique_ptr<char[]> JSStringToUTF8(JSStringRef ref);
#define kvp(k, v) make_pair(k, v) //KeyValuePair for EZJSParm Key=string, value=EZJSParm

class EZJSParm
{
public:
	enum Type
	{
		Null,
		Boolean,
		Number,
		String,
		TableKeyValuePair,
		Array,
	};
	EZJSParm();
	EZJSParm(bool value);
	EZJSParm(int value);
	EZJSParm(double value);
	EZJSParm(const char* value);
	EZJSParm(string value);
	EZJSParm(std::initializer_list<EZJSParm> value);
	EZJSParm(std::initializer_list<pair<string, EZJSParm>> value);

	EZJSParm(vector<EZJSParm> value);
	EZJSParm(vector<pair<string, EZJSParm>> value);
	~EZJSParm() {}
	Type GetType() const;

	bool AsBool() const;
	double AsDouble() const;
	string AsString() const;
	vector<EZJSParm>& AsArray();
	vector<pair<string,EZJSParm>>& AsKeyValuePairsTable();

	JSValueRef ToJSValueRef(JSContextRef inContext);
	static bool CreateFromJSValue(JSContextRef inContext, JSValueRef inJSValueRef, EZJSParm& outParm, std::string& outException);
	std::string ToString();
private:
	shared_ptr<void> m_ValueData = nullptr;
	Type m_Type = Type::Null;
};

vector<JSValueRef> BuildJSValueRefParms(JSContextRef inContext, vector<EZJSParm>& inParmList);
