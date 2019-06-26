#pragma once

#include <algorithm>
#include <ostream>

#include "halco/common/typed_array.h"
#include "halco/hicann-dls/vx/coordinates.h"

#include "haldls/vx/common.h"
#include "haldls/vx/genpybind.h"
#include "haldls/vx/traits.h"
#include "hate/visibility.h"

namespace cereal {
class access;
} // namespace cereal

namespace fisch::vx {
class OmnibusChipOverJTAG;
class OmnibusChip;
} // namespace fisch::vx

namespace haldls {
namespace vx GENPYBIND_TAG_HALDLS_VX {

class GENPYBIND(visible) CrossbarOutputConfig
{
public:
	typedef halco::hicann_dls::vx::CrossbarOutputConfigOnDLS coordinate_type;
	typedef std::true_type is_leaf_node;

	/** Default constructor. */
	CrossbarOutputConfig() SYMBOL_VISIBLE;

	/**
	 * Get enable value for event counter of specified output.
	 * @param coord Output coordinate
	 * @return Boolean value
	 */
	GENPYBIND(getter_for(enable_event_counter))
	bool get_enable_event_counter(halco::hicann_dls::vx::CrossbarOutputOnDLS const& coord) const
	    SYMBOL_VISIBLE;

	/**
	 * Set enable value for event counter of specified output.
	 * @param coord Output coordinate
	 * @param value Boolean value
	 */
	GENPYBIND(setter_for(enable_event_counter))
	void set_enable_event_counter(
	    halco::hicann_dls::vx::CrossbarOutputOnDLS const& coord, bool value) SYMBOL_VISIBLE;

	GENPYBIND(getter_for(enable_slow))
	bool get_enable_slow(halco::hicann_dls::vx::CrossbarL2OutputOnDLS const& coord) const
	    SYMBOL_VISIBLE;

	GENPYBIND(setter_for(enable_slow))
	void set_enable_slow(halco::hicann_dls::vx::CrossbarL2OutputOnDLS const& coord, bool value)
	    SYMBOL_VISIBLE;

	bool operator==(CrossbarOutputConfig const& other) const SYMBOL_VISIBLE;
	bool operator!=(CrossbarOutputConfig const& other) const SYMBOL_VISIBLE;

	static size_t constexpr write_config_size_in_words GENPYBIND(hidden) = 1;
	static size_t constexpr read_config_size_in_words GENPYBIND(hidden) = 0;
	template <typename AddressT>
	std::array<AddressT, read_config_size_in_words> read_addresses(
	    coordinate_type const& neuron) const SYMBOL_VISIBLE GENPYBIND(hidden);
	template <typename AddressT>
	std::array<AddressT, write_config_size_in_words> write_addresses(
	    coordinate_type const& neuron) const SYMBOL_VISIBLE GENPYBIND(hidden);
	template <typename WordT>
	std::array<WordT, write_config_size_in_words> encode() const SYMBOL_VISIBLE GENPYBIND(hidden);
	template <typename WordT>
	void decode(std::array<WordT, read_config_size_in_words> const& data) SYMBOL_VISIBLE
	    GENPYBIND(hidden);

	GENPYBIND(stringstream)
	friend std::ostream& operator<<(std::ostream& os, CrossbarOutputConfig const& config)
	    SYMBOL_VISIBLE;

private:
	friend class cereal::access;
	template <class Archive>
	void serialize(Archive& ar) SYMBOL_VISIBLE;

	halco::common::typed_array<bool, halco::hicann_dls::vx::CrossbarOutputOnDLS>
	    m_enable_event_counter;
	halco::common::typed_array<bool, halco::hicann_dls::vx::CrossbarL2OutputOnDLS> m_enable_slow;
};

namespace detail {

template <>
struct BackendContainerTrait<CrossbarOutputConfig>
    : public BackendContainerBase<
          CrossbarOutputConfig,
          fisch::vx::OmnibusChipOverJTAG,
          fisch::vx::OmnibusChip>
{};

} // namespace detail

} // namespace vx
} // namespace haldls
