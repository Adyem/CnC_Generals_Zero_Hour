#include "CncSimulation/ProductionQueue.hpp"

#include <algorithm>
#include <limits>
#include <utility>

#include "errno.hpp"

namespace cnc
{

Error ProductionQueue::initialize() noexcept
{
    if (_initialized == FT_TRUE) return FT_ERR_ALREADY_INITIALISED;
    _orders.clear();
    _next_sequence = 0U;
    _initialized = FT_TRUE;
    return FT_ERR_SUCCESS;
}

Error ProductionQueue::shutdown() noexcept
{
    if (_initialized != FT_TRUE) return FT_ERR_SUCCESS;
    _orders.clear();
    _next_sequence = 0U;
    _initialized = FT_FALSE;
    return FT_ERR_SUCCESS;
}

Error ProductionQueue::enqueue(EntityId producer, DefinitionId definition,
                                SimulationTick now, SimulationTick duration) noexcept
{
    if (_initialized != FT_TRUE) return FT_ERR_NOT_INITIALISED;
    if (!producer.is_valid() || definition.value == 0U) return FT_ERR_INVALID_ARGUMENT;
    if (duration.value > std::numeric_limits<SimulationTickValue>::max() - now.value)
        return FT_ERR_OUT_OF_RANGE;
    if (_next_sequence == std::numeric_limits<uint64_t>::max()) return FT_ERR_OUT_OF_RANGE;
    try
    {
        _orders.push_back(ProductionOrder{
            producer, definition, SimulationTick{now.value + duration.value}, _next_sequence++});
    }
    catch (...) { return FT_ERR_NO_MEMORY; }
    return FT_ERR_SUCCESS;
}

Error ProductionQueue::cancel(EntityId producer, uint64_t sequence) noexcept
{
    if (_initialized != FT_TRUE) return FT_ERR_NOT_INITIALISED;
    if (!producer.is_valid()) return FT_ERR_INVALID_ARGUMENT;
    for (auto iterator = _orders.begin(); iterator != _orders.end(); ++iterator)
        if (iterator->producer.value == producer.value && iterator->sequence == sequence)
        {
            _orders.erase(iterator);
            return FT_ERR_SUCCESS;
        }
    return FT_ERR_NOT_FOUND;
}

Error ProductionQueue::collect_ready(SimulationTick now,
                                     std::vector<ProductionOrder> *completed_out) noexcept
{
    if (completed_out == nullptr) return FT_ERR_INVALID_POINTER;
    if (_initialized != FT_TRUE) return FT_ERR_NOT_INITIALISED;
    try
    {
        std::vector<ProductionOrder> ordered = _orders;
        std::stable_sort(ordered.begin(), ordered.end(),
                         [](const ProductionOrder &first, const ProductionOrder &second)
                         { return first.sequence < second.sequence; });
        std::vector<ProductionOrder> completed;
        std::vector<ProductionOrder> remaining;
        completed.reserve(ordered.size());
        remaining.reserve(ordered.size());
        for (const ProductionOrder &order : ordered)
            (order.ready_at.value <= now.value ? completed : remaining).push_back(order);
        completed_out->swap(completed);
        _orders.swap(remaining);
    }
    catch (...)
    {
        completed_out->clear();
        return FT_ERR_NO_MEMORY;
    }
    return FT_ERR_SUCCESS;
}

Error ProductionQueue::discard() noexcept
{
    if (_initialized != FT_TRUE) return FT_ERR_NOT_INITIALISED;
    _orders.clear();
    _next_sequence = 0U;
    return FT_ERR_SUCCESS;
}

Size ProductionQueue::pending_count() const noexcept
{
    return static_cast<Size>(_orders.size());
}

Error ProductionQueue::export_snapshot(Snapshot *out) const noexcept
{
    if (out == nullptr) return FT_ERR_INVALID_POINTER;
    if (_initialized != FT_TRUE) return FT_ERR_NOT_INITIALISED;
    try { out->schema_version=1U; out->orders=_orders; std::stable_sort(out->orders.begin(), out->orders.end(), [](const ProductionOrder&a,const ProductionOrder&b) noexcept{return a.sequence<b.sequence;}); out->next_sequence=_next_sequence; }
    catch (...) { out->orders.clear(); return FT_ERR_NO_MEMORY; }
    return FT_ERR_SUCCESS;
}
Error ProductionQueue::import_snapshot(const Snapshot &snapshot) noexcept
{
    if (_initialized != FT_TRUE) return FT_ERR_NOT_INITIALISED;
    if (snapshot.schema_version != 1U || snapshot.orders.size() > (1U<<20U)) return FT_ERR_CONFIGURATION;
    std::vector<ProductionOrder> restored;
    try {
        restored.reserve(snapshot.orders.size());
        for (Size i=0U;i<snapshot.orders.size();++i) {
            const ProductionOrder &o=snapshot.orders[i];
            if (!o.producer.is_valid() || o.definition.value==0U ||
                o.sequence >= snapshot.next_sequence ||
                (i!=0U && snapshot.orders[i-1U].sequence>=o.sequence)) return FT_ERR_CONFIGURATION;
            restored.push_back(o);
        }
    } catch (...) { return FT_ERR_NO_MEMORY; }
    _orders.swap(restored); _next_sequence=snapshot.next_sequence; return FT_ERR_SUCCESS;
}
void ProductionQueue::swap(ProductionQueue &other) noexcept { _orders.swap(other._orders); std::swap(_next_sequence,other._next_sequence); std::swap(_initialized,other._initialized); }

}
