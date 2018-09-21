#include <gtest/gtest.h>

#include "halco/hicann-dls/v2/coordinates.h"
#include "haldls/v2/board.h"
#include "haldls/v2/capmem.h"
#include "haldls/v2/chip.h"
#include "haldls/v2/playback.h"

using namespace halco::common;
using namespace halco::hicann_dls::v2;
using namespace haldls::v2;

template <class T>
class CommonSerializationTests : public ::testing::Test {};

typedef ::testing::Types<Board,
                         DAC,
                         CapMem,
                         CapMemCell,
                         CapMemConfig,
                         FlyspiConfig,
                         FlyspiException,
                         SpikeRouter> SerializableTypes;

TYPED_TEST_CASE(CommonSerializationTests, SerializableTypes);

TYPED_TEST(CommonSerializationTests, IsDefaultConstructible)
{
	TypeParam obj;
	static_cast<void>(&obj);
}

TYPED_TEST(CommonSerializationTests, IsAssignable)
{
	TypeParam obj1, obj2;
	obj1 = obj2;
}

TYPED_TEST(CommonSerializationTests, HasSerialization)
{
	TypeParam obj1, obj2;

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
	// This does only test that Serialization does not insert wrong values
	// but does not check coverage since both instances are default constructed.
	// Coverage check is done in each container's test file.
	ASSERT_EQ(obj2, obj1);
}
