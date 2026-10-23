#pragma once
#include "countdown.h"
#include "status.h"
#include "greeneye.h"
namespace teleport::travel {
enum class Outcome { none, pending, arrived, failed };
struct Result {uint32_t entity{};uint64_t nonce{};Destination destination{};Outcome outcome{};uint64_t account{};};
bool Init(const GreeneyeHost* host);
Result Latest(uint32_t entity);
void TrackRemote(uint64_t nonce,uint64_t account);
Status RemoteStatus();
Status LocalStatus(uint32_t entity);
unsigned ClientProtectionAck();
// True means the request was queued on this process's authoritative world.
bool Local(uint32_t entity, Destination position);
enum class Submission { remote, queued, rejected };
Submission SubmitLocal(uint32_t entity,Destination position);
void SetClient(void* client);
bool Prepare();
void Resume();
}
