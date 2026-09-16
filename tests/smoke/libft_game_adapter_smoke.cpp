#include "CncGame/LibftEventSchedulerAdapter.hpp"

int main()
{
    cnc::LibftEventSchedulerAdapter adapter;
    t_event_scheduler_profile profile{};
    if (adapter.snapshot_profile(&profile) != FT_ERR_NOT_INITIALISED ||
        adapter.initialize() != FT_ERR_SUCCESS ||
        adapter.snapshot_profile(&profile) != FT_ERR_SUCCESS ||
        adapter.pending_count() != 0U ||
        adapter.shutdown() != FT_ERR_SUCCESS ||
        adapter.shutdown() != FT_ERR_SUCCESS)
        return 1;
    return 0;
}
