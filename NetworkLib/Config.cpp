#include <fstream>
#include <sstream>
#include <string>
#include <staticjson/staticjson.hpp>

#include "Config.h"


using namespace NetworkLib;


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
ErrorCode Config::Load()
{
	std::ifstream configFileStream(mConfigFileName);
	
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

	staticjson::ParseStatus parseResult;
	if(!staticjson::from_json_string<Config>(outputStream.str().c_str(), this, &parseResult))
	{
		return ErrorCode::CONFIG_FILE_PARSE_ERROR;
	}

	return ErrorCode::SUCCESS;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void Config::staticjson_init(staticjson::ObjectHandler* handler)
{
	handler->add_property("IPAddress", &mIPAddress);
	handler->add_property("PortNum", &mPortNum);
	handler->add_property("MaxSessionNum", &mMaxSessionNum);
	handler->add_property("MaxSessionBufferSize", &mMaxSessionBufferSize);
	handler->add_property("MaxPacketBodySize", &mMaxPacketBodySize);
	handler->set_flags(staticjson::Flags::DisallowUnknownKey);
}