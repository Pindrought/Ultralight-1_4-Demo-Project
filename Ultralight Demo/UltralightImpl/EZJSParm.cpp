#include <PCH.h>
#include "EZJSParm.h"

EZJSParm::EZJSParm()
	:m_Type(Type::Null)
{
}

EZJSParm::EZJSParm(bool value)
	:m_Type(Type::Boolean)
{
	m_ValueData = std::make_shared<bool>(value);
}

EZJSParm::EZJSParm(int value)
	:m_Type(Type::Number)
{
	m_ValueData = std::make_shared<double>(value);
}

EZJSParm::EZJSParm(double value)
	:m_Type(Type::Number)
{
	m_ValueData = std::make_shared<double>(value);
}

EZJSParm::EZJSParm(const char* value)
{
	if (value == nullptr)
	{
		m_Type = Type::Null;
	}
	else //c string?
	{
		m_Type =Type::String;
		m_ValueData = std::make_shared<string>(value);
	}
}

EZJSParm::EZJSParm(string value)
	:m_Type(Type::String)
{
	m_ValueData = std::make_shared<string>(value);
}

EZJSParm::EZJSParm(std::initializer_list<EZJSParm> value)
	:m_Type(Type::Array)
{
	m_ValueData = std::make_shared<vector<EZJSParm>>(value);
}

EZJSParm::EZJSParm(std::initializer_list<pair<string, EZJSParm>> value)
	:m_Type(Type::TableKeyValuePair)
{
	m_ValueData = std::make_shared<vector<pair<string, EZJSParm>>>(value);
}

EZJSParm::EZJSParm(vector<EZJSParm> value)
	:m_Type(Type::Array)
{
	m_ValueData = std::make_shared<vector<EZJSParm>>(value);
}

EZJSParm::EZJSParm(vector<pair<string, EZJSParm>> value)
	:m_Type(Type::TableKeyValuePair)
{
	m_ValueData = std::make_shared<vector<pair<string, EZJSParm>>>(value);
}

EZJSParm::Type EZJSParm::GetType() const
{
	return m_Type;
}

bool EZJSParm::AsBool() const
{
	assert(m_Type == EZJSParm::Type::Boolean);
	return *(bool*)(m_ValueData.get());
}

double EZJSParm::AsDouble() const
{
	assert(m_Type == EZJSParm::Type::Number);
	const double val = *(double*)(m_ValueData.get());
	return val;
}

string EZJSParm::AsString() const
{
	assert(m_Type == EZJSParm::Type::String);
	return *(string*)(m_ValueData.get());
}

vector<EZJSParm>& EZJSParm::AsArray()
{
	return *(vector<EZJSParm>*)(m_ValueData.get());
}

vector<pair<string, EZJSParm>>& EZJSParm::AsKeyValuePairsTable()
{
	return *(vector<pair<string,EZJSParm>>*)(m_ValueData.get());
}

JSValueRef EZJSParm::ToJSValueRef(JSContextRef inContext)
{
	switch (GetType())
	{
	case EZJSParm::Null:
		return JSValueMakeNull(inContext);
		break;
	case EZJSParm::Boolean:
		return JSValueMakeBoolean(inContext, AsBool());
		break;
	case EZJSParm::Number:
		return JSValueMakeNumber(inContext, AsDouble());
		break;
	case EZJSParm::String:
	{
		JSRetainPtr<JSStringRef> msg = adopt(JSStringCreateWithUTF8CString(AsString().c_str()));
		return JSValueMakeString(inContext, msg.get());
		break;
	}
	case EZJSParm::Array:
	{
		vector<JSValueRef> arrayItems = BuildJSValueRefParms(inContext, AsArray());
		return JSObjectMakeArray(inContext, arrayItems.size(), arrayItems.data(), nullptr);
		break;
	}
	case EZJSParm::TableKeyValuePair:
		JSObjectRef kvpObjectParm = JSObjectMake(inContext, nullptr, nullptr);
		vector<pair<string, EZJSParm>> kvps = AsKeyValuePairsTable();
		for (auto& kvp : kvps)
		{
			JSValueRef value = kvp.second.ToJSValueRef(inContext);
			JSRetainPtr<JSStringRef> key = adopt(JSStringCreateWithUTF8CString(kvp.first.c_str()));
			JSObjectSetProperty(inContext, kvpObjectParm, key.get(), value, kJSPropertyAttributeNone, nullptr);
		}
		return kvpObjectParm;
		break;
	}

	DebugBreak();
	return nullptr;
}

std::unique_ptr<char[]> JSStringToUTF8(JSStringRef ref)
{
	auto length = JSStringGetLength(ref) + 1;
	auto stringBuffer = std::make_unique<char[]>(length);
	JSStringGetUTF8CString(ref, stringBuffer.get(), length);
	return stringBuffer;
}

bool EZJSParm::CreateFromJSValue(JSContextRef inContext, JSValueRef inJSValueRef, EZJSParm& outParm, std::string& outException)
{
	JSType valueType = JSValueGetType(inContext, inJSValueRef);
	
	switch (valueType)
	{
	case JSType::kJSTypeNull:
	case JSType::kJSTypeUndefined: //Maybe should set up a custom type for undefined but for now just treating same as null
		outParm = EZJSParm();
		return true;
	case JSType::kJSTypeBoolean:
	{
		bool value = JSValueToBoolean(inContext, inJSValueRef);
		outParm = EZJSParm(value);
		return true;
	}
	case JSType::kJSTypeNumber:
	{
		double value = JSValueToNumber(inContext, inJSValueRef, nullptr);
		outParm = EZJSParm(value);
		return true;
	}
	case JSType::kJSTypeString:
	{
		auto stringRef = JSValueToStringCopy(inContext, inJSValueRef, nullptr);
		string str = JSStringToUTF8(stringRef).get();
		outParm = EZJSParm(str);
		return true;
	}
	case JSType::kJSTypeObject:
	{
		//TODO: Possibly other situations need to be handled?
		JSObjectRef objRef = JSValueToObject(inContext, inJSValueRef, nullptr);

		if (JSObjectIsFunction(inContext, objRef))
		{
			outException = "Cannot pass function from JS to CPP.";
			return false;
		}

		if (JSValueIsArray(inContext, inJSValueRef))
		{
			vector<EZJSParm> arr;

			JSRetainPtr<JSStringRef> msg = adopt(JSStringCreateWithUTF8CString("length"));
			JSValueRef len = JSObjectGetProperty(inContext, objRef, msg.get(), nullptr);
			int count = JSValueToNumber(inContext, len, nullptr);
			for (int i = 0; i < count; i++)
			{
				JSValueRef val = JSObjectGetPropertyAtIndex(inContext, objRef, i, nullptr);
				EZJSParm parm;
				if (!EZJSParm::CreateFromJSValue(inContext, val, parm, outException))
				{
					return false;
				}
				arr.push_back(parm);
			}
			outParm = EZJSParm(arr);

			return true;
		}
		else
		{
			if (JSValueIsObject(inContext, inJSValueRef))
			{
				vector<pair<string, EZJSParm>> kvp;
				JSPropertyNameArrayRef nameArray = JSObjectCopyPropertyNames(inContext, objRef);
				int arrayCount = JSPropertyNameArrayGetCount(nameArray);
				for (int i = 0; i < arrayCount; i++)
				{
					JSStringRef propertyName = JSPropertyNameArrayGetNameAtIndex(nameArray, i);
					JSValueRef val = JSObjectGetProperty(inContext, objRef, propertyName, nullptr);
					string propertyNameStr = JSStringToUTF8(propertyName).get();
					EZJSParm parm;
					if (!EZJSParm::CreateFromJSValue(inContext, val, parm, outException))
					{
						JSPropertyNameArrayRelease(nameArray);
						return false;
					}
					kvp.push_back(make_pair(propertyNameStr, parm));
				}
				JSPropertyNameArrayRelease(nameArray);
				outParm = EZJSParm(kvp);
				return true;
			}
		}
		break;
	}
	}
	outException = "Unsupported parameter(s) passed from JS to CPP.";
	return false;
}

std::string EZJSParm::ToString()
{
	switch (GetType())
	{
	case EZJSParm::Type::Null:
		return "null";
		break;
	case EZJSParm::Type::Boolean:
	{
		if (AsBool())
		{
			return "true";
		}
		else
		{
			return "false";
		}
	}
	case EZJSParm::Type::Number:
		return std::to_string(AsDouble());
		break;
	case EZJSParm::Type::String:
		return AsString();
		break;
	case EZJSParm::Type::Array:
	{
		auto arr = AsArray();
		string arrStr = "";
		for (int i=0; i<arr.size(); i++)
		{
			arrStr += strfmt("[%d]->[%s] || ", 
							 i, 
							 arr[i].ToString().c_str());
		}
		return arrStr;
		break;
	}
	case EZJSParm::Type::TableKeyValuePair:
	{
		auto kvpTable = AsKeyValuePairsTable();
		string kvpStr = "";
		for (auto kvp : kvpTable)
		{
			string key = kvp.first;
			EZJSParm& val = kvp.second;
			string msg = strfmt("[%s]->[%s]", 
								key.c_str(), 
								val.ToString().c_str());
			kvpStr += msg;
		}
		return kvpStr;
		break;
	}
	}
}

vector<JSValueRef> BuildJSValueRefParms(JSContextRef inContext, 
									   vector<EZJSParm>& inParmList)
{
	vector<JSValueRef> parms;
	for (auto& ezParm : inParmList)
	{
		parms.push_back(ezParm.ToJSValueRef(inContext));
	}
	return parms;
}