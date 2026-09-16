#include "ZeroHourData/UnitRegistryCodec.hpp"
#include <limits>
#include "errno.hpp"
namespace zero_hour { namespace {
constexpr cnc::Size H=8U,R=16U,M=1U<<20U;
void w32(std::vector<uint8_t>&o,uint32_t v){for(uint32_t s=0U;s<32U;s+=8U)o.push_back(static_cast<uint8_t>((v>>s)&255U));}
void w64(std::vector<uint8_t>&o,uint64_t v){for(uint32_t s=0U;s<64U;s+=8U)o.push_back(static_cast<uint8_t>((v>>s)&255U));}
uint32_t r32(const uint8_t*p){uint32_t v=0U;for(uint32_t s=0U;s<32U;s+=8U)v|=static_cast<uint32_t>(p[s/8U])<<s;return v;}
uint64_t r64(const uint8_t*p){uint64_t v=0U;for(uint32_t s=0U;s<64U;s+=8U)v|=static_cast<uint64_t>(p[s/8U])<<s;return v;}
bool size(cnc::Size n,cnc::Size*o)noexcept{if(!o||n>M||n>static_cast<cnc::Size>(std::numeric_limits<uint32_t>::max())||n>(std::numeric_limits<cnc::Size>::max()-H)/R)return false;*o=H+n*R;return true;}
}}
namespace zero_hour {
cnc::Error UnitRegistryCodec::encode(const UnitRegistry::Snapshot&s,std::vector<uint8_t>*o)noexcept{if(!o)return FT_ERR_INVALID_POINTER;cnc::Size n=static_cast<cnc::Size>(s.bindings.size()),z;if(s.schema_version!=1U)return FT_ERR_INVALID_ARGUMENT;if(!size(n,&z))return FT_ERR_OUT_OF_RANGE;try{o->clear();o->reserve(z);w32(*o,1U);w32(*o,static_cast<uint32_t>(n));uint64_t p=0U;for(const auto&b:s.bindings){if(!b.entity.is_valid()||b.unit.value==0U||(p!=0U&&p>=b.entity.value)){o->clear();return FT_ERR_INVALID_ARGUMENT;}p=b.entity.value;w64(*o,b.entity.value);w64(*o,b.unit.value);}}catch(...){o->clear();return FT_ERR_NO_MEMORY;}return FT_ERR_SUCCESS;}
cnc::Error UnitRegistryCodec::decode(const uint8_t*p,cnc::Size n,UnitRegistry::Snapshot*o)noexcept{if(!p||!o)return FT_ERR_INVALID_POINTER;if(n<H)return FT_ERR_INVALID_ARGUMENT;uint32_t c=r32(p+4U);cnc::Size z;if(r32(p)!=1U||!size(static_cast<cnc::Size>(c),&z)||z!=n)return FT_ERR_CONFIGURATION;UnitRegistry::Snapshot d;try{d.bindings.reserve(c);for(uint32_t i=0U;i<c;++i){cnc::Size x=H+static_cast<cnc::Size>(i)*R;UnitRegistry::Binding b{cnc::EntityId{r64(p+x)},cnc::DefinitionId{r64(p+x+8U)}};if(!b.entity.is_valid()||b.unit.value==0U||(i&&d.bindings.back().entity.value>=b.entity.value))return FT_ERR_CONFIGURATION;d.bindings.push_back(b);}}catch(...){return FT_ERR_NO_MEMORY;}o->schema_version=1U;o->bindings.swap(d.bindings);return FT_ERR_SUCCESS;}
}
