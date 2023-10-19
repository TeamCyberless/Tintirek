/*
 *	string_test.cpp
 */

#include <trkstring.h>
#include <gtest/gtest.h>
#include "memory_leak.h"


namespace TrkCpp
{
	TEST(TrkString, DefaultConstructor) {
		MemoryLeakDetector leakDetector;
		TrkString str;
		EXPECT_EQ(str.size(), 0);
		EXPECT_STREQ(str.c_str(), "");
	}

	TEST(TrkString, CStyleStringConstructor) {
		MemoryLeakDetector leakDetector;
		TrkString str("Hello, World!");
		EXPECT_EQ(str.size(), 13);
		EXPECT_STREQ(str.c_str(), "Hello, World!");
	}

	TEST(TrkString, CopyConstructor) {
		MemoryLeakDetector leakDetector;
		TrkString original("Original");
		TrkString copy(original);
		EXPECT_EQ(copy, original);
	}

	TEST(TrkString, AssignmentOperator) {
		MemoryLeakDetector leakDetector;
		TrkString str1("Hello");
		TrkString str2("World");
		str1 = str2;
		EXPECT_EQ(str1, str2);
	}

	TEST(TrkString, ToLower) {
		MemoryLeakDetector leakDetector;
		TrkString str("Hello, World!");
		str.ToLower();
		EXPECT_STREQ(str.c_str(), "hello, world!");
	}

	TEST(TrkString, ToUpper) {
		MemoryLeakDetector leakDetector;
		TrkString str("Hello, World!");
		str.ToUpper();
		EXPECT_STREQ(str.c_str(), "HELLO, WORLD!");
	}

	TEST(TrkString, StartsWith) {
		MemoryLeakDetector leakDetector;
		TrkString str("Hello, World!");
		TrkString prefix("Hello");
		TrkString notPrefix("Hi");
		EXPECT_TRUE(str.startswith(prefix));
		EXPECT_FALSE(str.startswith(notPrefix));
	}

	TEST(TrkString, Substring) {
		MemoryLeakDetector leakDetector;
		TrkString str("Hello, World!");
		TrkString substr = str.substr(7, 5);
		EXPECT_STREQ(substr.c_str(), "World");
	}
	
	TEST(TrkString, Erase) {
		MemoryLeakDetector leakDetector;
		TrkString str("Hello, World!");
		str.erase(5, 7);
		EXPECT_STREQ(str.c_str(), "Hello!");
	}

	TEST(TrkString, StreamOperator) {
		MemoryLeakDetector leakDetector;
		TrkString str;
		str << "Hello, " << 42;
		EXPECT_STREQ(str.c_str(), "Hello, 42");
	}
}