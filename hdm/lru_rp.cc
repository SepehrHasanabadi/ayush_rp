/**
 * Copyright (c) 2018-2020 Inria
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met: redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer;
 * redistributions in binary form must reproduce the above copyright
 * notice, this list of conditions and the following disclaimer in the
 * documentation and/or other materials provided with the distribution;
 * neither the name of the copyright holders nor the names of its
 * contributors may be used to endorse or promote products derived from
 * this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "mem/cache/replacement_policies/lru_rp.hh"

#include <cassert>
#include <memory>

#include "params/LRURP.hh"
#include "sim/cur_tick.hh"

namespace gem5
{

namespace replacement_policy
{

LRU::LRU(const Params &p)
  : Base(p), sramCounter(0), WS(p.WS), stats(this)
{
}

LRU::LRUStats::LRUStats(statistics::Group *parent)
  : statistics::Group(parent),
    ADD_STAT(nvmWrite, statistics::units::Count::get(),
             "number of NVM blocks in a set"),
    ADD_STAT(writeCounter, statistics::units::Count::get(),
    "Write Counter"),
    ADD_STAT(nvmVictims, statistics::units::Count::get(),
    "Number of NVM victims")
{
}

void
LRU::invalidate(const std::shared_ptr<ReplacementData>& replacement_data)
{
    // Reset last touch timestamp
    std::static_pointer_cast<LRUReplData>(
        replacement_data)->lastTouchTick = Tick(0);
}

void
LRU::touch(const std::shared_ptr<ReplacementData>& replacement_data) const
{
    auto lru_data = std::static_pointer_cast<LRUReplData>(replacement_data);
    lru_data->lastTouchTick = curTick();
}

void
LRU::reset(const std::shared_ptr<ReplacementData>& replacement_data) const
{
    auto instance = std::static_pointer_cast<LRUReplData>(replacement_data);
    instance->lastTouchTick = curTick();
}

ReplaceableEntry*
LRU::getVictim(const ReplacementCandidates& candidates) const
{
    assert(candidates.size() > 0);

    ReplaceableEntry* victim = candidates[0];
    ReplaceableEntry* sramVictim = nullptr;

    for (const auto& candidate : candidates) {
        auto candidateData = std::static_pointer_cast<LRUReplData>(candidate->replacementData);
        auto victimData = std::static_pointer_cast<LRUReplData>(victim->replacementData);
        if (candidateData->lastTouchTick < victimData->lastTouchTick) {
            victim = candidate;
        }
        if (!candidateData->isNVM) {
            if (!sramVictim || candidateData->lastTouchTick <
             std::static_pointer_cast<LRUReplData>(sramVictim->replacementData)->lastTouchTick) {
                sramVictim = candidate;
            }
        }
    }
    auto victimData = std::static_pointer_cast<LRUReplData>(victim->replacementData);
    auto sramVictimData = std::static_pointer_cast<LRUReplData>(sramVictim->replacementData);
    if (victimData->isNVM && lastWay == victimData->way) {
      stats.writeCounter += 1;
      victimData->isNVM = false;
      sramVictimData->isNVM = true;
      auto way = victimData->way;
      victimData->way = sramVictimData->way;
      sramVictimData->way = way;
      lastWay = 0;
      return sramVictim;
    }
    if (victimData->isNVM) {
      stats.nvmVictims += 1;
    }
    lastWay = victimData->way;
    return victim;
}

std::shared_ptr<ReplacementData>
LRU::instantiateEntry()
{
    bool nvm = false;
    wayCounter += 1;
    if (sramCounter > WS) {
      stats.nvmWrite += 1;
      nvm = true;
    } else {
      sramCounter += 1;
    }
    LRUReplData* lruReplData = new LRUReplData(wayCounter, nvm);
    return std::shared_ptr<ReplacementData>(lruReplData);
}

} // namespace replacement_policy
} // namespace gem5
