#include "SetActorList.h"
#include "../../ZFile.h"
#include "../ZRoom.h"
#include "../ActorList.h"
#include "../../BitConverter.h"
#include "../../StringHelper.h"

using namespace std;

SetActorList::SetActorList(ZRoom* nZRoom, std::vector<uint8_t> rawData, int rawDataIndex) : ZRoomCommand(nZRoom, rawData, rawDataIndex)
{
	numActors = rawData[rawDataIndex + 1];
	segmentOffset = BitConverter::ToInt32BE(rawData, rawDataIndex + 4) & 0x00FFFFFF;

	_rawData = rawData;
	_rawDataIndex = rawDataIndex;

	if (segmentOffset != 0)
		zRoom->parent->declarations[segmentOffset] = new Declaration(DeclarationAlignment::None, 0, "");
}

string SetActorList::GetSourceOutputCode(std::string prefix)
{
	return "";
}

string SetActorList::GenerateSourceCodePass1(string roomName, int baseAddress)
{
	return "";
}


string SetActorList::GenerateSourceCodePass2(string roomName, int baseAddress)
{
	string sourceOutput = "";
	int numActorsReal = zRoom->GetDeclarationSizeFromNeighbor(segmentOffset) / 16;
	actors = vector<ActorSpawnEntry*>();
	uint32_t currentPtr = segmentOffset;

	for (int i = 0; i < numActorsReal; i++)
	{
		ActorSpawnEntry* entry = new ActorSpawnEntry(_rawData, currentPtr);
		actors.push_back(entry);

		currentPtr += 16;
	}

	sourceOutput += StringHelper::Sprintf("%s 0x%02X, (u32)_%s_actorList_%08X };", ZRoomCommand::GenerateSourceCodePass1(roomName, baseAddress).c_str(), numActors, roomName.c_str(), segmentOffset);

	string declaration = "";
	declaration += StringHelper::Sprintf("ActorEntry _%s_actorList_%08X[%i] = \n{\n", roomName.c_str(), segmentOffset, actors.size());

	int index = 0;
	for (ActorSpawnEntry* entry : actors)
	{
		if (entry->actorNum < sizeof(ActorList) / sizeof(ActorList[0]))
			declaration += StringHelper::Sprintf("\t{ %s, %i, %i, %i, %i, %i, %i, 0x%04X }, //0x%08X \n", ActorList[entry->actorNum].c_str(), entry->posX, entry->posY, entry->posZ, entry->rotX, entry->rotY, entry->rotZ, (uint16_t)entry->initVar, segmentOffset + (index * 16));
		else
			declaration += StringHelper::Sprintf("\t{ 0x%04X, %i, %i, %i, %i, %i, %i, 0x%04X }, //0x%08X \n", entry->actorNum, entry->posX, entry->posY, entry->posZ, entry->rotX, entry->rotY, entry->rotZ, (uint16_t)entry->initVar, segmentOffset + (index * 16));

		index++;
	}

	declaration += "};\n\n";

	zRoom->parent->declarations[segmentOffset] = new Declaration(DeclarationAlignment::None, DeclarationPadding::Pad16, actors.size() * 16, declaration);
	return sourceOutput;
}

int32_t SetActorList::GetRawDataSize()
{
	return ZRoomCommand::GetRawDataSize() + (actors.size() * 16);
}

string SetActorList::GenerateExterns()
{
	return StringHelper::Sprintf("extern ActorEntry _%s_actorList_%08X[%i];\n", zRoom->GetName().c_str(), segmentOffset, actors.size());
}

string SetActorList::GetCommandCName()
{
	return "SCmdActorList";
}

RoomCommand SetActorList::GetRoomCommand()
{
	return RoomCommand::SetActorList;
}

ActorSpawnEntry::ActorSpawnEntry(std::vector<uint8_t> rawData, int rawDataIndex)
{
	uint8_t* data = rawData.data();

	actorNum = BitConverter::ToInt16BE(data, rawDataIndex + 0);
	posX = BitConverter::ToInt16BE(data, rawDataIndex + 2);
	posY = BitConverter::ToInt16BE(data, rawDataIndex + 4);
	posZ = BitConverter::ToInt16BE(data, rawDataIndex + 6);
	rotX = BitConverter::ToInt16BE(data, rawDataIndex + 8);
	rotY = BitConverter::ToInt16BE(data, rawDataIndex + 10);
	rotZ = BitConverter::ToInt16BE(data, rawDataIndex + 12);
	initVar = BitConverter::ToInt16BE(data, rawDataIndex + 14);
}