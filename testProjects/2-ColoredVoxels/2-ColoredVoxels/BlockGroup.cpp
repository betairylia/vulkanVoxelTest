#include "BlockGroup.h"

BlockGroup::BlockGroup()
{
}


BlockGroup::~BlockGroup()
{
}

void BlockGroup::generateTestChunk(double nowTime)
{
	memset(blockID, 0, sizeof(blockID));

	int x, y, z;
	for (x = 0;x < 32;x++)
	{
		for (y = 0;y < 32;y++)
		{
			for (z = 0;z < 32;z++)
			{
				if(16 + (z - 16) * 0.5 * sin(nowTime) > y)
					blockID[x][y][z] = 1;
			}
		}
	}
}
