#include <chrono>
#include <cli.hpp>
#include <date.h>
#include <rang.hpp>
#include <sstream>
#include <streampu.hpp>
#include <typeindex>
#include <typeinfo>
#include <unordered_map>

#include "Factory/Launcher/Launcher.hpp"
#include "Launcher/Code/BCH/BCH.hpp"

#include "Launcher/Code/Turbo_product/Turbo_product.hpp"
#include "Launcher/Launcher.hpp"
#include "Launcher/Simulation/BFER_ite.hpp"
#include "Launcher/Simulation/BFER_std.hpp"
#include "Tools/Documentation/documentation.h"
#include "Tools/general_utils.h"
#include "Tools/types.h"
#include "Tools/version.h"

using namespace aff3ct;
using namespace aff3ct::factory;

const std::string aff3ct::factory::Launcher_name = "Launcher";
const std::string aff3ct::factory::Launcher_prefix = "lch";

Launcher ::Launcher(const std::string& prefix)
  : Factory(Launcher_name, Launcher_name, prefix)
{
}

Launcher ::Launcher(const std::string& name, const std::string& short_name, const std::string& prefix)
  : Factory(name, short_name, prefix)
{
}

Launcher*
Launcher ::clone() const
{
    return new Launcher(*this);
}

void
Launcher ::get_description(cli::Argument_map_info& args) const
{
    auto p = this->get_prefix();
    const std::string class_name = "factory::Launcher::";

    tools::add_arg(
      args,
      p,
      class_name + "p+cde-type,C",
      cli::Text(cli::Including_set(
        "POLAR", "POLAR_MK", "TURBO", "TURBO_DB", "TPC", "LDPC", "REP", "RA", "RSC", "RSC_DB", "BCH", "UNCODED", "RS")),
      cli::arg_rank::REQ);

    tools::add_arg(args, p, class_name + "p+type", cli::Text(cli::Including_set("BFER", "BFERI")));

#ifdef AFF3CT_MULTI_PREC
    tools::add_arg(args, p, class_name + "p+prec,p", cli::Integer(cli::Including_set(8, 16, 32, 64)));
#endif

    tools::add_arg(args, p, class_name + "help,h", cli::None());

    tools::add_arg(args, p, class_name + "Help,H", cli::None());

    tools::add_arg(args, p, class_name + "version,v", cli::None());

#ifdef SPU_STACKTRACE
    tools::add_arg(args, p, class_name + "except-no-bt", cli::None(), cli::arg_rank::ADV);
#endif

    tools::add_arg(args, p, class_name + "no-legend", cli::None(), cli::arg_rank::ADV);

    tools::add_arg(args, p, class_name + "full-legend", cli::None(), cli::arg_rank::ADV);

    args.add_link({ "no-legend" }, { "full-legend" });

#ifdef SPU_COLORS
    tools::add_arg(args, p, class_name + "no-colors", cli::None());
#endif

    tools::add_arg(args, p, class_name + "keys,k", cli::None(), cli::arg_rank::ADV);
}

void
Launcher ::store(const cli::Argument_map_value& vals)
{
    auto p = this->get_prefix();

    if (vals.exist({ p + "-cde-type", "C" })) this->cde_type = vals.at({ p + "-cde-type", "C" }); // required
    if (vals.exist({ p + "-type" })) this->sim_type = vals.at({ p + "-type" });
    if (vals.exist({ "version", "v" })) this->display_version = true;
    if (vals.exist({ "keys", "k" })) this->display_keys = true;
    if (vals.exist({ "no-legend" })) this->display_legend = false;
    if (vals.exist({ "full-legend" })) this->full_legend = true;

    if (vals.exist({ "help", "h" }))
    {
        this->display_help = true;
        this->display_adv_help = false;
    }

    if (vals.exist({ "Help", "H" }))
    {
        this->display_help = true;
        this->display_adv_help = true;
    }

#ifdef AFF3CT_MULTI_PREC
    if (vals.exist({ p + "-prec", "p" })) this->sim_prec = vals.to_int({ p + "-prec", "p" });
#endif

    spu::tools::exception::no_stacktrace = vals.exist({ "except-no-bt" });

#ifdef SPU_COLORS
    if (vals.exist({ "no-colors" }))
    {
        rang::setControlMode(rang::control::Off);
        cli::disable_colors();
    }
#else
    rang::setControlMode(rang::control::Off);
    cli::disable_colors();
#endif
}

void
Launcher ::get_headers(std::map<std::string, tools::header_list>& params_headers, const bool /*full*/) const
{
    auto p = this->get_prefix();

    std::unordered_map<std::type_index, std::string> type_names;
    // define type names
    type_names[typeid(int8_t)] = "int8";
    type_names[typeid(int16_t)] = "int16";
    type_names[typeid(int32_t)] = "int32";
    type_names[typeid(int64_t)] = "int64";
    type_names[typeid(float)] = "float32";
    type_names[typeid(double)] = "float64";

    params_headers[p].push_back(std::make_pair("Type", this->sim_type));

#ifdef AFF3CT_MULTI_PREC
    std::type_index id_B = typeid(int32_t), id_R = typeid(float), id_Q = typeid(float);
    switch (this->sim_prec)
    {
        case 8:
            id_B = typeid(B_8);
            id_R = typeid(R_8);
            id_Q = typeid(Q_8);
            break;

        case 16:
            id_B = typeid(B_16);
            id_R = typeid(R_16);
            id_Q = typeid(Q_16);
            break;

        case 32:
            id_B = typeid(B_32);
            id_R = typeid(R_32);
            id_Q = typeid(Q_32);
            break;

        case 64:
            id_B = typeid(B_64);
            id_R = typeid(R_64);
            id_Q = typeid(Q_64);
            break;

        default:
            std::stringstream message;
            message << "Unsupported bit precision: " << this->sim_prec << ").";
            throw spu::tools::invalid_argument(__FILE__, __LINE__, __func__, message.str());
            break;
    }
#else
    std::type_index id_B = typeid(B), id_R = typeid(R), id_Q = typeid(Q);
#endif

    params_headers[p].push_back(std::make_pair("Type of bits", type_names[id_B]));
    params_headers[p].push_back(std::make_pair("Type of reals", type_names[id_R]));
    //	if (std::is_integral<Q>::value) // do not works for int8_t, int16_t, etc.
    if (id_Q == typeid(int8_t) || id_Q == typeid(int16_t) || id_Q == typeid(int32_t) || id_Q == typeid(int64_t))
        params_headers[p].push_back(std::make_pair("Type of quant. reals", type_names[id_Q]));

    using namespace date;
    std::stringstream date;
    date << std::chrono::system_clock::now();
    auto split_date = tools::split(date.str(), '.');
    params_headers[p].push_back(std::make_pair("Date (UTC)", split_date[0]));
    if (tools::version() != "GIT-NOTFOUND")
        params_headers[p].push_back(std::make_pair("Git version", tools::version()));

    params_headers[p].push_back(std::make_pair("Code type (C)", this->cde_type));
}

template<typename B, typename R, typename Q>
launcher::Launcher*
Launcher ::build(const int argc, const char** argv) const
{


    if (this->cde_type == "TPC")
    {
        if (this->sim_type == "BFER")
            return new launcher::Turbo_product<launcher::BFER_std<B, R, Q>, B, R, Q>(argc, argv);
    }


    if (this->cde_type == "BCH")
    {
        if (this->sim_type == "BFER") return new launcher::BCH<launcher::BFER_std<B, R, Q>, B, R, Q>(argc, argv);
    }







    std::stringstream message;
    message << "Invalid code type ('" << this->cde_type << "') or simulation type ('" << this->sim_type << "').";
    throw spu::tools::invalid_argument(__FILE__, __LINE__, __func__, message.str());
}

// ==================================================================================== explicit template instantiation
#include "Tools/types.h"
#ifdef AFF3CT_MULTI_PREC
template aff3ct::launcher::Launcher*
aff3ct::factory::Launcher::build<B_8, R_8, Q_8>(const int, const char**) const;
template aff3ct::launcher::Launcher*
aff3ct::factory::Launcher::build<B_16, R_16, Q_16>(const int, const char**) const;
template aff3ct::launcher::Launcher*
aff3ct::factory::Launcher::build<B_32, R_32, Q_32>(const int, const char**) const;
template aff3ct::launcher::Launcher*
aff3ct::factory::Launcher::build<B_64, R_64, Q_64>(const int, const char**) const;
#else
template aff3ct::launcher::Launcher*
aff3ct::factory::Launcher::build<B, R, Q>(const int, const char**) const;
#endif
// ==================================================================================== explicit template instantiation
