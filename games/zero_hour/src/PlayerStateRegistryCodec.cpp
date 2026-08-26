#include "ZeroHourData/PlayerStateRegistryCodec.hpp"

#include <limits>

#include "errno.hpp"

namespace zero_hour
{
namespace
{
constexpr cnc::Size header_size = 8U;
constexpr cnc::Size entry_size = 20U;
void append_u32(std::vector<uint8_t> &bytes, uint32_t value)
{ for (uint32_t shift = 0U; shift < 32U; shift += 8U) bytes.push_back(static_cast<uint8_t>((value >> shift) & 0xFFU)); }
void append_u64(std::vector<uint8_t> &bytes, uint64_t value)
{ for (uint32_t shift = 0U; shift < 64U; shift += 8U) bytes.push_back(static_cast<uint8_t>((value >> shift) & 0xFFU)); }
uint32_t read_u32(const uint8_t *bytes)
{ uint32_t value = 0U; for (uint32_t shift = 0U; shift < 32U; shift += 8U) value |= static_cast<uint32_t>(bytes[shift / 8U]) << shift; return value; }
uint64_t read_u64(const uint8_t *bytes)
{ uint64_t value = 0U; for (uint32_t shift = 0U; shift < 64U; shift += 8U) value |= static_cast<uint64_t>(bytes[shift / 8U]) << shift; return value; }
bool checked_size(cnc::Size count, cnc::Size *out) noexcept
{ if (out == nullptr || count > PlayerStateRegistryCodec::max_entries || count > (std::numeric_limits<cnc::Size>::max() - header_size) / entry_size) return false; *out = header_size + count * entry_size; return true; }
}
cnc::Error PlayerStateRegistryCodec::encode(const PlayerStateRegistry::Snapshot &snapshot,
                                            std::vector<uint8_t> *bytes_out) noexcept
{
    if (bytes_out == nullptr) return FT_ERR_INVALID_POINTER;
    const cnc::Size count = static_cast<cnc::Size>(snapshot.entries.size());
    cnc::Size wire_size = 0U;
    if (snapshot.schema_version != wire_schema_version || !checked_size(count, &wire_size)) return FT_ERR_INVALID_ARGUMENT;
    try
    {
        bytes_out->clear(); bytes_out->reserve(static_cast<std::vector<uint8_t>::size_type>(wire_size));
        append_u32(*bytes_out, wire_schema_version); append_u32(*bytes_out, static_cast<uint32_t>(count));
        for (const PlayerStateRegistry::SnapshotEntry &entry : snapshot.entries)
        {
            if (!entry.player.is_valid()) { bytes_out->clear(); return FT_ERR_INVALID_ARGUMENT; }
            append_u32(*bytes_out, entry.player.value); append_u64(*bytes_out, entry.faction.value); append_u32(*bytes_out, entry.science_points); append_u32(*bytes_out, 0U);
        }
    }
    catch (...) { bytes_out->clear(); return FT_ERR_NO_MEMORY; }
    return FT_ERR_SUCCESS;
}
cnc::Error PlayerStateRegistryCodec::decode(const uint8_t *bytes, cnc::Size byte_count,
                                            PlayerStateRegistry::Snapshot *snapshot_out) noexcept
{
    if (bytes == nullptr || snapshot_out == nullptr) return FT_ERR_INVALID_POINTER;
    if (byte_count < header_size) return FT_ERR_INVALID_ARGUMENT;
    const uint32_t schema = read_u32(bytes); const uint32_t count = read_u32(bytes + 4U);
    cnc::Size expected = 0U;
    if (schema != wire_schema_version || !checked_size(static_cast<cnc::Size>(count), &expected) || expected != byte_count) return FT_ERR_CONFIGURATION;
    PlayerStateRegistry::Snapshot decoded;
    try
    {
        decoded.schema_version = wire_schema_version; decoded.entries.reserve(count);
        for (uint32_t index = 0U; index < count; ++index)
        {
            const cnc::Size offset = header_size + static_cast<cnc::Size>(index) * entry_size;
            const cnc::PlayerId player{read_u32(bytes + offset)};
            if (!player.is_valid() ||
                (index != 0U && decoded.entries.back().player.value >= player.value) ||
                read_u32(bytes + offset + 16U) != 0U)
                return FT_ERR_CONFIGURATION;
            decoded.entries.push_back(PlayerStateRegistry::SnapshotEntry{
                player, cnc::DefinitionId{read_u64(bytes + offset + 4U)}, read_u32(bytes + offset + 12U)});
        }
    }
    catch (...) { return FT_ERR_NO_MEMORY; }
    snapshot_out->schema_version = decoded.schema_version; snapshot_out->entries.swap(decoded.entries);
    return FT_ERR_SUCCESS;
}
}
