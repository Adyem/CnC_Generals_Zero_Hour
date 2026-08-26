#include "CncSimulation/PlayerRegistryCodec.hpp"

#include <limits>

#include "errno.hpp"

namespace cnc
{
namespace
{
constexpr ft_size_t header_size = 24U;
constexpr ft_size_t player_size = 4U;
constexpr ft_size_t team_size = 4U;
constexpr ft_size_t membership_size = 8U;
constexpr ft_size_t relationship_size = 9U;
constexpr ft_size_t ownership_size = 12U;

void append_u32(std::vector<uint8_t> &bytes, uint32_t value)
{
    for (uint32_t shift = 0U; shift < 32U; shift += 8U)
        bytes.push_back(static_cast<uint8_t>((value >> shift) & 0xFFU));
}

void append_u64(std::vector<uint8_t> &bytes, uint64_t value)
{
    for (uint32_t shift = 0U; shift < 64U; shift += 8U)
        bytes.push_back(static_cast<uint8_t>((value >> shift) & 0xFFU));
}

uint32_t read_u32(const uint8_t *bytes)
{
    uint32_t value = 0U;
    for (uint32_t shift = 0U; shift < 32U; shift += 8U)
        value |= static_cast<uint32_t>(bytes[shift / 8U]) << shift;
    return value;
}

uint64_t read_u64(const uint8_t *bytes)
{
    uint64_t value = 0U;
    for (uint32_t shift = 0U; shift < 64U; shift += 8U)
        value |= static_cast<uint64_t>(bytes[shift / 8U]) << shift;
    return value;
}

bool checked_add(ft_size_t left, ft_size_t right, ft_size_t *out) noexcept
{
    if (out == nullptr || left > std::numeric_limits<ft_size_t>::max() - right)
        return false;
    *out = left + right;
    return true;
}

bool checked_size(const PlayerRegistrySnapshot &snapshot, ft_size_t *size_out) noexcept
{
    if (size_out == nullptr) return false;
    const ft_size_t counts[] = {
        static_cast<ft_size_t>(snapshot.players.size()),
        static_cast<ft_size_t>(snapshot.teams.size()),
        static_cast<ft_size_t>(snapshot.team_memberships.size()),
        static_cast<ft_size_t>(snapshot.relationships.size()),
        static_cast<ft_size_t>(snapshot.ownership.size())};
    const ft_size_t widths[] = {player_size, team_size, membership_size,
                                relationship_size, ownership_size};
    ft_size_t result = header_size;
    for (uint32_t index = 0U; index < 5U; ++index)
    {
        if (counts[index] > PlayerRegistryCodec::max_records ||
            counts[index] > (std::numeric_limits<ft_size_t>::max() - result) / widths[index])
            return false;
        result += counts[index] * widths[index];
    }
    *size_out = result;
    return true;
}
}

Error PlayerRegistryCodec::encode(const PlayerRegistrySnapshot &snapshot,
                                  std::vector<uint8_t> *bytes_out) noexcept
{
    if (bytes_out == nullptr) return FT_ERR_INVALID_POINTER;
    ft_size_t wire_size = 0U;
    if (snapshot.schema_version != wire_schema_version || !checked_size(snapshot, &wire_size))
        return FT_ERR_INVALID_ARGUMENT;
    try
    {
        bytes_out->clear();
        bytes_out->reserve(static_cast<std::vector<uint8_t>::size_type>(wire_size));
        append_u32(*bytes_out, wire_schema_version);
        append_u32(*bytes_out, static_cast<uint32_t>(snapshot.players.size()));
        append_u32(*bytes_out, static_cast<uint32_t>(snapshot.teams.size()));
        append_u32(*bytes_out, static_cast<uint32_t>(snapshot.team_memberships.size()));
        append_u32(*bytes_out, static_cast<uint32_t>(snapshot.relationships.size()));
        append_u32(*bytes_out, static_cast<uint32_t>(snapshot.ownership.size()));
        for (const PlayerId player : snapshot.players) append_u32(*bytes_out, player.value);
        for (const TeamId team : snapshot.teams) append_u32(*bytes_out, team.value);
        for (const TeamMembership &membership : snapshot.team_memberships)
        {
            append_u32(*bytes_out, membership.player.value);
            append_u32(*bytes_out, membership.team.value);
        }
        for (const PlayerRelationship &relationship : snapshot.relationships)
        {
            append_u32(*bytes_out, relationship.first.value);
            append_u32(*bytes_out, relationship.second.value);
            bytes_out->push_back(static_cast<uint8_t>(relationship.value));
        }
        for (const PlayerOwnership &ownership : snapshot.ownership)
        {
            append_u64(*bytes_out, ownership.entity.value);
            append_u32(*bytes_out, ownership.owner.value);
        }
    }
    catch (...)
    {
        bytes_out->clear();
        return FT_ERR_NO_MEMORY;
    }
    return FT_ERR_SUCCESS;
}

Error PlayerRegistryCodec::decode(const uint8_t *bytes, ft_size_t byte_count,
                                  PlayerRegistrySnapshot *snapshot_out) noexcept
{
    if (bytes == nullptr || snapshot_out == nullptr) return FT_ERR_INVALID_POINTER;
    snapshot_out->players.clear();
    snapshot_out->teams.clear();
    snapshot_out->team_memberships.clear();
    snapshot_out->relationships.clear();
    snapshot_out->ownership.clear();
    if (byte_count < header_size) return FT_ERR_INVALID_ARGUMENT;
    const uint32_t schema = read_u32(bytes);
    const uint32_t counts[] = {read_u32(bytes + 4U), read_u32(bytes + 8U),
                               read_u32(bytes + 12U), read_u32(bytes + 16U),
                               read_u32(bytes + 20U)};
    const ft_size_t widths[] = {player_size, team_size, membership_size,
                                relationship_size, ownership_size};
    if (schema != wire_schema_version) return FT_ERR_CONFIGURATION;
    ft_size_t expected_size = header_size;
    for (uint32_t index = 0U; index < 5U; ++index)
    {
        if (counts[index] > max_records ||
            !checked_add(expected_size, static_cast<ft_size_t>(counts[index]) * widths[index],
                         &expected_size))
            return FT_ERR_CONFIGURATION;
    }
    if (expected_size != byte_count) return FT_ERR_CONFIGURATION;
    try
    {
        snapshot_out->schema_version = wire_schema_version;
        ft_size_t offset = header_size;
        snapshot_out->players.reserve(counts[0]);
        for (uint32_t index = 0U; index < counts[0]; ++index, offset += player_size)
            snapshot_out->players.push_back(PlayerId{read_u32(bytes + offset)});
        snapshot_out->teams.reserve(counts[1]);
        for (uint32_t index = 0U; index < counts[1]; ++index, offset += team_size)
            snapshot_out->teams.push_back(TeamId{read_u32(bytes + offset)});
        snapshot_out->team_memberships.reserve(counts[2]);
        for (uint32_t index = 0U; index < counts[2]; ++index, offset += membership_size)
            snapshot_out->team_memberships.push_back(TeamMembership{
                PlayerId{read_u32(bytes + offset)}, TeamId{read_u32(bytes + offset + 4U)}});
        snapshot_out->relationships.reserve(counts[3]);
        for (uint32_t index = 0U; index < counts[3]; ++index, offset += relationship_size)
        {
            const uint8_t diplomacy = bytes[offset + 8U];
            if (diplomacy > static_cast<uint8_t>(Diplomacy::hostile))
            {
                snapshot_out->players.clear();
                snapshot_out->teams.clear();
                snapshot_out->team_memberships.clear();
                snapshot_out->relationships.clear();
                snapshot_out->ownership.clear();
                return FT_ERR_CONFIGURATION;
            }
            snapshot_out->relationships.push_back(PlayerRelationship{
                PlayerId{read_u32(bytes + offset)}, PlayerId{read_u32(bytes + offset + 4U)},
                static_cast<Diplomacy>(diplomacy)});
        }
        snapshot_out->ownership.reserve(counts[4]);
        for (uint32_t index = 0U; index < counts[4]; ++index, offset += ownership_size)
            snapshot_out->ownership.push_back(PlayerOwnership{
                EntityId{read_u64(bytes + offset)}, PlayerId{read_u32(bytes + offset + 8U)}});
    }
    catch (...)
    {
        snapshot_out->players.clear();
        snapshot_out->teams.clear();
        snapshot_out->team_memberships.clear();
        snapshot_out->relationships.clear();
        snapshot_out->ownership.clear();
        return FT_ERR_NO_MEMORY;
    }
    return FT_ERR_SUCCESS;
}
}
