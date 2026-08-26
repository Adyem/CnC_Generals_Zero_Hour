#include "ZeroHourData/GeneralRosterCodec.hpp"
#include <limits>
#include "errno.hpp"
namespace zero_hour { namespace {
constexpr cnc::Size H=8U, R=16U;
void w32(std::vector<uint8_t>& b,uint32_t v){for(uint32_t s=0;s<32U;s+=8U)b.push_back(static_cast<uint8_t>((v>>s)&255U));}
void w64(std::vector<uint8_t>& b,uint64_t v){for(uint32_t s=0;s<64U;s+=8U)b.push_back(static_cast<uint8_t>((v>>s)&255U));}
uint32_t r32(const uint8_t*p){uint32_t v=0;for(uint32_t s=0;s<32U;s+=8U)v|=static_cast<uint32_t>(p[s/8U])<<s;return v;}
uint64_t r64(const uint8_t*p){uint64_t v=0;for(uint32_t s=0;s<64U;s+=8U)v|=static_cast<uint64_t>(p[s/8U])<<s;return v;}
bool size(cnc::Size n,cnc::Size*out)noexcept{if(!out||n>GeneralRosterCodec::max_entries||n>(std::numeric_limits<cnc::Size>::max()-H)/R)return false;*out=H+n*R;return true;}
}}
namespace zero_hour {
cnc::Error GeneralRosterCodec::encode(const GeneralRoster::Snapshot&s,std::vector<uint8_t>*out)noexcept{if(!out)return FT_ERR_INVALID_POINTER;cnc::Size n=static_cast<cnc::Size>(s.bindings.size()),z;if(s.schema_version!=1U||!size(n,&z))return FT_ERR_INVALID_ARGUMENT;try{out->clear();out->reserve(z);w32(*out,1U);w32(*out,static_cast<uint32_t>(n));cnc::EntityId previous;for(const auto&b:s.bindings){if(!b.entity.is_valid()||b.general.value==0U||(previous.is_valid()&&previous.value>=b.entity.value)){out->clear();return FT_ERR_INVALID_ARGUMENT;}previous=b.entity;w64(*out,b.entity.value);w64(*out,b.general.value);}}catch(...){out->clear();return FT_ERR_NO_MEMORY;}return FT_ERR_SUCCESS;}
cnc::Error GeneralRosterCodec::decode(const uint8_t*p,cnc::Size bytes,GeneralRoster::Snapshot*out)noexcept{if(!p||!out)return FT_ERR_INVALID_POINTER;if(bytes<H)return FT_ERR_INVALID_ARGUMENT;uint32_t n=r32(p+4U);cnc::Size z;if(r32(p)!=1U||!size(static_cast<cnc::Size>(n),&z)||z!=bytes)return FT_ERR_CONFIGURATION;GeneralRoster::Snapshot d;try{d.bindings.reserve(n);for(uint32_t i=0;i<n;++i){cnc::Size o=H+static_cast<cnc::Size>(i)*R;cnc::EntityId e{r64(p+o)};cnc::DefinitionId g{r64(p+o+8U)};if(!e.is_valid()||g.value==0U||(i&&d.bindings.back().entity.value>=e.value))return FT_ERR_CONFIGURATION;d.bindings.push_back({e,g});}}catch(...){return FT_ERR_NO_MEMORY;}out->schema_version=1U;out->bindings.swap(d.bindings);return FT_ERR_SUCCESS;}
}
