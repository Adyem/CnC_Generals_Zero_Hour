#include "CncGame/SessionSnapshotCodec.hpp"

#include <limits>

#include "CncSimulation/PlayerRegistryCodec.hpp"
#include "CncSimulation/SpatialIndexCodec.hpp"
#include "CncSimulation/CombatRegistryCodec.hpp"
#include "CncSimulation/VisibilityRegistryCodec.hpp"
#include "ZeroHourData/PlayerStateRegistryCodec.hpp"
#include "ZeroHourData/GeneralRosterCodec.hpp"
#include "ZeroHourData/SpecialPowerLedgerCodec.hpp"
#include "errno.hpp"

namespace cnc
{
namespace
{
constexpr ft_size_t header_size = 36U;

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
                   ft_size_t player_states_size,
                   ft_size_t generals_size,
                   ft_size_t powers_size,
                   ft_size_t visibility_size,
                   ft_size_t *total_out) noexcept
{
    if (total_out == nullptr || world_size > std::numeric_limits<uint32_t>::max() ||
        players_size > std::numeric_limits<uint32_t>::max() ||
        spatial_size > std::numeric_limits<uint32_t>::max() ||
        combat_size > std::numeric_limits<uint32_t>::max() ||
        player_states_size > std::numeric_limits<uint32_t>::max() ||
        generals_size > std::numeric_limits<uint32_t>::max() ||
        powers_size > std::numeric_limits<uint32_t>::max() ||
        visibility_size > std::numeric_limits<uint32_t>::max() ||
        world_size > std::numeric_limits<ft_size_t>::max() - header_size)
        return false;
    const ft_size_t with_world = header_size + world_size;
    if (players_size > std::numeric_limits<ft_size_t>::max() - with_world) return false;
    const ft_size_t with_players = with_world + players_size;
    if (spatial_size > std::numeric_limits<ft_size_t>::max() - with_players) return false;
    const ft_size_t with_spatial = with_players + spatial_size;
    if (combat_size > std::numeric_limits<ft_size_t>::max() - with_spatial) return false;
    const ft_size_t with_combat = with_spatial + combat_size;
    if (player_states_size > std::numeric_limits<ft_size_t>::max() - with_combat) return false;
    const ft_size_t with_player_states = with_combat + player_states_size;
    if (generals_size > std::numeric_limits<ft_size_t>::max() - with_player_states) return false;
    const ft_size_t with_generals = with_player_states + generals_size;
    if (powers_size > std::numeric_limits<ft_size_t>::max() - with_generals) return false;
    const ft_size_t with_powers = with_generals + powers_size;
    if (visibility_size > std::numeric_limits<ft_size_t>::max() - with_powers) return false;
    *total_out = with_powers + visibility_size;
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
        snapshot.combat.schema_version != CombatRegistryCodec::wire_schema_version ||
        snapshot.player_states.schema_version != zero_hour::PlayerStateRegistryCodec::wire_schema_version ||
        snapshot.generals.schema_version != zero_hour::GeneralRosterCodec::wire_schema_version ||
        snapshot.powers.schema_version != zero_hour::SpecialPowerLedgerCodec::wire_schema_version ||
        snapshot.visibility.schema_version != VisibilityRegistryCodec::wire_schema_version)
        return FT_ERR_INVALID_ARGUMENT;
    std::vector<uint8_t> world_bytes;
    std::vector<uint8_t> player_bytes;
    std::vector<uint8_t> spatial_bytes;
    std::vector<uint8_t> combat_bytes;
    std::vector<uint8_t> player_state_bytes;
    std::vector<uint8_t> general_bytes;
    std::vector<uint8_t> power_bytes;
    std::vector<uint8_t> visibility_bytes;
    Error error = WorldSnapshotCodec::encode(snapshot.world, &world_bytes);
    if (error != FT_ERR_SUCCESS) return error;
    error = PlayerRegistryCodec::encode(snapshot.players, &player_bytes);
    if (error != FT_ERR_SUCCESS) return error;
    error = SpatialIndexCodec::encode(snapshot.spatial, &spatial_bytes);
    if (error != FT_ERR_SUCCESS) return error;
    error = CombatRegistryCodec::encode(snapshot.combat, &combat_bytes);
    if (error != FT_ERR_SUCCESS) return error;
    error = zero_hour::PlayerStateRegistryCodec::encode(snapshot.player_states, &player_state_bytes);
    if (error != FT_ERR_SUCCESS) return error;
    error = zero_hour::GeneralRosterCodec::encode(snapshot.generals, &general_bytes);
    if (error != FT_ERR_SUCCESS) return error;
    error = zero_hour::SpecialPowerLedgerCodec::encode(snapshot.powers, &power_bytes);
    if (error != FT_ERR_SUCCESS) return error;
    error = VisibilityRegistryCodec::encode(snapshot.visibility, &visibility_bytes);
    if (error != FT_ERR_SUCCESS) return error;
    ft_size_t total_size = 0U;
    if (!checked_total(static_cast<ft_size_t>(world_bytes.size()),
                       static_cast<ft_size_t>(player_bytes.size()),
                       static_cast<ft_size_t>(spatial_bytes.size()),
                       static_cast<ft_size_t>(combat_bytes.size()),
                       static_cast<ft_size_t>(player_state_bytes.size()),
                       static_cast<ft_size_t>(general_bytes.size()),
                       static_cast<ft_size_t>(power_bytes.size()),
                       static_cast<ft_size_t>(visibility_bytes.size()), &total_size))
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
        append_u32(*bytes_out, static_cast<uint32_t>(player_state_bytes.size()));
        append_u32(*bytes_out, static_cast<uint32_t>(general_bytes.size()));
        append_u32(*bytes_out, static_cast<uint32_t>(power_bytes.size()));
        append_u32(*bytes_out, static_cast<uint32_t>(visibility_bytes.size()));
        bytes_out->insert(bytes_out->end(), world_bytes.begin(), world_bytes.end());
        bytes_out->insert(bytes_out->end(), player_bytes.begin(), player_bytes.end());
        bytes_out->insert(bytes_out->end(), spatial_bytes.begin(), spatial_bytes.end());
        bytes_out->insert(bytes_out->end(), combat_bytes.begin(), combat_bytes.end());
        bytes_out->insert(bytes_out->end(), player_state_bytes.begin(), player_state_bytes.end());
        bytes_out->insert(bytes_out->end(), general_bytes.begin(), general_bytes.end());
        bytes_out->insert(bytes_out->end(), power_bytes.begin(), power_bytes.end());
        bytes_out->insert(bytes_out->end(), visibility_bytes.begin(), visibility_bytes.end());
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
    const uint32_t player_states_size = read_u32(bytes + 20U);
    const uint32_t generals_size = read_u32(bytes + 24U);
    const uint32_t powers_size = read_u32(bytes + 28U);
    const uint32_t visibility_size = read_u32(bytes + 32U);
    ft_size_t expected_size = 0U;
    if (schema != wire_schema_version ||
        !checked_total(static_cast<ft_size_t>(world_size),
                       static_cast<ft_size_t>(players_size),
                       static_cast<ft_size_t>(spatial_size),
                       static_cast<ft_size_t>(combat_size),
                       static_cast<ft_size_t>(player_states_size),
                       static_cast<ft_size_t>(generals_size),
                       static_cast<ft_size_t>(powers_size),
                       static_cast<ft_size_t>(visibility_size), &expected_size) ||
        expected_size != byte_count)
        return FT_ERR_CONFIGURATION;
    WorldSnapshot world;
    PlayerRegistrySnapshot players;
    SpatialIndexSnapshot spatial;
    CombatRegistrySnapshot combat;
    zero_hour::PlayerStateRegistry::Snapshot player_states;
    zero_hour::GeneralRoster::Snapshot generals;
    zero_hour::SpecialPowerLedger::Snapshot powers;
    VisibilitySnapshot visibility;
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
    error = zero_hour::PlayerStateRegistryCodec::decode(bytes + header_size + world_size + players_size + spatial_size + combat_size,
                                            player_states_size, &player_states);
    if (error != FT_ERR_SUCCESS) return error;
    error = zero_hour::GeneralRosterCodec::decode(bytes + header_size + world_size + players_size + spatial_size + combat_size + player_states_size,
                                            generals_size, &generals);
    if (error != FT_ERR_SUCCESS) return error;
    error = zero_hour::SpecialPowerLedgerCodec::decode(bytes + header_size + world_size + players_size + spatial_size + combat_size + player_states_size + generals_size,
                                            powers_size, &powers);
    if (error != FT_ERR_SUCCESS) return error;
    error = VisibilityRegistryCodec::decode(bytes + header_size + world_size + players_size + spatial_size + combat_size + player_states_size + generals_size + powers_size,
                                            visibility_size, &visibility);
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
    snapshot_out->player_states.schema_version = player_states.schema_version;
    snapshot_out->player_states.entries.swap(player_states.entries);
    snapshot_out->generals.schema_version = generals.schema_version;
    snapshot_out->generals.bindings.swap(generals.bindings);
    snapshot_out->powers.schema_version = powers.schema_version;
    snapshot_out->powers.cooldowns.swap(powers.cooldowns);
    snapshot_out->visibility.schema_version = visibility.schema_version;
    snapshot_out->visibility.records.swap(visibility.records);
    return FT_ERR_SUCCESS;
}
}
