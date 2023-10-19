/*
 *	trk_string_test.cpp
 */

#include <trk_string.h>
#include <string>
#include <gtest/gtest.h>
#include "memory_leak.h"


namespace TrkCore
{
	TEST(TrkString, Create) {
		MemoryLeakDetector leakDetector;

		trk_string_t* empty = &trk_string_create_empty();
		EXPECT_TRUE(empty->data == NULL);
		EXPECT_EQ(empty->length, 0);
		trk_string_destroy(empty);
	}

	TEST(TrkString, CreateDetailed) {
		MemoryLeakDetector leakDetector;

		trk_string_t* new_str = &trk_string_create("HelloX");
		EXPECT_STREQ(new_str->data, "HelloX");
		EXPECT_EQ(new_str->length, 6);

		trk_string_t* duplicate = &trk_string_duplicate(new_str);
		EXPECT_STREQ(duplicate->data, "HelloX");
		EXPECT_EQ(duplicate->length, 6);
		trk_string_destroy(new_str);
		trk_string_destroy(duplicate);

		char between_test[] = "This is a test string.";
		trk_string_t* between = &trk_string_between(between_test + 5, between_test + 10);
		EXPECT_STREQ(between->data, "is a ");
		EXPECT_EQ(between->length, 5);
		trk_string_destroy(between);

		trk_string_t* single_char = &trk_string_create_char('a');
		EXPECT_STREQ(single_char->data, "a");
		EXPECT_EQ(single_char->length, 1);
		trk_string_destroy(single_char);

		trk_string_t* reserved = &trk_string_reserve(10);
		EXPECT_STREQ(reserved->data, "\0");
		EXPECT_EQ(reserved->length, 0);
		trk_string_destroy(reserved);
	}

	TEST(TrkString, LowerUpper) {
		MemoryLeakDetector leakDetector;
		trk_string_t* str = &trk_string_create("Utilities");

		trk_string_to_lower(str);
		EXPECT_STREQ(str->data, "utilities");
		EXPECT_EQ(str->length, 9);

		trk_string_to_upper(str);
		EXPECT_STREQ(str->data, "UTILITIES");
		EXPECT_EQ(str->length, 9);

		trk_string_destroy(str);
	}

	TEST(TrkString, CharacterUtilities) {
		MemoryLeakDetector leakDetector;

		trk_string_t* str = &trk_string_create("XYZ");
		EXPECT_TRUE(trk_string_begin(str) == &str->data[0]);
		EXPECT_TRUE(trk_string_end(str) == &str->data[str->length]);
		EXPECT_TRUE(strcmp(trk_string_create_char(trk_string_first(str)).data, "X") == 0);
		EXPECT_TRUE(strcmp(trk_string_create_char(trk_string_last(str)).data, "Z") == 0);
		EXPECT_TRUE(trk_string_starts_with(str, "XY"));
		EXPECT_TRUE(trk_string_size(str), 3);
		EXPECT_TRUE(trk_string_find(str, "W", 0), npos);
		EXPECT_TRUE(trk_string_find(str, "X", 1), npos);
		EXPECT_TRUE(trk_string_find(str, "Z", 0), 2);
		trk_string_destroy(str);
	}

	TEST(TrkString, FindFirstOrLastNotOf) {
		MemoryLeakDetector leakDetector;

		trk_string_t* str = &trk_string_create("ABCDEFGHIJKL");
		EXPECT_EQ(trk_string_find_first_not_of(str, "XYZ"), 0);
		EXPECT_EQ(std::string(str->data).find_first_not_of("XYZ"), trk_string_find_first_not_of(str, "XYZ"));
		EXPECT_EQ(trk_string_find_first_not_of(str, "ABC"), 3);
		EXPECT_EQ(std::string(str->data).find_first_not_of("ABC"), trk_string_find_first_not_of(str, "ABC"));
		EXPECT_EQ(trk_string_find_first_not_of(str, "EFG"), 0);
		EXPECT_EQ(std::string(str->data).find_first_not_of("EFG"), trk_string_find_first_not_of(str, "EFG"));
		EXPECT_EQ(trk_string_find_last_not_of(str, "XYZ"), str->length - 1);
		EXPECT_EQ(std::string(str->data).find_last_not_of("XYZ"), trk_string_find_last_not_of(str, "XYZ"));
		EXPECT_EQ(trk_string_find_last_not_of(str, "EFG"), str->length - 1);
		EXPECT_EQ(std::string(str->data).find_last_not_of("EFG"), trk_string_find_last_not_of(str, "EFG"));
		EXPECT_EQ(trk_string_find_last_not_of(str, "JKL"), 8);
		EXPECT_EQ(std::string(str->data).find_last_not_of("JKL"), trk_string_find_last_not_of(str, "JKL"));
		trk_string_destroy(str);
	}

	TEST(TrkString, Substring) {
		MemoryLeakDetector leakDetector;
		trk_string_t* str = &trk_string_create("SUBSTR_TEST"), * returned;

		returned = &trk_string_substr(str, 6, npos);
		EXPECT_STREQ(returned->data, "_TEST");
		EXPECT_EQ(returned->length, 5);
		trk_string_destroy(returned);

		returned = &trk_string_substr(str, 7, 4);
		EXPECT_STREQ(returned->data, "TEST");
		EXPECT_EQ(returned->length, 4);
		trk_string_destroy(returned);
	}

	TEST(TrkString, EraseString) {
		MemoryLeakDetector leakDetector;
		trk_string_t* str = &trk_string_create("SUBSTR_TEST");

		trk_string_erase(str, 6, 1);
		EXPECT_STREQ(str->data, "SUBSTRTEST");
		EXPECT_EQ(str->length, 10);
		trk_string_erase(str, 0, 6);
		EXPECT_STREQ(str->data, "TEST");
		EXPECT_EQ(str->length, 4);
		trk_string_destroy(str);

		str = &trk_string_create("TeamCyberless");
		trk_string_append_cstr(str, trk_string_create(" Tintirek"));
		EXPECT_STREQ(str->data, "TeamCyberless Tintirek");
		EXPECT_EQ(str->length, 22);
		trk_string_destroy(str);
	}
}