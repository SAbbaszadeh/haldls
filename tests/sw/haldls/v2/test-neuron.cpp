#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "test-helper.h"
#include "haldls/v2/neuron.h"
#include "stadls/visitors.h"

using namespace haldls::v2;
using namespace halco::hicann_dls::v2;
using namespace halco::common;

typedef std::vector<hardware_address_type> addresses_type;
typedef std::vector<hardware_word_type> words_type;

TEST(CommonNeuronConfig, General)
{
	CommonNeuronConfig config;

	// test default constructor
	ASSERT_EQ(config.get_enable_digital_out(), false);
	ASSERT_EQ(
	    config.get_post_correlation_signal_length(),
	    CommonNeuronConfig::PostCorrelationSignalLength(0));
	ASSERT_EQ(config.get_enable_external_post_correlation_signal(), false);
	ASSERT_EQ(config.get_inhibit_spike_comparator(), false);

	EXPECT_ANY_THROW(CommonNeuronConfig::PostCorrelationSignalLength(16));
	EXPECT_NO_THROW(CommonNeuronConfig::PostCorrelationSignalLength(15));

	config.set_enable_digital_out(true);
	ASSERT_EQ(config.get_enable_digital_out(), true);

	config.set_post_correlation_signal_length(CommonNeuronConfig::PostCorrelationSignalLength(15));
	ASSERT_EQ(
	    config.get_post_correlation_signal_length(),
	    CommonNeuronConfig::PostCorrelationSignalLength(15));

	config.set_enable_external_post_correlation_signal(true);
	ASSERT_EQ(config.get_enable_external_post_correlation_signal(), true);

	config.set_inhibit_spike_comparator(true);
	ASSERT_EQ(config.get_inhibit_spike_comparator(), true);

	CommonNeuronConfig config_eq = config;
	CommonNeuronConfig config_ne = config;
	config_ne.set_inhibit_spike_comparator(false);
	config_ne.set_post_correlation_signal_length(CommonNeuronConfig::PostCorrelationSignalLength(13));


	// test relation operator
	ASSERT_EQ(config, config_eq);
	ASSERT_FALSE(config == config_ne);

	ASSERT_NE(config, config_ne);
	ASSERT_FALSE(config != config_eq);
}

TEST(CommonNeuronConfig, EncodeDecode)
{
	CommonNeuronConfig config;

	config.set_enable_digital_out(true);
	config.set_post_correlation_signal_length(CommonNeuronConfig::PostCorrelationSignalLength(11));
	config.set_enable_external_post_correlation_signal(false);
	config.set_inhibit_spike_comparator(true);

	CommonNeuronConfigOnDLS coord;

	std::array<hardware_address_type, CommonNeuronConfig::config_size_in_words> ref_addresses = {{0x1a000000}};
	std::array<hardware_word_type, CommonNeuronConfig::config_size_in_words> ref_data = {{0b0101'0111ul}};

	{ // write addresses
		addresses_type write_addresses;
		visit_preorder(
			config, coord, stadls::WriteAddressVisitor<addresses_type>{write_addresses});
		EXPECT_THAT(write_addresses, ::testing::ElementsAreArray(ref_addresses));
	}

	{ // read addresses
		addresses_type read_addresses;
		visit_preorder(
			config, coord, stadls::ReadAddressVisitor<addresses_type>{read_addresses});
		EXPECT_THAT(read_addresses, ::testing::ElementsAreArray(ref_addresses));
	}

	words_type data;
	visit_preorder(config, coord, stadls::EncodeVisitor<words_type>{data});
	EXPECT_THAT(data, ::testing::ElementsAreArray(ref_data));

	CommonNeuronConfig config_copy;
	ASSERT_NE(config, config_copy);
	visit_preorder(config_copy, coord, stadls::DecodeVisitor<words_type>{std::move(data)});
	ASSERT_EQ(config, config_copy);
}

TEST(CommonNeuronConfig, CerealizeCoverage)
{
	CommonNeuronConfig obj1,obj2;
	obj1.set_enable_digital_out(!obj1.get_enable_digital_out());
	obj1.set_enable_external_post_correlation_signal(!obj1.get_enable_external_post_correlation_signal());
	obj1.set_inhibit_spike_comparator(!obj1.get_inhibit_spike_comparator());
	obj1.set_post_correlation_signal_length(draw_ranged_non_default_value<CommonNeuronConfig::PostCorrelationSignalLength>(obj1.get_post_correlation_signal_length()));

	std::ostringstream ostream;
	{
		cereal::JSONOutputArchive oa(ostream);
		oa(obj1);
	}

	std::istringstream istream(ostream.str());
	{
		cereal::JSONInputArchive ia(istream);
		ia(obj2);
	}
	ASSERT_EQ(obj1, obj2);
}

TEST(NeuronDigitalConfig, General)
{
	NeuronDigitalConfig neuron;

	// test default constructor
	ASSERT_EQ(neuron.get_enable_synapse_input_excitatory(), false);
	ASSERT_EQ(neuron.get_enable_synapse_input_inhibitory(), false);
	ASSERT_EQ(neuron.get_enable_high_conductance_leak(), false);
	ASSERT_EQ(neuron.get_enable_leak(), true);
	ASSERT_EQ(neuron.get_enable_smallcap(), true);
	ASSERT_EQ(neuron.get_fire_out_mode(), NeuronDigitalConfig::FireOutMode::disabled);
	ASSERT_EQ(neuron.get_mux_readout_mode(), NeuronDigitalConfig::MuxReadoutMode::v_mem);
	ASSERT_EQ(neuron.get_enable_unbuffered_readout(), false);

	neuron.set_enable_synapse_input_excitatory(true);
	ASSERT_EQ(neuron.get_enable_synapse_input_excitatory(), true);

	neuron.set_enable_synapse_input_inhibitory(true);
	ASSERT_EQ(neuron.get_enable_synapse_input_inhibitory(), true);

	neuron.set_enable_high_conductance_leak(true);
	ASSERT_EQ(neuron.get_enable_high_conductance_leak(), true);

	neuron.set_enable_leak(false);
	ASSERT_EQ(neuron.get_enable_leak(), false);

	neuron.set_enable_bigcap(false);
	ASSERT_EQ(neuron.get_enable_bigcap(), false);

	neuron.set_enable_smallcap(false);
	ASSERT_EQ(neuron.get_enable_smallcap(), false);

	neuron.set_fire_out_mode(NeuronDigitalConfig::FireOutMode::bypass_exc);
	ASSERT_EQ(neuron.get_fire_out_mode(), NeuronDigitalConfig::FireOutMode::bypass_exc);

	neuron.set_mux_readout_mode(NeuronDigitalConfig::MuxReadoutMode::i_psp);
	ASSERT_EQ(neuron.get_mux_readout_mode(), NeuronDigitalConfig::MuxReadoutMode::i_psp);

	neuron.set_enable_unbuffered_readout(true);
	ASSERT_EQ(neuron.get_enable_unbuffered_readout(), true);

	NeuronDigitalConfig neuron2 = neuron;
	NeuronDigitalConfig neuron3 = neuron;
	neuron3.set_fire_out_mode(NeuronDigitalConfig::FireOutMode::bypass_inh);

	ASSERT_EQ(neuron, neuron2);
	ASSERT_FALSE(neuron == neuron3);

	ASSERT_NE(neuron, neuron3);
	ASSERT_FALSE(neuron != neuron2);
}

TEST(NeuronDigitalConfig, EncodeDecode)
{
	NeuronDigitalConfig config;

	config.set_enable_synapse_input_excitatory(true);
	config.set_enable_synapse_input_inhibitory(true);
	config.set_enable_high_conductance_leak(true);
	config.set_enable_leak(false);
	config.set_enable_bigcap(false);
	config.set_enable_smallcap(false);
	config.set_fire_out_mode(NeuronDigitalConfig::FireOutMode::bypass_exc);
	config.set_mux_readout_mode(NeuronDigitalConfig::MuxReadoutMode::i_psp);

	NeuronOnDLS coord(5);

	std::array<hardware_address_type, NeuronDigitalConfig::config_size_in_words> ref_addresses = {{0x1a000001 + 5}};
	std::array<hardware_word_type, NeuronDigitalConfig::config_size_in_words> ref_data = {{0b1000'0000'0111'0100ul}};

	{ // write addresses
		addresses_type write_addresses;
		visit_preorder(
			config, coord, stadls::WriteAddressVisitor<addresses_type>{write_addresses});
		EXPECT_THAT(write_addresses, ::testing::ElementsAreArray(ref_addresses));
	}

	{ // read addresses
		addresses_type read_addresses;
		visit_preorder(
			config, coord, stadls::ReadAddressVisitor<addresses_type>{read_addresses});
		EXPECT_THAT(read_addresses, ::testing::ElementsAreArray(ref_addresses));
	}

	words_type data;
	visit_preorder(config, coord, stadls::EncodeVisitor<words_type>{data});
	EXPECT_THAT(data, ::testing::ElementsAreArray(ref_data));

	NeuronDigitalConfig config_copy;
	ASSERT_NE(config, config_copy);
	visit_preorder(config_copy, coord, stadls::DecodeVisitor<words_type>{std::move(data)});
	ASSERT_EQ(config, config_copy);
}

TEST(NeuronDigitalConfig, CerealizeCoverage)
{
	NeuronDigitalConfig obj1,obj2;

// take boolean member and invert its state
#define INVERT(what)\
	obj1.set_##what (!obj1.get_##what ());
	INVERT(enable_synapse_input_excitatory)
	INVERT(enable_synapse_input_inhibitory)
	INVERT(enable_high_conductance_leak)
	INVERT(enable_leak)
	INVERT(enable_bigcap)
	INVERT(enable_smallcap)
	INVERT(enable_unbuffered_readout)
#undef INVERT
	obj1.set_fire_out_mode(NeuronDigitalConfig::FireOutMode::enabled);
	obj1.set_mux_readout_mode(NeuronDigitalConfig::MuxReadoutMode::spike);
	// buffered readout tested for chip, as not settable at a single neuron container

	std::ostringstream ostream;
	{
		cereal::JSONOutputArchive oa(ostream);
		oa(obj1);
	}

	std::istringstream istream(ostream.str());
	{
		cereal::JSONInputArchive ia(istream);
		ia(obj2);
	}
	ASSERT_EQ(obj1, obj2);
}

