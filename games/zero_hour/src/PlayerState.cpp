#include "ZeroHourData/PlayerState.hpp"

#include "errno.hpp"

namespace zero_hour
{

cnc::Error PlayerState::initialize(const Catalog *catalog, ScienceLedger *science,
                                   SpecialPowerLedger *powers, GeneralRoster *generals) noexcept
{
    if (_initialized) return FT_ERR_ALREADY_INITIALISED;
    if (catalog == nullptr || science == nullptr || powers == nullptr || generals == nullptr)
        return FT_ERR_INVALID_POINTER;
    _catalog = catalog;
    _science = science;
    _powers = powers;
    _generals = generals;
    _faction = cnc::DefinitionId{};
    _commander = cnc::EntityId{};
    _science_points = 0U;
    _initialized = true;
    return FT_ERR_SUCCESS;
}

cnc::Error PlayerState::set_faction(cnc::DefinitionId faction) noexcept
{
    if (!_initialized) return FT_ERR_INVALID_STATE;
    if (_catalog->find_faction(faction) == nullptr) return FT_ERR_NOT_FOUND;
    if (_faction.value != 0U && _faction.value != faction.value)
        return FT_ERR_INVALID_OPERATION;
    _faction = faction;
    return FT_ERR_SUCCESS;
}

cnc::Error PlayerState::set_science_points(uint32_t points) noexcept
{
    if (!_initialized) return FT_ERR_INVALID_STATE;
    _science_points = points;
    return FT_ERR_SUCCESS;
}

cnc::Error PlayerState::purchase_science(cnc::DefinitionId science) noexcept
{
    if (!_initialized) return FT_ERR_INVALID_STATE;
    if (_faction.value == 0U) return FT_ERR_INVALID_STATE;
    return _science->purchase(_faction, science, _science_points, &_science_points);
}

cnc::Error PlayerState::assign_general(cnc::EntityId entity, cnc::DefinitionId general) noexcept
{
    if (!_initialized) return FT_ERR_INVALID_STATE;
    if (_faction.value == 0U) return FT_ERR_INVALID_STATE;
    const GeneralDefinition *definition = _catalog->find_general(general);
    if (definition == nullptr) return FT_ERR_NOT_FOUND;
    if (definition->faction.value != _faction.value) return FT_ERR_PERMISSION_DENIED;
    if (_commander.is_valid() && _commander.value != entity.value)
        return FT_ERR_INVALID_OPERATION;
    const cnc::Error error = _generals->assign(entity, general);
    if (error == FT_ERR_SUCCESS) _commander = entity;
    return error;
}

cnc::Error PlayerState::activate_power(cnc::DefinitionId power, cnc::SimulationTick now,
                                       cnc::SimulationTick *ready_at) noexcept
{
    if (!_initialized) return FT_ERR_INVALID_STATE;
    if (_faction.value == 0U) return FT_ERR_INVALID_STATE;
    if (!_commander.is_valid()) return FT_ERR_INVALID_STATE;
    cnc::DefinitionId assigned_general;
    if (_generals->find(_commander, &assigned_general) != FT_ERR_SUCCESS)
        return FT_ERR_INVALID_STATE;
    const GeneralDefinition *general = _catalog->find_general(assigned_general);
    const SpecialPowerDefinition *definition = _catalog->find_special_power(power);
    if (general == nullptr || definition == nullptr) return FT_ERR_NOT_FOUND;
    if (general->signature_power.value != definition->id.value)
        return FT_ERR_PERMISSION_DENIED;
    return _powers->activate(power, now, ready_at);
}

cnc::DefinitionId PlayerState::faction() const noexcept { return _faction; }
uint32_t PlayerState::science_points() const noexcept { return _science_points; }

cnc::Error PlayerState::shutdown() noexcept
{
    _catalog = nullptr;
    _science = nullptr;
    _powers = nullptr;
    _generals = nullptr;
    _faction = cnc::DefinitionId{};
    _commander = cnc::EntityId{};
    _science_points = 0U;
    _initialized = false;
    return FT_ERR_SUCCESS;
}

}
