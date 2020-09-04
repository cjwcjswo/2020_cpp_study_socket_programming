#include "../NetworkLib/Network.h"
#include "../NetworkLib/Logger.h"
#include "PacketHandler.h"
#include "RoomManager.h"
#include "Room.h"
#include "UserManager.h"
#include "User.h"


using namespace CS;


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
ErrorCode PacketHandler::RoomEnter(const Packet& packet)
{
	RoomEnterResponse response;
	response.mErrorCode = ErrorCode::SUCCESS;
	response.mUserUniqueId = packet.mSessionUniqueId;

	User* user = mUserManager->FindUser(packet.mSessionUniqueId);
	if (user == nullptr)
	{
		response.mErrorCode = ErrorCode::USER_NOT_CONNECTED_STATE;
		mNetwork->Send(packet.mSessionIndex, static_cast<uint16>(PacketId::ROOM_ENTER_RESPONSE), reinterpret_cast<char*>(&response), sizeof(response));
		return response.mErrorCode;
	}

	RoomEnterRequest* request = reinterpret_cast<RoomEnterRequest*>(packet.mBodyData);

	Room* room = mRoomManager->FindRoom(request->mRoomIndex);
	if (room == nullptr)
	{
		response.mErrorCode = ErrorCode::ROOM_NOT_EXIST;
		mNetwork->Send(packet.mSessionIndex, static_cast<uint16>(PacketId::ROOM_ENTER_RESPONSE), reinterpret_cast<char*>(&response), sizeof(response));
		return response.mErrorCode;
	}

	ErrorCode errorCode = room->Enter(*user);
	if (errorCode != ErrorCode::SUCCESS)
	{
		response.mErrorCode = errorCode;
		mNetwork->Send(packet.mSessionIndex, static_cast<uint16>(PacketId::ROOM_ENTER_RESPONSE), reinterpret_cast<char*>(&response), sizeof(response));
		return response.mErrorCode;
	}
	mNetwork->Send(packet.mSessionIndex, static_cast<uint16>(PacketId::ROOM_ENTER_RESPONSE), reinterpret_cast<char*>(&response), sizeof(response));

	char* listNotifyBuffer = new char[RoomUserListNotify::Size(room->mRoomUserCount)];
	RoomUserListNotify* roomUserListNotify = reinterpret_cast<RoomUserListNotify*>(listNotifyBuffer);
	roomUserListNotify->mUserCount = room->mRoomUserCount;

	for (int i = 0; i < room->mRoomUserCount; ++i)
	{
		User* roomUser = room->mRoomUserList[i];
		roomUserListNotify->mRoomUserList[i].mUserUniqueId = roomUser->mSessionUniqueId;
		memcpy_s(roomUserListNotify->mRoomUserList[i].mUserIdList, MAX_USER_ID_SIZE, roomUser->mUserId, MAX_USER_ID_SIZE);
	}
	mNetwork->Send(packet.mSessionIndex, static_cast<uint16>(PacketId::ROOM_USER_LIST_NOTIFY), reinterpret_cast<char*>(roomUserListNotify), RoomUserListNotify::Size(room->mRoomUserCount));

	RoomNewUserBroadcast newUserBroadcast;
	newUserBroadcast.mUserUniueId = user->mSessionUniqueId;
	memcpy_s(newUserBroadcast.mUserId, MAX_USER_ID_SIZE, user->mUserId, MAX_USER_ID_SIZE);
	for (int i = 0; i < room->mRoomUserCount; ++i)
	{
		User* roomUser = room->mRoomUserList[i];
		if (roomUser->mSessionUniqueId == user->mSessionUniqueId)
		{
			continue;
		}
		mNetwork->Send(roomUser->mSessionIndex,
			static_cast<uint16>(PacketId::ROOM_NEW_USER_BROADCAST), reinterpret_cast<char*>(&newUserBroadcast), sizeof(newUserBroadcast));
	}

	delete[] listNotifyBuffer;

	return response.mErrorCode;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
ErrorCode PacketHandler::RoomLeave(const Packet& packet)
{
	RoomLeaveResponse response;
	response.mErrorCode = ErrorCode::SUCCESS;

	User* user = mUserManager->FindUser(packet.mSessionUniqueId);
	if (user == nullptr)
	{
		response.mErrorCode = ErrorCode::USER_NOT_CONNECTED_STATE;
		mNetwork->Send(packet.mSessionIndex, static_cast<uint16>(PacketId::ROOM_LEAVE_RESPONSE), reinterpret_cast<char*>(&response), sizeof(response));
		return response.mErrorCode;
	}

	Room* room = mRoomManager->FindRoom(user->mRoomIndex);
	if (room == nullptr)
	{
		response.mErrorCode = ErrorCode::ROOM_NOT_EXIST;
		mNetwork->Send(packet.mSessionIndex, static_cast<uint16>(PacketId::ROOM_LEAVE_RESPONSE), reinterpret_cast<char*>(&response), sizeof(response));
		return response.mErrorCode;
	}

	room->Leave(*user);

	mNetwork->Send(packet.mSessionIndex, static_cast<uint16>(PacketId::ROOM_ENTER_RESPONSE), reinterpret_cast<char*>(&response), sizeof(response));

	RoomLeaveUserBroadcast leaveUserBroadcast;
	leaveUserBroadcast.mUserUniqueId = user->mSessionUniqueId;
	for (int i = 0; i < room->mRoomUserCount; ++i)
	{
		User* roomUser = room->mRoomUserList[i];
		if (roomUser->mSessionUniqueId == user->mSessionUniqueId)
		{
			continue;
		}
		mNetwork->Send(roomUser->mSessionIndex, 
			static_cast<uint16>(PacketId::ROOM_NEW_USER_BROADCAST), reinterpret_cast<char*>(&leaveUserBroadcast), sizeof(leaveUserBroadcast));
	}

	return response.mErrorCode;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
ErrorCode PacketHandler::RoomChat(const Packet& packet)
{
	RoomChatRequest* request = reinterpret_cast<RoomChatRequest*>(packet.mBodyData);
	RoomChatResponse response;

	User* user = mUserManager->FindUser(packet.mSessionUniqueId);
	if (user == nullptr)
	{
		response.mErrorCode = ErrorCode::USER_NOT_CONNECTED_STATE;
		mNetwork->Send(packet.mSessionIndex, static_cast<uint16>(PacketId::ROOM_CHAT_RESPONSE), reinterpret_cast<char*>(&response), sizeof(response));
		return response.mErrorCode;
	}

	Room* room = mRoomManager->FindRoom(user->mRoomIndex);
	if (room == nullptr)
	{
		response.mErrorCode = ErrorCode::ROOM_NOT_EXIST;
		mNetwork->Send(packet.mSessionIndex, static_cast<uint16>(PacketId::ROOM_CHAT_RESPONSE), reinterpret_cast<char*>(&response), sizeof(response));
		return response.mErrorCode;
	}

	RoomChatBroadcast broadcast;
	broadcast.mUid = packet.mSessionUniqueId;
	broadcast.mMessageLen = request->mMessageLen;
	wmemcpy_s(broadcast.mMessage, request->mMessageLen, request->mMessage, request->mMessageLen);

	for (int i = 0; i < room->mRoomUserCount; ++i)
	{
		User* roomUser = room->mRoomUserList[i];
		mNetwork->Send(roomUser->mSessionIndex, static_cast<uint16>(PacketId::ROOM_CHAT_BROADCAST), reinterpret_cast<char*>(&broadcast), sizeof(broadcast) - MAX_CHAT_SIZE - broadcast.mMessageLen);
	}

	GLogger->PrintConsole(Color::LGREEN, L"<RoomChat> [%lu]: %ls\n", 5, broadcast.mMessage);

	response.mErrorCode = ErrorCode::SUCCESS;
	mNetwork->Send(packet.mSessionIndex, static_cast<uint16>(PacketId::ROOM_CHAT_RESPONSE), reinterpret_cast<char*>(&response), sizeof(response));

	return ErrorCode::SUCCESS;
}