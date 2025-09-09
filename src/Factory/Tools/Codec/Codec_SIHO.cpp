#include <streampu.hpp>

#include "Factory/Tools/Codec/BCH/Codec_BCH.hpp"
#include "Factory/Tools/Codec/Codec_SIHO.hpp"


#include "Factory/Tools/Codec/Repetition/Codec_repetition.hpp"

#include "Factory/Tools/Codec/Turbo_product/Codec_turbo_product.hpp"

using namespace aff3ct;
using namespace aff3ct::factory;

const std::string aff3ct::factory::Codec_SIHO_name = "Codec SIHO";
const std::string aff3ct::factory::Codec_SIHO_prefix = "cdc";

Codec_SIHO ::Codec_SIHO(const std::string& prefix)
  : Codec_HIHO(Codec_SIHO_name, prefix)
{
}

Codec_SIHO ::Codec_SIHO(const std::string& name, const std::string& prefix)
  : Codec_HIHO(name, prefix)
{
}

template<typename B, typename Q>
tools::Codec_SIHO<B, Q>*
Codec_SIHO ::build(const module::CRC<B>* crc) const
{
    if (get_name() == Codec_BCH_name) return dynamic_cast<const Codec_BCH&>(*this).template build<B, Q>(crc);


    if (get_name() == Codec_turbo_product_name)
        return dynamic_cast<const Codec_turbo_product&>(*this).template build<B, Q>(crc);

    throw spu::tools::cannot_allocate(__FILE__, __LINE__, __func__);
}

// ==================================================================================== explicit template instantiation
#include "Tools/types.h"
#ifdef AFF3CT_MULTI_PREC
template aff3ct::tools::Codec_SIHO<B_8, Q_8>*
aff3ct::factory::Codec_SIHO::build<B_8, Q_8>(const aff3ct::module::CRC<B_8>*) const;
template aff3ct::tools::Codec_SIHO<B_16, Q_16>*
aff3ct::factory::Codec_SIHO::build<B_16, Q_16>(const aff3ct::module::CRC<B_16>*) const;
template aff3ct::tools::Codec_SIHO<B_32, Q_32>*
aff3ct::factory::Codec_SIHO::build<B_32, Q_32>(const aff3ct::module::CRC<B_32>*) const;
template aff3ct::tools::Codec_SIHO<B_64, Q_64>*
aff3ct::factory::Codec_SIHO::build<B_64, Q_64>(const aff3ct::module::CRC<B_64>*) const;
#else
template aff3ct::tools::Codec_SIHO<B, Q>*
aff3ct::factory::Codec_SIHO::build<B, Q>(const aff3ct::module::CRC<B>*) const;
#endif
// ==================================================================================== explicit template instantiation
