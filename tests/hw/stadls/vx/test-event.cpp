#include <gtest/gtest.h>

#include "haldls/vx/event.h"
#include "haldls/vx/systime.h"
#include "haldls/vx/timer.h"
#include "stadls/vx/playback_program_builder.h"

#include "executor.h"
#include "test-init_helper.h"

using namespace halco::common;
using namespace halco::hicann_dls::vx;
using namespace haldls::vx;
using namespace stadls::vx;

#define TEST_SPIKE(Num)                                                                            \
	TEST(SpikePack##Num##ToChip, Loopback)                                                         \
	{                                                                                              \
		PlaybackProgramBuilder builder;                                                            \
                                                                                                   \
		insert_highspeed_init(builder);                                                            \
                                                                                                   \
		builder.write(SystimeSyncOnFPGA(), SystimeSync());                                         \
                                                                                                   \
		/* wait until systime init is finished */                                                  \
		builder.write(TimerOnDLS(), Timer());                                                      \
		builder.wait_until(TimerOnDLS(), Timer::Value(1000));                                      \
                                                                                                   \
		constexpr size_t num_spikes = 1000;                                                        \
                                                                                                   \
		std::vector<SpikeLabel> to_fpga_spike_labels;                                              \
		for (size_t i = 0; i < num_spikes; ++i) {                                                  \
			auto label = SpikeLabel(                                                               \
			    NeuronLabel(i % NeuronLabel::size), SPL1Address(i % SPL1Address::size));           \
			SpikePack##Num##ToChip::labels_type labels;                                            \
			labels.fill(label);                                                                    \
			SpikePack##Num##ToChip spike(labels);                                                  \
			builder.write(SpikePack##Num##ToChipOnDLS(), spike);                                   \
			builder.write(TimerOnDLS(), Timer());                                                  \
			builder.wait_until(TimerOnDLS(), Timer::Value(10));                                    \
			to_fpga_spike_labels.push_back(spike.get_labels().at(0));                              \
		}                                                                                          \
                                                                                                   \
		builder.write(TimerOnDLS(), Timer());                                                      \
		builder.wait_until(TimerOnDLS(), Timer::Value(1000));                                      \
		auto program = builder.done();                                                             \
                                                                                                   \
		auto executor = generate_playback_program_test_executor();                                 \
		executor.run(program);                                                                     \
                                                                                                   \
		auto spikes = program.get_spikes();                                                        \
                                                                                                   \
		EXPECT_LE(spikes.size(), num_spikes* Num);                                                 \
		EXPECT_GT(spikes.size(), 0);                                                               \
                                                                                                   \
		for (auto spike : spikes) {                                                                \
			auto it = std::find(                                                                   \
			    to_fpga_spike_labels.cbegin(), to_fpga_spike_labels.cend(),                        \
			    spike.get_spike().get_label());                                                    \
			EXPECT_TRUE(it != to_fpga_spike_labels.cend()) << "Received spike not sent.";          \
		}                                                                                          \
	}

// clang-format off
TEST_SPIKE(1)
TEST_SPIKE(2)
TEST_SPIKE(3)
// clang-format on

#undef TEST_SPIKE