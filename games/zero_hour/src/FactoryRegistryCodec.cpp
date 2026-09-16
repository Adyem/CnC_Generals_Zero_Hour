#include "ZeroHourData/FactoryRegistryCodec.hpp"

#include <limits>

#include "errno.hpp"

namespace zero_hour
{
namespace
{
constexpr cnc::Size header_size = 8U;
constexpr cnc::Size record_size = 16U;
constexpr cnc::Size max_entries = 1U << 20U;
void write_u32(std::vector<uint8_t> &out, uint32_t value) { for (uint32_t s=0U;s<32U;s+=8U) out.push_back(static_cast<uint8_t>((value>>s)&0xFFU)); }
void write_u64(std::vector<uint8_t> &out, uint64_t value) { for (uint32_t s=0U;s<64U;s+=8U) out.push_back(static_cast<uint8_t>((value>>s)&0xFFU)); }
uint32_t read_u32(const uint8_t *p) { uint32_t v=0U; for (uint32_t s=0U;s<32U;s+=8U) v|=static_cast<uint32_t>(p[s/8U])<<s; return v; }
uint64_t read_u64(const uint8_t *p) { uint64_t v=0U; for (uint32_t s=0U;s<64U;s+=8U) v|=static_cast<uint64_t>(p[s/8U])<<s; return v; }
bool checked_size(cnc::Size count, cnc::Size *size_out) noexcept
{
    if (size_out == nullptr || count > max_entries ||
        count > static_cast<cnc::Size>(std::numeric_limits<uint32_t>::max()) ||
        count > (std::numeric_limits<cnc::Size>::max() - header_size) / record_size)
        return false;
    *size_out = header_size + count * record_size;
    return true;
}
}

cnc::Error FactoryRegistryCodec::encode(const FactoryRegistry::Snapshot &snapshot,
                                        std::vector<uint8_t> *bytes_out) noexcept
{
    if (bytes_out == nullptr) return FT_ERR_INVALID_POINTER;
    const cnc::Size count = static_cast<cnc::Size>(snapshot.bindings.size());
    cnc::Size total = 0U;
    if (snapshot.schema_version != wire_schema_version)
        return FT_ERR_INVALID_ARGUMENT;
    if (!checked_size(count, &total)) return FT_ERR_OUT_OF_RANGE;
    try
    {
        bytes_out->clear(); bytes_out->reserve(total);
        write_u32(*bytes_out, wire_schema_version); write_u32(*bytes_out, static_cast<uint32_t>(count));
        uint64_t previous = 0U;
        for (const auto &binding : snapshot.bindings)
        {
            if (!binding.entity.is_valid() || binding.factory.value == 0U ||
                (previous != 0U && previous >= binding.entity.value))
            { bytes_out->clear(); return FT_ERR_INVALID_ARGUMENT; }
            previous = binding.entity.value;
            write_u64(*bytes_out, binding.entity.value); write_u64(*bytes_out, binding.factory.value);
        }
    }
    catch (...) { bytes_out->clear(); return FT_ERR_NO_MEMORY; }
    return FT_ERR_SUCCESS;
}

cnc::Error FactoryRegistryCodec::decode(const uint8_t *bytes, cnc::Size byte_count,
                                        FactoryRegistry::Snapshot *snapshot_out) noexcept
{
    if (bytes == nullptr || snapshot_out == nullptr) return FT_ERR_INVALID_POINTER;
    if (byte_count < header_size) return FT_ERR_INVALID_ARGUMENT;
    const uint32_t schema = read_u32(bytes);
    const cnc::Size count = static_cast<cnc::Size>(read_u32(bytes + 4U));
    cnc::Size expected = 0U;
    if (schema != wire_schema_version || !checked_size(count, &expected) || expected != byte_count)
        return FT_ERR_CONFIGURATION;
    FactoryRegistry::Snapshot decoded;
    try
    {
        decoded.bindings.reserve(count);
        for (cnc::Size i=0U;i<count;++i)
        {
            const cnc::Size offset = header_size + i * record_size;
            const FactoryRegistry::Binding binding{cnc::EntityId{read_u64(bytes + offset)},
                                                   cnc::DefinitionId{read_u64(bytes + offset + 8U)}};
            if (!binding.entity.is_valid() || binding.factory.value == 0U ||
                (i != 0U && decoded.bindings.back().entity.value >= binding.entity.value))
                return FT_ERR_CONFIGURATION;
            decoded.bindings.push_back(binding);
        }
    }
    catch (...) { return FT_ERR_NO_MEMORY; }
    snapshot_out->schema_version = wire_schema_version;
    snapshot_out->bindings.swap(decoded.bindings);
    return FT_ERR_SUCCESS;
}
}
