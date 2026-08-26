#pragma once
#include <cstdint>
#include <vector>
#include "ZeroHourData/ScienceLedger.hpp"
namespace zero_hour { class ScienceLedgerCodec final { public: static constexpr uint32_t wire_schema_version=1U; static constexpr cnc::Size max_entries=static_cast<cnc::Size>(1U<<20U); static cnc::Error encode(const ScienceLedger::Snapshot&,std::vector<uint8_t>*) noexcept; static cnc::Error decode(const uint8_t*,cnc::Size,ScienceLedger::Snapshot*) noexcept; }; }
