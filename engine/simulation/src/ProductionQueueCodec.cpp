#include "CncSimulation/ProductionQueueCodec.hpp"
#include <limits>
#include "errno.hpp"
namespace cnc { namespace {
constexpr Size H=12U,R=32U;
void w32(std::vector<uint8_t>&o,uint32_t v){for(uint32_t s=0U;s<32U;s+=8U)o.push_back(static_cast<uint8_t>((v>>s)&255U));}
void w64(std::vector<uint8_t>&o,uint64_t v){for(uint32_t s=0U;s<64U;s+=8U)o.push_back(static_cast<uint8_t>((v>>s)&255U));}
uint32_t r32(const uint8_t*p){uint32_t v=0U;for(uint32_t s=0U;s<32U;s+=8U)v|=static_cast<uint32_t>(p[s/8U])<<s;return v;}
uint64_t r64(const uint8_t*p){uint64_t v=0U;for(uint32_t s=0U;s<64U;s+=8U)v|=static_cast<uint64_t>(p[s/8U])<<s;return v;}
bool size(Size n,Size*o)noexcept{if(!o||n>ProductionQueueCodec::max_entries||n>(std::numeric_limits<Size>::max()-H)/R)return false;*o=H+n*R;return true;}
bool valid(const ProductionOrder&o,uint64_t next)noexcept{return o.producer.is_valid()&&o.definition.value!=0U&&o.sequence<next;}
}}
namespace cnc {
Error ProductionQueueCodec::encode(const ProductionQueue::Snapshot&s,std::vector<uint8_t>*o)noexcept{if(!o)return FT_ERR_INVALID_POINTER;Size n=static_cast<Size>(s.orders.size()),z;if(s.schema_version!=wire_schema_version||!size(n,&z))return FT_ERR_INVALID_ARGUMENT;try{o->clear();o->reserve(z);w32(*o,wire_schema_version);w32(*o,static_cast<uint32_t>(n));w64(*o,s.next_sequence);for(Size i=0U;i<n;++i){const auto&e=s.orders[i];if(!valid(e,s.next_sequence)||(i&&s.orders[i-1U].sequence>=e.sequence)){o->clear();return FT_ERR_INVALID_ARGUMENT;}w64(*o,e.producer.value);w64(*o,e.definition.value);w64(*o,e.ready_at.value);w64(*o,e.sequence);}}catch(...){o->clear();return FT_ERR_NO_MEMORY;}return FT_ERR_SUCCESS;}
Error ProductionQueueCodec::decode(const uint8_t*p,Size n,ProductionQueue::Snapshot*o)noexcept{if(!p||!o)return FT_ERR_INVALID_POINTER;if(n<H)return FT_ERR_INVALID_ARGUMENT;uint32_t c=r32(p+4U);Size z;if(r32(p)!=wire_schema_version||!size(static_cast<Size>(c),&z)||z!=n)return FT_ERR_CONFIGURATION;ProductionQueue::Snapshot d;d.next_sequence=r64(p+8U);try{d.orders.reserve(c);for(uint32_t i=0U;i<c;++i){Size x=H+static_cast<Size>(i)*R;ProductionOrder e{EntityId{r64(p+x)},DefinitionId{r64(p+x+8U)},SimulationTick{r64(p+x+16U)},r64(p+x+24U)};if(!valid(e,d.next_sequence)||(i&&d.orders.back().sequence>=e.sequence))return FT_ERR_CONFIGURATION;d.orders.push_back(e);}}catch(...){return FT_ERR_NO_MEMORY;}o->schema_version=wire_schema_version;o->next_sequence=d.next_sequence;o->orders.swap(d.orders);return FT_ERR_SUCCESS;}
}
