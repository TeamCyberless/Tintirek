/*
 *	trk_string_test.cpp
 */

#include <trk_string.h>
#include <gtest/gtest.h>

TEST(TrkString, Create) {
	trk_string_t test = trk_string_create_empty();
	EXPECT_TRUE(test.data == NULL);
	EXPECT_EQ(test.length, 0);

	test = trk_string_create("HelloX");
	EXPECT_STREQ(test.data, "HelloX");
	EXPECT_EQ(test.length, 6);

	trk_string_t result;
	trk_string_assign(&result, "HelloXX");
	EXPECT_STREQ(result.data, "HelloXX");
	EXPECT_EQ(result.length, 7);

	EXPECT_EQ(1, 1);
}

int main(int argc, char** argv) {
	::testing::InitGoogleTest(&argc, argv);
	return RUN_ALL_TESTS();
}