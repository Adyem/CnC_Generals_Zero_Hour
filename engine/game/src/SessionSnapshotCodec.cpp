#include "CncGame/SessionSnapshotCodec.hpp"

#include <limits>

#include "CncSimulation/PlayerRegistryCodec.hpp"
#include "CncSimulation/SpatialIndexCodec.hpp"
#include "CncSimulation/CombatRegistryCodec.hpp"
#include "errno.hpp"

namespace cnc
{
namespace
{
constexpr ft_size_t header_size = 20U;

void append_u32(std::vector<uint8_t> &bytes, uint32_t value)
{
    for (uint32_t shift = 0U; shift < 32U; shift += 8U)
        bytes.push_back(static_cast<uint8_t>((value >> shift) & 0xFFU));
}

uint32_t read_u32(const uint8_t *bytes)
{
    uint32_t value = 0U;
    for (uint32_t shift = 0U; shift < 32U; shift += 8U)
        value |= static_cast<uint32_t>(bytes[shift / 8U]) << shift;
    return value;
}

bool checked_total(ft_size_t world_size, ft_size_t players_size, ft_size_t spatial_size,
                   ft_size_t combat_size,
                   ft_size_t *total_out) noexcept
{
    if (total_out == nullptr || world_size > std::numeric_limits<uint32_t>::max() ||
        players_size > std::numeric_limits<uint32_t>::max() ||
        spatial_size > std::numeric_limits<uint32_t>::max() ||
        combat_size > std::numeric_limits<uint32_t>::max() ||
        world_size > std::numeric_limits<ft_size_t>::max() - header_size)
        return false;
    const ft_size_t with_world = header_size + world_size;
    if (players_size > std::numeric_limits<ft_size_t>::max() - with_world) return false;
    const ft_size_t with_players = with_world + players_size;
    if (spatial_size > std::numeric_limits<ft_size_t>::max() - with_players) return false;
    const ft_size_t with_spatial = with_players + spatial_size;
    if (combat_size > std::numeric_limits<ft_size_t>::max() - with_spatial) return false;
    *total_out = with_spatial + combat_size;
    return true;
}
}

Error SessionSnapshotCodec::encode(const SessionSnapshot &snapshot,
                                   std::vector<uint8_t> *bytes_out) noexcept
{
    if (bytes_out == nullptr) return FT_ERR_INVALID_POINTER;
    if (snapshot.schema_version != wire_schema_version ||
        snapshot.world.schema_version != WorldSnapshotCodec::wire_schema_version ||
        snapshot.players.schema_version != PlayerRegistryCodec::wire_schema_version ||
        snapshot.spatial.schema_version != SpatialIndexCodec::wire_schema_version ||
        snapshot.combat.schema_version != CombatRegistryCodec::wire_schema_version)
        return FT_ERR_INVALID_ARGUMENT;
    std::vector<uint8_t> world_bytes;
    std::vector<uint8_t> player_bytes;
    std::vector<uint8_t> spatial_bytes;
    std::vector<uint8_t> combat_bytes;
    Error error = WorldSnapshotCodec::encode(snapshot.world, &world_bytes);
    if (error != FT_ERR_SUCCESS) return error;
    error = PlayerRegistryCodec::encode(snapshot.players, &player_bytes);
    if (error != FT_ERR_SUCCESS) return error;
    error = SpatialIndexCodec::encode(snapshot.spatial, &spatial_bytes);
    if (error != FT_ERR_SUCCESS) return error;
    error = CombatRegistryCodec::encode(snapshot.combat, &combat_bytes);
    if (error != FT_ERR_SUCCESS) return error;
    ft_size_t total_size = 0U;
    if (!checked_total(static_cast<ft_size_t>(world_bytes.size()),
                       static_cast<ft_size_t>(player_bytes.size()),
                       static_cast<ft_size_t>(spatial_bytes.size()),
                       static_cast<ft_size_t>(combat_bytes.size()), &total_size))
        return FT_ERR_OUT_OF_RANGE;
    try
    {
        bytes_out->clear();
        bytes_out->reserve(static_cast<std::vector<uint8_t>::size_type>(total_size));
        append_u32(*bytes_out, wire_schema_version);
        append_u32(*bytes_out, static_cast<uint32_t>(world_bytes.size()));
        append_u32(*bytes_out, static_cast<uint32_t>(player_bytes.size()));
        append_u32(*bytes_out, static_cast<uint32_t>(spatial_bytes.size()));
        append_u32(*bytes_out, static_cast<uint32_t>(combat_bytes.size()));
        bytes_out->insert(bytes_out->end(), world_bytes.begin(), world_bytes.end());
        bytes_out->insert(bytes_out->end(), player_bytes.begin(), player_bytes.end());
        bytes_out->insert(bytes_out->end(), spatial_bytes.begin(), spatial_bytes.end());
        bytes_out->insert(bytes_out->end(), combat_bytes.begin(), combat_bytes.end());
    }
    catch (...)
    {
        bytes_out->clear();
        return FT_ERR_NO_MEMORY;
    }
    return FT_ERR_SUCCESS;
}

Error SessionSnapshotCodec::decode(const uint8_t *bytes, ft_size_t byte_count,
                                   SessionSnapshot *snapshot_out) noexcept
{
    if (bytes == nullptr || snapshot_out == nullptr) return FT_ERR_INVALID_POINTER;
    if (byte_count < header_size) return FT_ERR_INVALID_ARGUMENT;
    const uint32_t schema = read_u32(bytes);
    const uint32_t world_size = read_u32(bytes + 4U);
    const uint32_t players_size = read_u32(bytes + 8U);
    const uint32_t spatial_size = read_u32(bytes + 12U);
    const uint32_t combat_size = read_u32(bytes + 16U);
    ft_size_t expected_size = 0U;
    if (schema != wire_schema_version ||
        !checked_total(static_cast<ft_size_t>(world_size),
                       static_cast<ft_size_t>(players_size),
                       static_cast<ft_size_t>(spatial_size),
                       static_cast<ft_size_t>(combat_size), &expected_size) ||
        expected_size != byte_count)
        return FT_ERR_CONFIGURATION;
    WorldSnapshot world;
    PlayerRegistrySnapshot players;
    SpatialIndexSnapshot spatial;
    CombatRegistrySnapshot combat;
    Error error = WorldSnapshotCodec::decode(bytes + header_size, world_size, &world);
    if (error != FT_ERR_SUCCESS) return error;
    error = PlayerRegistryCodec::decode(bytes + header_size + world_size, players_size, &players);
    if (error != FT_ERR_SUCCESS) return error;
    error = SpatialIndexCodec::decode(bytes + header_size + world_size + players_size,
                                      spatial_size, &spatial);
    if (error != FT_ERR_SUCCESS) return error;
    error = CombatRegistryCodec::decode(bytes + header_size + world_size + players_size + spatial_size,
                                         combat_size, &combat);
    if (error != FT_ERR_SUCCESS) return error;
    snapshot_out->schema_version = wire_schema_version;
    snapshot_out->world.schema_version = world.schema_version;
    snapshot_out->world.tick = world.tick;
    snapshot_out->world.entries.swap(world.entries);
    snapshot_out->players.schema_version = players.schema_version;
    snapshot_out->players.players.swap(players.players);
    snapshot_out->players.teams.swap(players.teams);
    snapshot_out->players.team_memberships.swap(players.team_memberships);
    snapshot_out->players.relationships.swap(players.relationships);
    snapshot_out->players.ownership.swap(players.ownership);
    snapshot_out->spatial.schema_version = spatial.schema_version;
    snapshot_out->spatial.entries.swap(spatial.entries);
    snapshot_out->combat.schema_version = combat.schema_version;
    snapshot_out->combat.health.swap(combat.health);
    return FT_ERR_SUCCESS;
}
}
