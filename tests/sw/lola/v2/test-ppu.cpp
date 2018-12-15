#include <cstdlib>

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "haldls/cerealization.h"
#include "haldls/v2/ppu.h"
#include "lola/v2/ppu.h"

using namespace lola::v2;
using namespace halco::hicann_dls::v2;

static std::string const test_ppu_program = TEST_PPU_PROGRAM;
/**
 * The source code to the pregenerated test program looks like:
 *
 * #include <s2pp.h>
 *
 * uint32_t a;
 *
 * double b;
 *
 * struct S
 * {
 *  int a;
 *  double b;
 * };
 *
 * void start()
 * {
 *  a = 3;
 *  b = 4.0;
 * }
 *
 */

TEST(PPUElfFile, General)
{
	EXPECT_NO_THROW(auto _ = PPUElfFile(test_ppu_program););

	PPUElfFile file(test_ppu_program);
	auto symbols = file.read_symbols();
	auto block = file.read_program();

	// all numbers below might and will change on changes in build profile, compiler or runtime
	constexpr size_t program_num_words = 113;
	PPUMemoryBlockOnDLS symbol_a_position(PPUMemoryWordOnDLS(113), PPUMemoryWordOnDLS(113));
	PPUMemoryBlockOnDLS symbol_b_position(PPUMemoryWordOnDLS(114), PPUMemoryWordOnDLS(115));
	PPUMemoryBlockOnDLS symbol___call_constructors_position(
	    PPUMemoryWordOnDLS(65), PPUMemoryWordOnDLS(88));
	PPUMemoryBlockOnDLS symbol___call_destructors_position(
	    PPUMemoryWordOnDLS(89), PPUMemoryWordOnDLS(112));
	PPUMemoryBlockOnDLS symbol___cxa_pure_virtual_position(
	    PPUMemoryWordOnDLS(24), PPUMemoryWordOnDLS(36));
	PPUMemoryBlockOnDLS symbol_start_position(PPUMemoryWordOnDLS(49), PPUMemoryWordOnDLS(64));
	PPUMemoryBlockOnDLS symbol__start_position(PPUMemoryWordOnDLS(37), PPUMemoryWordOnDLS(48));

	ASSERT_EQ(block.size(), program_num_words);

	PPUElfFile::symbols_type expected{
	    {"a", PPUProgram::Symbol{PPUProgram::Symbol::Type::object, symbol_a_position}},
	    {"b", PPUProgram::Symbol{PPUProgram::Symbol::Type::object, symbol_b_position}},
	    {"__call_constructors", PPUProgram::Symbol{PPUProgram::Symbol::Type::function,
	                                               symbol___call_constructors_position}},
	    {"__cxa_pure_virtual", PPUProgram::Symbol{PPUProgram::Symbol::Type::function,
	                                              symbol___cxa_pure_virtual_position}},
	    {"__call_destructors", PPUProgram::Symbol{PPUProgram::Symbol::Type::function,
	                                              symbol___call_destructors_position}},
	    {"_Z5startv",
	     PPUProgram::Symbol{PPUProgram::Symbol::Type::function, symbol_start_position}},
	    {"_start", PPUProgram::Symbol{PPUProgram::Symbol::Type::function, symbol__start_position}}};

	ASSERT_EQ(symbols, expected);
}

TEST(Symbol, General)
{
	EXPECT_NO_THROW(PPUProgram::Symbol());

	PPUProgram::Symbol sym;


	ASSERT_EQ(sym.type, PPUProgram::Symbol::Type::other);
	ASSERT_EQ(sym.coordinate, PPUMemoryBlockOnDLS());

	sym.type = PPUProgram::Symbol::Type::object;
	ASSERT_EQ(sym.type, PPUProgram::Symbol::Type::object);

	PPUMemoryBlockOnDLS coord(PPUMemoryWordOnDLS(10), PPUMemoryWordOnDLS(14));
	sym.coordinate = coord;
	ASSERT_EQ(sym.coordinate, coord);

	PPUProgram::Symbol sym1(sym.type, sym.coordinate);
	ASSERT_EQ(sym1, sym);

	PPUProgram::Symbol sym2 = sym;
	PPUProgram::Symbol sym3;

	ASSERT_EQ(sym, sym2);
	ASSERT_EQ(sym == sym3, false);

	ASSERT_NE(sym2, sym3);
	ASSERT_EQ(sym2 != sym, false);
}

TEST(Symbol, CerealizeCoverage)
{
	PPUProgram::Symbol obj1, obj2;
	obj1.type = PPUProgram::Symbol::Type::function;
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
