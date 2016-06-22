#pragma once

#include <cstdlib>
#include <memory>

class BlockGroup
{
public:
	BlockGroup();
	~BlockGroup();

	void generateTestChunk(double nowTime = 4.0);

	short blockID[32][32][32];
};

