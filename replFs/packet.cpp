/*
 * packet.cpp
 *
 *  Created on: May 17, 2014
 *      Author: songhan
 */
#include "packet.h"

PacketBase::PacketBase(uint8_t type, uint8_t nodeType, uint32_t GUID, int fd,
                       uint32_t seqNum, uint32_t transNum) {
  this->type = type;
  this->nodeType = nodeType;
  this->checkSum = 0;
  this->GUID = GUID;
  this->fd = fd;
  this->seqNum = seqNum;
  this->transNum = transNum;
  this->checkSum = cksum(this, sizeof(PacketBase));
}

OpenPkt::OpenPkt(uint32_t GUID, int fd, uint32_t seqNum, uint32_t transNum,
                 char* fileName)
    : PacketBase(OPEN, CLIENT, GUID, fd, seqNum, transNum) {
  unsigned int i;
  for (i = 0; i < strlen(fileName) && i < MAX_FILE_NAME - 1; i++) {
    this->fileName[i] = fileName[i];
  }
  fileName[i] = '\0';
}

OpenAckPkt::OpenAckPkt(uint32_t GUID, int fd, uint32_t seqNum,
                       uint32_t transNum, bool status)
    : PacketBase(OPEN_ACK, SERVER, GUID, fd, seqNum, transNum) {
  this->status = status;
}

WriteBlockPkt::WriteBlockPkt(uint32_t GUID, int fd, uint32_t seqNum,
                             uint32_t transNum, int blockID, int offset,
                             int size, uint8_t* payload)
    : PacketBase(WRITE_BLOCK, CLIENT, GUID, fd, seqNum, transNum) {
  this->blockID = blockID;
  this->offset = offset;
  this->size = size;
  for (int i = 0; i < size; i++) {
    this->payload[i] = payload[i];
  }

}

CommitVotingPkt::CommitVotingPkt(uint32_t GUID, int fd, uint32_t seqNum,
                                 uint32_t transNum, int totalPending)
    : PacketBase(COMMIT_VOTING, CLIENT, GUID, fd, seqNum, transNum) {
  this->totalPending = totalPending;
}

CommitVotingSuccessPkt::CommitVotingSuccessPkt(uint32_t GUID, int fd,
                                               uint32_t seqNum,
                                               uint32_t transNum)
    : PacketBase(COMMIT_VOTING_SUCCESS, SERVER, GUID, fd, seqNum, transNum) {
}

CommitVotingResendPkt::CommitVotingResendPkt(uint32_t GUID, int fd,
                                             uint32_t seqNum, uint32_t transNum,
                                             int totalMissing, int* MissingIDs)
    : PacketBase(COMMIT_VOTING_RESEND, SERVER, GUID, fd, seqNum, transNum) {
  this->totalMissing = totalMissing;
  for (int i = 0; i < totalMissing; i++) {
    this->vectorMissingID.push_back(MissingIDs[i]);
  }
}

CommitFinalPkt::CommitFinalPkt(uint32_t GUID, int fd, uint32_t seqNum,
                               uint32_t transNum)
    : PacketBase(COMMIT_FINAL, CLIENT, GUID, fd, seqNum, transNum) {
}

CommitFinalReplyPkt::CommitFinalReplyPkt(uint32_t GUID, int fd, uint32_t seqNum,
                                         uint32_t transNum, bool status)
    : PacketBase(COMMIT_FINAL_REPLY, SERVER, GUID, fd, seqNum, transNum) {
  this->status = status;
}

AbortPkt::AbortPkt(uint32_t GUID, int fd, uint32_t seqNum, uint32_t transNum)
    : PacketBase(ABORT, CLIENT, GUID, fd, seqNum, transNum) {
}

ClosePkt::ClosePkt(uint32_t GUID, int fd, uint32_t seqNum, uint32_t transNum,
                   int totalPending)
    : PacketBase(CLOSE, CLIENT, GUID, fd, seqNum, transNum) {
  this->totalPending = totalPending;
}

void PacketBase::serialize(std::ostream& stream) const {
  uint16_t checkSum1 = htons(checkSum);
  uint32_t GUID1 = htonl(GUID);
  int fd1 = htonl(fd);
  uint32_t seqNum1 = htonl(seqNum);
  uint32_t transNum1 = htonl(transNum);
  stream.write(reinterpret_cast<const char *>(&type), sizeof(type));
  stream.write(reinterpret_cast<const char *>(&nodeType), sizeof(nodeType));
  stream.write(reinterpret_cast<const char *>(&checkSum1), sizeof(checkSum));
  stream.write(reinterpret_cast<const char *>(&GUID1), sizeof(GUID));
  stream.write(reinterpret_cast<const char *>(&fd1), sizeof(fd));
  stream.write(reinterpret_cast<const char *>(&seqNum1), sizeof(seqNum));
  stream.write(reinterpret_cast<const char *>(&transNum1), sizeof(transNum));
}

void PacketBase::deserialize(std::istream& stream) {
  stream.read(reinterpret_cast<char *>(&type), sizeof(type));
  stream.read(reinterpret_cast<char *>(&nodeType), sizeof(nodeType));
  stream.read(reinterpret_cast<char *>(&checkSum), sizeof(checkSum));
  stream.read(reinterpret_cast<char *>(&GUID), sizeof(GUID));
  stream.read(reinterpret_cast<char *>(&fd), sizeof(fd));
  stream.read(reinterpret_cast<char *>(&seqNum), sizeof(seqNum));
  stream.read(reinterpret_cast<char *>(&transNum), sizeof(transNum));
  checkSum = ntohs(checkSum);
  GUID = ntohl(GUID);
  fd = ntohl(fd);
  seqNum = ntohl(seqNum);
  transNum = ntohl(transNum);
}

void OpenPkt::serialize(std::ostream& stream) const {
  PacketBase::serialize(stream);
  stream.write(reinterpret_cast<const char *>(&fileName), sizeof(fileName));

}

void OpenPkt::deserialize(std::istream& stream) {
  PacketBase::deserialize(stream);
  //TODO
  stream.read(reinterpret_cast<char *>(&fileName), sizeof(fileName));

}

void OpenAckPkt::serialize(std::ostream& stream) const {
  PacketBase::serialize(stream);
  stream.write(reinterpret_cast<const char *>(&status), sizeof(status));

}

void OpenAckPkt::deserialize(std::istream& stream) {
  PacketBase::deserialize(stream);
  stream.read(reinterpret_cast<char *>(&status), sizeof(status));
}

void WriteBlockPkt::serialize(std::ostream& stream) const {
  PacketBase::serialize(stream);
  int blockID1 = htonl(blockID);
  int offset1 = htonl(offset);
  int size1 = htonl(size);
  stream.write(reinterpret_cast<const char *>(&blockID1), sizeof(blockID));
  stream.write(reinterpret_cast<const char *>(&offset1), sizeof(offset));
  stream.write(reinterpret_cast<const char *>(&size1), sizeof(size));
  stream.write(reinterpret_cast<const char *>(&payload),
               size * sizeof(uint8_t));
}

void WriteBlockPkt::deserialize(std::istream& stream) {
  PacketBase::deserialize(stream);
  stream.read(reinterpret_cast<char *>(&blockID), sizeof(blockID));
  stream.read(reinterpret_cast<char *>(&offset), sizeof(offset));
  stream.read(reinterpret_cast<char *>(&size), sizeof(size));
  stream.read(reinterpret_cast<char *>(&payload), size * sizeof(uint8_t));
  blockID = ntohl(blockID);
  offset = ntohl(offset);
  size = ntohl(size);
}

void CommitVotingPkt::serialize(std::ostream& stream) const {
  PacketBase::serialize(stream);
  int totalPending1 = htonl(totalPending);
  stream.write(reinterpret_cast<const char *>(&totalPending1),
               sizeof(totalPending));
}

void CommitVotingPkt::deserialize(std::istream& stream) {
  PacketBase::deserialize(stream);
  stream.read(reinterpret_cast<char *>(&totalPending), sizeof(totalPending));
  totalPending = ntohl(totalPending);
}

void CommitVotingSuccessPkt::serialize(std::ostream& stream) const {
  PacketBase::serialize(stream);
}

void CommitVotingSuccessPkt::deserialize(std::istream& stream) {
  PacketBase::deserialize(stream);
}

void CommitVotingResendPkt::serialize(std::ostream& stream) const {
  PacketBase::serialize(stream);
  int totalMissing1 = htonl(totalMissing);
  stream.write(reinterpret_cast<const char *>(&totalMissing1),
               sizeof(totalMissing));
  for (int i = 0; i < totalMissing; i++) {
    int missingID = htonl(vectorMissingID[i]);
    stream.write(reinterpret_cast<const char *>(&missingID), sizeof(int));
  }
}

void CommitVotingResendPkt::deserialize(std::istream& stream) {
  PacketBase::deserialize(stream);
  stream.read(reinterpret_cast<char *>(&totalMissing), sizeof(totalMissing));
  totalMissing = ntohl(totalMissing);
  int missingID = 0;
  for (int i = 0; i < totalMissing; i++) {
    stream.read(reinterpret_cast<char *>(&missingID), sizeof(int));
    missingID = ntohl(missingID);
    vectorMissingID.push_back(missingID);
  }
}

void CommitFinalPkt::serialize(std::ostream& stream) const {
  PacketBase::serialize(stream);
}

void CommitFinalPkt::deserialize(std::istream& stream) {
  PacketBase::deserialize(stream);
}

void CommitFinalReplyPkt::serialize(std::ostream& stream) const {
  PacketBase::serialize(stream);
  stream.write(reinterpret_cast<const char *>(&status), sizeof(status));
}

void CommitFinalReplyPkt::deserialize(std::istream& stream) {
  PacketBase::deserialize(stream);
  stream.read(reinterpret_cast<char *>(&status), sizeof(status));
}

void AbortPkt::serialize(std::ostream& stream) const {
  PacketBase::serialize(stream);
}

void AbortPkt::deserialize(std::istream& stream) {
  PacketBase::deserialize(stream);
}

void ClosePkt::serialize(std::ostream& stream) const {
  PacketBase::serialize(stream);
  stream.write(reinterpret_cast<const char *>(&totalPending),
               sizeof(totalPending));
}

void ClosePkt::deserialize(std::istream& stream) {
  PacketBase::deserialize(stream);
  stream.read(reinterpret_cast<char *>(&totalPending), sizeof(totalPending));
}

std::istream& operator>>(std::istream &stream, PacketBase *packet) {
  packet->deserialize(stream);
  return stream;
}

std::ostream& operator<<(std::ostream &stream, const PacketBase *packet) {
  packet->serialize(stream);
  return stream;
}

uint16_t PacketBase::cksum(const void* _data, int len) {
  const uint8_t *data = (const uint8_t*) _data;
  uint32_t sum;

  for (sum = 0; len >= 2; data += 2, len -= 2)
    sum += data[0] << 8 | data[1];
  if (len > 0)
    sum += data[0] << 8;
  while (sum > 0xffff)
    sum = (sum >> 16) + (sum & 0xffff);
  sum = htons(~sum);
  return sum ? sum : 0xffff;
}

bool PacketBase::checkSumCorrect() {
  return cksum(this, sizeof(PacketBase)) == 0xffff;
}

void PacketBase::printPacket() {
 std::string typeStr[] = { "OPEN", "OPEN_ACK", "WRITE_BLOCK", "COMMIT_VOTING",
      "COMMIT_VOTING_SUCCESS", "COMMIT_VOTING_RESEND", "COMMIT_FINAL",
      "COMMIT_FINAL_REPLY", "ABORT", "CLOSE" };
  DBG("\nPrint Packet: type=%s, nodeType=%d, GUID=%x, fd=%d, seqNum=%d, transNum=%d | ",
      typeStr[type].c_str(), nodeType, GUID, fd, seqNum, transNum);
}

void OpenPkt::printPacket() {
  PacketBase::printPacket();
  DBG("fileName=%s\n", fileName);
}

void OpenAckPkt::printPacket() {
  PacketBase::printPacket();
  DBG("status = %d\n", status);
}

void WriteBlockPkt::printPacket() {
  PacketBase::printPacket();
  DBG("blockID=%d, offset=%d, size=%d \n    ====== payload= ", blockID, offset, size);
  for (int i = 0; i < size; i++) {
    DBG("%c", payload[i]);
  }DBG("\n");
}

void CommitVotingPkt::printPacket() {
  PacketBase::printPacket();
  DBG("%d\n", totalPending);
}

void CommitVotingSuccessPkt::printPacket() {
  PacketBase::printPacket();
  DBG("\n");
}

void CommitVotingResendPkt::printPacket() {
  PacketBase::printPacket();
  DBG("totalMissing=%d \n missingID= ", totalMissing);
  for (int i = 0; i < totalMissing; i++) {
    DBG("%c", vectorMissingID[i]);
  }DBG("\n");
}

void CommitFinalPkt::printPacket() {
  PacketBase::printPacket();
  DBG("\n");
}

void CommitFinalReplyPkt::printPacket() {
  PacketBase::printPacket();
  DBG("status =%d \n", status);
}

void AbortPkt::printPacket() {
  PacketBase::printPacket();
  DBG("\n");
}

void ClosePkt::printPacket() {
  PacketBase::printPacket();
  DBG("totalPending=%d\n", totalPending);
}

