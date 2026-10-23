#pragma once
#include "greeneye.h"
namespace teleport::protection {
// Candidate native adapter: must be verified in gameplay before prank activation.
bool Init(const GreeneyeHost* host);
bool Enable(void* storage,uint32_t entity);
bool Observed(void* storage,uint32_t entity);
void RestoreGlider(void* storage,uint32_t entity);
bool Grounded(void* storage,uint32_t entity);
bool Disable(void* storage,uint32_t entity);
bool Busy();
bool Faulted();
}
