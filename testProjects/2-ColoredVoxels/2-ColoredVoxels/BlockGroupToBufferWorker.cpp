#include "BlockGroupToBufferWorker.h"



BlockGroupToBufferWorker::BlockGroupToBufferWorker()
{
}


BlockGroupToBufferWorker::~BlockGroupToBufferWorker()
{
}

void BlockGroupToBufferWorker::workCreate(VkDevice device, Renderable & renderable, BlockGroup & blockGroup)
{
	//todo
}

void BlockGroupToBufferWorker::workUpdate(VkDevice device, Renderable & renderable, BlockGroup & blockGroup)
{
	vector<Vertex> vertices;
	vector<uint32_t> indices;
	int count = 0;

	int x, y, z;
	for (x = 0;x < 32;x++)
	{
		for (y = 0;y < 32;y++)
		{
			for (z = 0;z < 32;z++)
			{
				if (blockGroup.blockID[x][y][z] == 0)
				{
					continue;
				}

				if (x == 0 || blockGroup.blockID[x - 1][y][z] == 0)//todo: == 0 => is not solid block
				{
					//todo: more attributes
					vertices.push_back({ (float)x + 0, (float)y + 0, (float)z + 0, 1, 1, 1, 1, 1 });
					vertices.push_back({ (float)x + 0, (float)y + 1, (float)z + 0, 1, 1, 1, 1, 1 });
					vertices.push_back({ (float)x + 0, (float)y + 1, (float)z + 1, 1, 1, 1, 1, 1 });
					vertices.push_back({ (float)x + 0, (float)y + 0, (float)z + 1, 1, 1, 1, 1, 1 });

					indices.push_back(count + 0);
					indices.push_back(count + 1);
					indices.push_back(count + 2);
					indices.push_back(count + 0);
					indices.push_back(count + 2);
					indices.push_back(count + 3);

					count += 4;
				}

				if (x == 31 || blockGroup.blockID[x + 1][y][z] == 0)//todo: == 0 => is not solid block
				{
					//todo: more attributes
					vertices.push_back({ (float)x + 1, (float)y + 0, (float)z + 0, 1, 1, 1, 1, 1 });
					vertices.push_back({ (float)x + 1, (float)y + 0, (float)z + 1, 1, 1, 1, 1, 1 });
					vertices.push_back({ (float)x + 1, (float)y + 1, (float)z + 1, 1, 1, 1, 1, 1 });
					vertices.push_back({ (float)x + 1, (float)y + 1, (float)z + 0, 1, 1, 1, 1, 1 });

					indices.push_back(count + 0);
					indices.push_back(count + 1);
					indices.push_back(count + 2);
					indices.push_back(count + 0);
					indices.push_back(count + 2);
					indices.push_back(count + 3);

					count += 4;
				}

				if (y == 0 || blockGroup.blockID[x][y - 1][z] == 0)//todo: == 0 => is not solid block
				{
					//todo: more attributes
					vertices.push_back({ (float)x + 0, (float)y + 0, (float)z + 0, 1, 1, 1, 1, 1 });
					vertices.push_back({ (float)x + 0, (float)y + 0, (float)z + 1, 1, 1, 1, 1, 1 });
					vertices.push_back({ (float)x + 1, (float)y + 0, (float)z + 1, 1, 1, 1, 1, 1 });
					vertices.push_back({ (float)x + 1, (float)y + 0, (float)z + 0, 1, 1, 1, 1, 1 });

					indices.push_back(count + 0);
					indices.push_back(count + 1);
					indices.push_back(count + 2);
					indices.push_back(count + 0);
					indices.push_back(count + 2);
					indices.push_back(count + 3);

					count += 4;
				}

				if (y == 31 || blockGroup.blockID[x][y + 1][z] == 0)//todo: == 0 => is not solid block
				{
					//todo: more attributes
					vertices.push_back({ (float)x + 0, (float)y + 1, (float)z + 0, 1, 1, 1, 1, 1 });
					vertices.push_back({ (float)x + 1, (float)y + 1, (float)z + 0, 1, 1, 1, 1, 1 });
					vertices.push_back({ (float)x + 1, (float)y + 1, (float)z + 1, 1, 1, 1, 1, 1 });
					vertices.push_back({ (float)x + 0, (float)y + 1, (float)z + 1, 1, 1, 1, 1, 1 });

					indices.push_back(count + 0);
					indices.push_back(count + 1);
					indices.push_back(count + 2);
					indices.push_back(count + 0);
					indices.push_back(count + 2);
					indices.push_back(count + 3);

					count += 4;
				}

				if (z == 0 || blockGroup.blockID[x][y][z - 1] == 0)//todo: == 0 => is not solid block
				{
					//todo: more attributes
					vertices.push_back({ (float)x + 0, (float)y + 0, (float)z + 0, 1, 1, 1, 1, 1 });
					vertices.push_back({ (float)x + 1, (float)y + 0, (float)z + 0, 1, 1, 1, 1, 1 });
					vertices.push_back({ (float)x + 1, (float)y + 1, (float)z + 0, 1, 1, 1, 1, 1 });
					vertices.push_back({ (float)x + 0, (float)y + 1, (float)z + 0, 1, 1, 1, 1, 1 });

					indices.push_back(count + 0);
					indices.push_back(count + 1);
					indices.push_back(count + 2);
					indices.push_back(count + 0);
					indices.push_back(count + 2);
					indices.push_back(count + 3);

					count += 4;
				}

				if (z == 31 || blockGroup.blockID[x][y][z + 1] == 0)//todo: == 0 => is not solid block
				{
					//todo: more attributes
					vertices.push_back({ (float)x + 0, (float)y + 0, (float)z + 1, 1, 1, 1, 1, 1 });
					vertices.push_back({ (float)x + 0, (float)y + 1, (float)z + 1, 1, 1, 1, 1, 1 });
					vertices.push_back({ (float)x + 1, (float)y + 1, (float)z + 1, 1, 1, 1, 1, 1 });
					vertices.push_back({ (float)x + 1, (float)y + 0, (float)z + 1, 1, 1, 1, 1, 1 });

					indices.push_back(count + 0);
					indices.push_back(count + 1);
					indices.push_back(count + 2);
					indices.push_back(count + 0);
					indices.push_back(count + 2);
					indices.push_back(count + 3);

					count += 4;
				}
			}
		}
	}

	uint32_t indicesCount = indices.size();

	//todo:uniform
	renderable.UpdateVertexBuffer(device, (const void*)(vertices.data()), vertices.size() * sizeof(Vertex));
	renderable.UpdateIndexBuffer(device, (const void*)(indices.data()), indicesCount * sizeof(uint32_t));

	renderable.indicesCount = indicesCount;
}
