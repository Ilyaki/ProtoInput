#pragma once
#include <cstdint>
#include "protoloader.h"

namespace Proto
{

#define PROTO_MESSAGE_FILTERS MessageFilterTest

template<unsigned int FilterId, unsigned int MsgMin, unsigned int MsgMax = MsgMin>
struct MessageFilterBase
{
	constexpr static unsigned int MessageMin() { return MsgMin; }
	constexpr static unsigned int MessageMax() { return MsgMax; }
	constexpr static unsigned int FilterID() { return FilterId; }
};

class MessageFilterTest : public MessageFilterBase<ProtoMessageFilterIDs::TestFilterID, 0x00FF>
{
public:
	static bool Filter(unsigned int message, unsigned int lparam, unsigned int wparam, intptr_t hwnd)
	{
		return true;
	}

	static const char* Name() { return "MessageFilterTest"; }
	static const char* Description() { return "MessageFilterTest desc"; }
};

}
