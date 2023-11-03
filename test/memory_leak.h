/*
 *	memory_leak.h
 */


#include <crtdbg.h>
#include <sstream>
#include <gtest/gtest.h>


class MemoryLeakDetector {
public:
	MemoryLeakDetector() {
		_CrtMemCheckpoint(&memState_);
	}

	~MemoryLeakDetector() {
		_CrtMemState stateNow, stateDiff;
		_CrtMemCheckpoint(&stateNow);
		int diffResult = _CrtMemDifference(&stateDiff, &memState_, &stateNow);
		if (diffResult && stateDiff.lCounts[_NORMAL_BLOCK] > 0)
			reportFailure(stateDiff.lSizes[_NORMAL_BLOCK]);
	}
private:
	void reportFailure(unsigned int unfreedBytes) {
		FAIL() << "Memory leak of " << unfreedBytes << " byte(s) detected.";
	}

	_CrtMemState memState_;
};