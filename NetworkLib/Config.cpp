#include <fstream>
#include <sstream>
#include <string>

#include "Config.h"


using namespace NetworkLib;

using namespace rapidjson;


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
ErrorCode Config::Load()
{
	std::ifstream configFileStream(Config::CONFIG_FILE_NAME);
	
	std::ostringstream outputStream;

	if (!configFileStream.is_open())
	{
		return ErrorCode::CONFIG_FILE_NOT_FOUND;
	}

	const int MAX_LINE_SIZE = 1024;
	char line[MAX_LINE_SIZE] = { 0, };

	while (configFileStream.getline(line, MAX_LINE_SIZE))
	{
		outputStream << line;
	}
	configFileStream.close();

	Document document;
	document.Parse(outputStream.str().c_str());
	if (ParseErrorCode::kParseErrorNone != document.GetParseError())
	{
		return ErrorCode::CONFIG_FILE_PARSE_ERROR;
	}
	
	for (auto iter = document.MemberBegin(); iter != document.MemberEnd(); ++iter)
	{
		ErrorCode errorCode = LoadFromMemberIter(iter);
		if (ErrorCode::SUCCESS != errorCode)
		{
			return errorCode;
		}
	}

	return ErrorCode::SUCCESS;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// TODO 최진우: 더 깔끔한 방법이 없을까?
ErrorCode Config::LoadFromMemberIter(const Value::ConstMemberIterator& iter)
{
	std::string memberName = iter->name.GetString();
	if (memberName == std::string{"IPAddress"})
	{
		if (!iter->value.IsString())
		{
			return ErrorCode::CONFIG_FILE_LOAD_FAIL;
		}

		std::wstring wideString;
		std::string ipString = iter->value.GetString();
		wideString.assign(ipString.begin(), ipString.end());
		mIPAddress = wideString.c_str();
	}
	else if (memberName == std::string{ "PortNum" })
	{
		if (!iter->value.IsUint())
		{
			return ErrorCode::CONFIG_FILE_LOAD_FAIL;
		}

		mPortNum = iter->value.GetUint();
	}
	else if (memberName == std::string{ "MaxSessionNum" })
	{
		if (!iter->value.IsUint())
		{
			return ErrorCode::CONFIG_FILE_LOAD_FAIL;
		}

		mMaxSessionNum = iter->value.GetUint();
	}
	else if (memberName == std::string{ "MaxSessionBufferSize" })
	{
		if (!iter->value.IsUint())
		{
			return ErrorCode::CONFIG_FILE_LOAD_FAIL;
		}

		mMaxSessionBufferSize = iter->value.GetUint();
	}
	else if (memberName == std::string{ "MaxPacketBodySize" })
	{
		if (!iter->value.IsUint())
		{
			return ErrorCode::CONFIG_FILE_LOAD_FAIL;
		}

		mMaxPacketBodySize = iter->value.GetUint();
	}
	else 
	{
		return ErrorCode::CONFIG_FILE_LOAD_FAIL;
	}

	return ErrorCode::SUCCESS;
}