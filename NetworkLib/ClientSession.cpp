#include "ClientSession.h"

std::string ClientSession::String() const
{
	std::ostringstream out;
	out << "Index: " << this->index_ << ", UniqueId: " << this->unique_id_;
	return out.str();
}