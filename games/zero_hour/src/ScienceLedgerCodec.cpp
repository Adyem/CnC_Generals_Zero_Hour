#include "ZeroHourData/ScienceLedgerCodec.hpp"
#include <limits>
#include "errno.hpp"
namespace zero_hour { namespace {
constexpr cnc::Size H=8U,R=8U;
void write_u32(std::vector<uint8_t>&out,uint32_t value){for(uint32_t shift=0U;shift<32U;shift+=8U)out.push_back(static_cast<uint8_t>((value>>shift)&0xFFU));}
void write_u64(std::vector<uint8_t>&out,uint64_t value){for(uint32_t shift=0U;shift<64U;shift+=8U)out.push_back(static_cast<uint8_t>((value>>shift)&0xFFU));}
uint64_t read_u64(const uint8_t*data){uint64_t value=0U;for(uint32_t shift=0U;shift<64U;shift+=8U)value|=static_cast<uint64_t>(data[shift/8U])<<shift;return value;}
bool checked_size(cnc::Size count,cnc::Size*out)noexcept{if(out==nullptr||count>ScienceLedgerCodec::max_entries||count>(std::numeric_limits<cnc::Size>::max()-H)/R)return false;*out=H+count*R;return true;}
}}
namespace zero_hour {
cnc::Error ScienceLedgerCodec::encode(const ScienceLedger::Snapshot&snapshot,std::vector<uint8_t>*out)noexcept
{if(out==nullptr)return FT_ERR_INVALID_POINTER;const cnc::Size count=static_cast<cnc::Size>(snapshot.purchased.size());cnc::Size total=0U;if(snapshot.schema_version!=wire_schema_version||!checked_size(count,&total))return FT_ERR_INVALID_ARGUMENT;try{out->clear();out->reserve(total);write_u32(*out,wire_schema_version);write_u32(*out,static_cast<uint32_t>(count));for(cnc::Size i=0U;i<count;++i){const auto id=snapshot.purchased[i];if(id.value==0U||(i!=0U&&snapshot.purchased[i-1U].value>=id.value)){out->clear();return FT_ERR_INVALID_ARGUMENT;}write_u64(*out,id.value);}}catch(...){out->clear();return FT_ERR_NO_MEMORY;}return FT_ERR_SUCCESS;}
cnc::Error ScienceLedgerCodec::decode(const uint8_t*data,cnc::Size byte_count,ScienceLedger::Snapshot*out)noexcept
{if(data==nullptr||out==nullptr)return FT_ERR_INVALID_POINTER;if(byte_count<H)return FT_ERR_INVALID_ARGUMENT;const uint32_t schema=static_cast<uint32_t>(data[0U])|(static_cast<uint32_t>(data[1U])<<8U)|(static_cast<uint32_t>(data[2U])<<16U)|(static_cast<uint32_t>(data[3U])<<24U);const uint32_t count=static_cast<uint32_t>(data[4U])|(static_cast<uint32_t>(data[5U])<<8U)|(static_cast<uint32_t>(data[6U])<<16U)|(static_cast<uint32_t>(data[7U])<<24U);cnc::Size expected=0U;if(schema!=wire_schema_version||!checked_size(static_cast<cnc::Size>(count),&expected)||expected!=byte_count)return FT_ERR_CONFIGURATION;ScienceLedger::Snapshot decoded;try{decoded.purchased.reserve(count);for(uint32_t i=0U;i<count;++i){const cnc::DefinitionId id{read_u64(data+H+static_cast<cnc::Size>(i)*R)};if(id.value==0U||(i!=0U&&decoded.purchased.back().value>=id.value))return FT_ERR_CONFIGURATION;decoded.purchased.push_back(id);}}catch(...){return FT_ERR_NO_MEMORY;}out->schema_version=wire_schema_version;out->purchased.swap(decoded.purchased);return FT_ERR_SUCCESS;}
}
