#include <gtest/gtest.h>

#include "halco/hicann-dls/vx/coordinates.h"
#include "haldls/vx/jtag.h"
#include "haldls/vx/pll.h"
#include "haldls/vx/reset.h"
#include "haldls/vx/timer.h"
#include "stadls/vx/playback_program.h"
#include "stadls/vx/playback_program_builder.h"

#include "executor.h"

using namespace halco::common;
using namespace halco::hicann_dls::vx;
using namespace haldls::vx;
using namespace stadls::vx;


class PLLTest : public ::testing::Test
{
protected:
	PLLTest() {}

	void configure()
	{
		builder.write(ResetChipOnDLS(), ResetChip(true));
		builder.write(TimerOnDLS(), Timer());
		builder.wait_until(TimerOnDLS(), Timer::Value(10));
		builder.write(ResetChipOnDLS(), ResetChip(false));
		builder.wait_until(TimerOnDLS(), Timer::Value(100));

		builder.write(JTAGClockScalerOnDLS(), JTAGClockScaler(JTAGClockScaler::Value(3)));
		builder.write(ResetJTAGTapOnDLS(), ResetJTAGTap());

		// wait until ASIC omnibus is up (22 us)
		builder.wait_until(TimerOnDLS(), Timer::Value(22 * 125));
		builder.write(TimerOnDLS(), Timer());
	}

	void test_run_program(PlaybackProgram& program)
	{
		auto executor = generate_playback_program_test_executor();
		executor.run(program);
	}

	PlaybackProgramBuilder builder;
}; // PLLTest

class PLLTestWriteReadADPLL
    : public PLLTest
    , public ::testing::WithParamInterface<size_t>
{
protected:
	PLLTestWriteReadADPLL() : PLLTest() {}

	void configure()
	{
		PLLTest::configure();

		auto coord = ADPLLOnDLS(GetParam());

		ADPLL adpll_config;

		builder.write(coord, adpll_config, Backend::OmnibusChipOverJTAG);
		builder.write(TimerOnDLS(), Timer());
		// wait for omnibus clock to come up again
		builder.wait_until(TimerOnDLS(), Timer::Value(30 * 125));
		ticket = builder.read(coord, Backend::OmnibusChipOverJTAG);

		builder.wait_until(TimerOnDLS(), Timer::Value(10000));

		program = builder.done();
	}

	void test_run_program() { PLLTest::test_run_program(program); }

	void check_equality()
	{
		auto read_config = ticket->get();
		EXPECT_EQ(read_config, ADPLL());
	}

	PlaybackProgram program;
	std::optional<PlaybackProgram::ContainerTicket<ADPLL>> ticket;
}; // PLLTestWriteReadADPLL

INSTANTIATE_TEST_CASE_P(
    PLLTestWriteReadADPLLIteration,
    PLLTestWriteReadADPLL,
    ::testing::Range(ADPLLOnDLS::begin, ADPLLOnDLS::end));

TEST_P(PLLTestWriteReadADPLL, OmnibusEquality)
{
	configure();
	test_run_program();
	check_equality();
}

TEST_F(PLLTest, WriteReadPLLClockOutputBlockOmnibusEquality)
{
	configure();

	PLLClockOutputBlock config;
	PLLClockOutputBlockOnDLS coord;

	builder.write(coord, config, Backend::OmnibusChipOverJTAG);
	auto ticket = builder.read(coord, Backend::OmnibusChipOverJTAG);

	builder.wait_until(TimerOnDLS(), Timer::Value(10000));

	auto program = builder.done();
	test_run_program(program);

	EXPECT_EQ(ticket.get(), config);
}

TEST_F(PLLTest, WriteReadPLLSelfTestOmnibusEquality)
{
	configure();

	PLLSelfTest config;
	config.set_check_value(PLLSelfTest::CheckValue(0x1234));
	config.set_clock_enable(true);

	PLLSelfTestOnDLS coord;

	builder.write(coord, config, Backend::OmnibusChipOverJTAG);
	auto ticket = builder.read(coord, Backend::OmnibusChipOverJTAG);

	builder.wait_until(TimerOnDLS(), Timer::Value(10000));

	auto program = builder.done();
	test_run_program(program);

	EXPECT_EQ(ticket.get(), config);
}

TEST_F(PLLTest, PLLSelfTest)
{
	configure();

	{
		ADPLL adpll_config;
		for (auto adpll : iter_all<ADPLLOnDLS>()) {
			builder.write(adpll, adpll_config, Backend::OmnibusChipOverJTAG);
		}
	}

	{
		PLLClockOutputBlock config;
		PLLClockOutputBlockOnDLS coord;
		builder.write(coord, config, Backend::OmnibusChipOverJTAG);
	}

	PLLSelfTest config;
	config.set_pre_scaler_p(PLLSelfTest::PreScalerP(8));
	config.set_check_value(PLLSelfTest::CheckValue(10240));
	config.set_check_range(PLLSelfTest::CheckRange(2));
	config.set_clock_enable(true);

	PLLSelfTestOnDLS coord;

	builder.write(coord, config, Backend::OmnibusChipOverJTAG);

	builder.write(TimerOnDLS(), Timer());
	builder.wait_until(TimerOnDLS(), Timer::Value(10000));

	auto ticket = builder.read(PLLSelfTestStatusOnDLS(), Backend::OmnibusChipOverJTAG);

	builder.write(TimerOnDLS(), Timer());
	builder.wait_until(TimerOnDLS(), Timer::Value(10000));

	auto program = builder.done();
	test_run_program(program);

	EXPECT_TRUE(ticket.valid());

	auto status = ticket.get();

	EXPECT_TRUE(status.get_finished());
	EXPECT_TRUE(status.get_success());
	EXPECT_TRUE(
	    std::abs(
	        static_cast<long>(status.get_counter_value()) -
	        static_cast<long>(config.get_check_value())) <=
	    static_cast<long>(config.get_check_range()));
}
