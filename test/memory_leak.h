/*
 *	memory_leak.h
 */


#include <crtdbg.h>
#include <sstream>
#include <gtest/gtest.h>


class MemoryLeakDetector {
public:
	MemoryLeakDetector() {
		_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
		_CrtMemCheckpoint(&memState_);
	}

	~MemoryLeakDetector() {
		_CrtMemState stateNow, stateDiff;
		_CrtMemCheckpoint(&stateNow);
		int diffResult = _CrtMemDifference(&stateDiff, &memState_, &stateNow);
		if (diffResult)
			reportFailure(stateDiff);
	}
private:
	void reportFailure(_CrtMemState& stateDiff) {
		std::stringstream info;
		info << "Memory leak detected:\n\n";

		if (stateDiff.lCounts[_CLIENT_BLOCK] > 0) {
			info << "_CLIENT_BLOCK count: " << stateDiff.lCounts[_CLIENT_BLOCK] << "\n";
			info << "_NORMAL_BLOCK leaks: " << stateDiff.lSizes[_CLIENT_BLOCK] << "\n";
		}

		if (stateDiff.lCounts[_NORMAL_BLOCK] > 0) {
			info << "_NORMAL_BLOCK count: " << stateDiff.lCounts[_NORMAL_BLOCK] << "\n";
			info << "_NORMAL_BLOCK leaks: " << stateDiff.lSizes[_NORMAL_BLOCK] << "\n";

		}

		if (stateDiff.lCounts[_CRT_BLOCK] > 0) {
			info << "_CRT_BLOCK count: " << stateDiff.lCounts[_CRT_BLOCK] << "\n";
			info << "_NORMAL_BLOCK leaks: " << stateDiff.lSizes[_CRT_BLOCK] << "\n";
		}

		if (stateDiff.lCounts[_IGNORE_BLOCK] > 0) {
			info << "_IGNORE_BLOCK count: " << stateDiff.lCounts[_IGNORE_BLOCK] << "\n";
			info << "_NORMAL_BLOCK leaks: " << stateDiff.lSizes[_IGNORE_BLOCK] << "\n";
		}

		info << "Total count: " << stateDiff.lTotalCount;

		FAIL() << info.str();
	}
	_CrtMemState memState_;
};