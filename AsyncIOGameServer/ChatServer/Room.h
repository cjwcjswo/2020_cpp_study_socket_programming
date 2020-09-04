#pragma once

#include <queue>

#include "../NetworkLib/Define.h"
#include "ErrorCode.h"
#include "User.h"


class Room
{
public:
	Room() = default;
	~Room() = default;


private:
	int32 mRoomIndex = INVALID_INDEX;
	uint16 mMaxRoomUserNum = 0;
	std::queue<int32> mRoomUserIndexPool;


public:
	// TODO �����  �Ϻη� deque�� ����� �� ������ ������ �����ΰ���? �����̳� �߻�ȭ ���鿡�� �ùٸ��� �ʽ��ϴ�.
	// ���� �Ϸ�
	User** mRoomUserList = nullptr;
	uint16 mRoomUserCount = 0;


private:
	User* FindUser(const uint64 sessionUniqueId);


public:
	void Init(const int32 roomIndex, const uint16 maxUserNum);
	CS::ErrorCode Enter(User& user);
	CS::ErrorCode Leave(User& user);
};