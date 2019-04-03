#include <cctype>
#include <iomanip>
#include <utility>

#include "fisch/vx/jtag.h"
#include "haldls/cerealization.h"
#include "haldls/vx/omnibus_constants.h"
#include "haldls/vx/spi.h"

namespace haldls {
namespace vx {

ShiftRegister::ShiftRegister() :
    m_adc_source(halco::hicann_dls::vx::ADCSourceOnBoard::None),
    m_enable_capmem_i_ref(true),
    m_enable_measure_capmem_i_ref(false),
    m_enable_dac_to_readout_0(false),
    m_enable_dac_to_readout_1(false),
    m_enable_led(),
    m_enable_adc_power_down(false),
    m_enable_adc_reset(false),
    m_enable_vdd()
{
	m_enable_led.fill(false);
	m_enable_vdd.fill(true);
}

bool ShiftRegister::get_enable_capmem_i_ref() const
{
	return m_enable_capmem_i_ref;
}

void ShiftRegister::set_enable_capmem_i_ref(bool const value)
{
	m_enable_capmem_i_ref = value;
}

bool ShiftRegister::get_enable_measure_capmem_i_ref() const
{
	return m_enable_measure_capmem_i_ref;
}

void ShiftRegister::set_enable_measure_capmem_i_ref(bool const value)
{
	m_enable_measure_capmem_i_ref = value;
}

halco::hicann_dls::vx::ADCSourceOnBoard ShiftRegister::get_adc_source() const
{
	return m_adc_source;
}

void ShiftRegister::set_adc_source(halco::hicann_dls::vx::ADCSourceOnBoard const& value)
{
	m_adc_source = value;
}

bool ShiftRegister::get_enable_dac_to_readout_0() const
{
	return m_enable_dac_to_readout_0;
}

void ShiftRegister::set_enable_dac_to_readout_0(bool const value)
{
	m_enable_dac_to_readout_0 = value;
}

bool ShiftRegister::get_enable_dac_to_readout_1() const
{
	return m_enable_dac_to_readout_1;
}

void ShiftRegister::set_enable_dac_to_readout_1(bool const value)
{
	m_enable_dac_to_readout_1 = value;
}

bool ShiftRegister::get_enable_led(halco::hicann_dls::vx::LEDOnBoard const& coord) const
{
	return m_enable_led[coord];
}

void ShiftRegister::set_enable_led(halco::hicann_dls::vx::LEDOnBoard const& coord, bool const value)
{
	m_enable_led[coord] = value;
}

bool ShiftRegister::get_enable_adc_power_down() const
{
	return m_enable_adc_power_down;
}

void ShiftRegister::set_enable_adc_power_down(bool const value)
{
	m_enable_adc_power_down = value;
}

bool ShiftRegister::get_enable_adc_reset() const
{
	return m_enable_adc_reset;
}

void ShiftRegister::set_enable_adc_reset(bool const value)
{
	m_enable_adc_reset = value;
}

bool ShiftRegister::get_enable_vdd(halco::hicann_dls::vx::VDDOnBoard const& coord) const
{
	return m_enable_vdd[coord];
}

void ShiftRegister::set_enable_vdd(halco::hicann_dls::vx::VDDOnBoard const& coord, bool const value)
{
	m_enable_vdd[coord] = value;
}

bool ShiftRegister::operator==(ShiftRegister const& other) const
{
	return m_adc_source == other.m_adc_source &&
	       m_enable_capmem_i_ref == other.m_enable_capmem_i_ref &&
	       m_enable_measure_capmem_i_ref == other.m_enable_measure_capmem_i_ref &&
	       m_enable_dac_to_readout_0 == other.m_enable_dac_to_readout_0 &&
	       m_enable_dac_to_readout_1 == other.m_enable_dac_to_readout_1 &&
	       m_enable_led == other.m_enable_led &&
	       m_enable_adc_power_down == other.m_enable_adc_power_down &&
	       m_enable_adc_reset == other.m_enable_adc_reset && m_enable_vdd == other.m_enable_vdd;
}

bool ShiftRegister::operator!=(ShiftRegister const& other) const
{
	return !(*this == other);
}

std::array<omnibus_address_type, ShiftRegister::config_size_in_words> ShiftRegister::addresses(
    coordinate_type const& /*coord*/) const
{
	constexpr omnibus_address_type addr(spi_over_omnibus_mask + 1);
	return {addr, addr, addr};
}

namespace {

struct ShiftRegisterBitfield0
{
	union
	{
		uint32_t raw;
		// clang-format off
		struct __attribute__((packed)) {
			uint32_t switch_0            :  2;
			uint32_t switch_1            :  2;
			uint32_t switch_2            :  2;
			uint32_t enable_capmem_i_ref   :  1;
			uint32_t enable_measure_capmem_i_ref :  1;
			uint32_t /* unused */        : 24;
		} m;
		// clang-format on
		static_assert(sizeof(raw) == sizeof(m), "sizes of union types should match");
	} u;

	ShiftRegisterBitfield0() { u.raw = 0u; }

	ShiftRegisterBitfield0(uint32_t data) { u.raw = data; }
};

struct ShiftRegisterBitfield1
{
	union
	{
		uint32_t raw;
		// clang-format off
		struct __attribute__((packed)) {
			uint32_t enable_dac_to_readout_0    :  1;
			uint32_t enable_dac_to_readout_1    :  1;
			uint32_t disable_led_1              :  1;
			uint32_t disable_led_2              :  1;
			uint32_t disable_led_3              :  1;
			uint32_t enable_adc_power_down :  1;
			uint32_t enable_adc_reset      :  1;
			uint32_t disable_led_4              :  1;
			uint32_t /* unused */               : 24;
		} m;
		// clang-format on
		static_assert(sizeof(raw) == sizeof(m), "sizes of union types should match");
	} u;

	ShiftRegisterBitfield1() { u.raw = 0u; }

	ShiftRegisterBitfield1(uint32_t data) { u.raw = data; }
};

struct ShiftRegisterBitfield2
{
	union
	{
		uint32_t raw;
		// clang-format off
		struct __attribute__((packed)) {
			uint32_t enable_vdd25digital : 1;
			uint32_t enable_vdd12digital : 1;
			uint32_t enable_vdd25analog : 1;
			uint32_t enable_vdd12analog : 1;
			uint32_t enable_vdd12madc : 1;
			uint32_t enable_vdd12pll : 1;
			uint32_t disable_led_7 : 1;
			uint32_t disable_led_8 : 1;
			uint32_t /* unused */ : 24;
		} m;
		// clang-format on
		static_assert(sizeof(raw) == sizeof(m), "sizes of union types should match");
	} u;

	ShiftRegisterBitfield2() { u.raw = 0u; }

	ShiftRegisterBitfield2(uint32_t data) { u.raw = data; }
};

} // namespace

std::array<fisch::vx::Omnibus, ShiftRegister::config_size_in_words> ShiftRegister::encode() const
{
	ShiftRegisterBitfield0 bitfield_0;
	switch (m_adc_source / 3) {
		case 0: { // source is on switch 0
			bitfield_0.u.m.switch_0 = m_adc_source % 3;
			break;
		}
		case 1: { // source is on switch 1
			bitfield_0.u.m.switch_0 = m_adc_source % 3;
			break;
		}
		case 2: { // source is on switch 2
			bitfield_0.u.m.switch_0 = m_adc_source % 3;
			break;
		}
		case 3: { // m_adc_source == ADCSourceOnBoard::None
			break;
		}
	}

	bitfield_0.u.m.enable_capmem_i_ref = m_enable_capmem_i_ref;
	bitfield_0.u.m.enable_measure_capmem_i_ref = m_enable_measure_capmem_i_ref;

	ShiftRegisterBitfield1 bitfield_1;
	bitfield_1.u.m.enable_dac_to_readout_0 = m_enable_dac_to_readout_0;
	bitfield_1.u.m.enable_dac_to_readout_1 = m_enable_dac_to_readout_1;
	bitfield_1.u.m.disable_led_1 = !m_enable_led[halco::hicann_dls::vx::LEDOnBoard::LED1];
	bitfield_1.u.m.disable_led_2 = !m_enable_led[halco::hicann_dls::vx::LEDOnBoard::LED2];
	bitfield_1.u.m.disable_led_3 = !m_enable_led[halco::hicann_dls::vx::LEDOnBoard::LED3];
	bitfield_1.u.m.enable_adc_power_down = m_enable_adc_power_down;
	bitfield_1.u.m.enable_adc_reset = m_enable_adc_reset;
	bitfield_1.u.m.disable_led_4 = !m_enable_led[halco::hicann_dls::vx::LEDOnBoard::LED4];

	ShiftRegisterBitfield2 bitfield_2;
	bitfield_2.u.m.enable_vdd25digital =
	    m_enable_vdd[halco::hicann_dls::vx::VDDOnBoard::VDD25Digital];
	bitfield_2.u.m.enable_vdd12digital =
	    m_enable_vdd[halco::hicann_dls::vx::VDDOnBoard::VDD12Digital];
	bitfield_2.u.m.enable_vdd25analog =
	    m_enable_vdd[halco::hicann_dls::vx::VDDOnBoard::VDD25Analog];
	bitfield_2.u.m.enable_vdd12analog =
	    m_enable_vdd[halco::hicann_dls::vx::VDDOnBoard::VDD12Analog];
	bitfield_2.u.m.enable_vdd12madc = m_enable_vdd[halco::hicann_dls::vx::VDDOnBoard::VDD12MADC];
	bitfield_2.u.m.enable_vdd12pll = m_enable_vdd[halco::hicann_dls::vx::VDDOnBoard::VDD12Pll];
	bitfield_2.u.m.disable_led_7 = !m_enable_led[halco::hicann_dls::vx::LEDOnBoard::LED7];
	bitfield_2.u.m.disable_led_8 = !m_enable_led[halco::hicann_dls::vx::LEDOnBoard::LED8];

	return {
	    fisch::vx::Omnibus(fisch::vx::OmnibusData(bitfield_2.u.raw)),
	    fisch::vx::Omnibus(fisch::vx::OmnibusData(bitfield_1.u.raw)),
	    fisch::vx::Omnibus(fisch::vx::OmnibusData(spi_over_omnibus_stop_bit | bitfield_0.u.raw))};
}

void ShiftRegister::decode(
    std::array<fisch::vx::Omnibus, ShiftRegister::config_size_in_words> const& /*data*/)
{}

template <typename Archive>
void ShiftRegister::cerealize(Archive& ar)
{
	ar(CEREAL_NVP(m_adc_source));
	ar(CEREAL_NVP(m_enable_capmem_i_ref));
	ar(CEREAL_NVP(m_enable_measure_capmem_i_ref));
	ar(CEREAL_NVP(m_enable_dac_to_readout_0));
	ar(CEREAL_NVP(m_enable_dac_to_readout_1));
	ar(CEREAL_NVP(m_enable_led));
	ar(CEREAL_NVP(m_enable_adc_power_down));
	ar(CEREAL_NVP(m_enable_adc_reset));
	ar(CEREAL_NVP(m_enable_vdd));
}

EXPLICIT_INSTANTIATE_CEREAL_SERIALIZE(ShiftRegister)

} // namespace vx
} // namespace haldls
