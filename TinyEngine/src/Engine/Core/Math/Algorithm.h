#pragma once

#include <math.h>
#include <vector>
#include <random>
#include <time.h>
#include <glm/glm.hpp>
#include <iostream>

namespace Engine
{
	class Random
	{
	public:
		static uint32_t Rand()
		{
			auto seed = time(0);
			static std::mt19937 rnd(seed);
			return rnd();
		}

		static uint32_t Range(uint32_t L, uint32_t R)
		{
			return uint32_t((1.0f * Rand() / UINT_MAX) * (R - L) + L);
		}

		static uint32_t Range(uint32_t max)
		{
			return Range(0, max);
		}

		/// <summary>
		/// �ӵ�λԲ�л�ȡ�����
		/// </summary>
		static glm::vec2 InsideUnitCircle()
		{
			double theta = Range(360 * 1000) / 1000.0;
			double r = Range(1000) / 1000.0;
			return { (sqrt(r) * cos(theta)), (sqrt(r) * sin(theta)) };
		}
	};

	class Algorithm
	{
	public:
		static std::vector<glm::vec2> FastPossionSampling(float width, float height, float radius)
		{			
			std::vector<glm::vec2> list;
			const int32_t max_retry = 30;
				
			//����CellSize��һ��Cell�ڲ����ܳ��ֶ���㣬�������������radius��Cell�ڲ������Ϊ�Խ��ߣ������radiusΪ�Խ��߼���CellSize
			auto cellSize = radius / std::sqrt(2);
				
			//�����򻮷�Ϊgrids��-1��ʾcell��û�е�
			int32_t cols = std::ceil(width / cellSize);
			int32_t rows = std::ceil(height / cellSize);
			std::vector<std::vector<int32_t>> grids;
			grids.resize(rows);
			for (auto& row : grids)
			{
				row.resize(cols, -1);
			}

			// ���ѡȡ��ʼ��
			auto start = glm::vec2{ Random::Range(width), Random::Range(height) };
			int32_t col = std::floor(start.x / cellSize);
			int32_t row = std::floor(start.y / cellSize);

			auto startIndex = list.size();
			list.push_back(start);
			grids[row][col] = startIndex;

			std::vector<int> activeList;
			activeList.push_back(startIndex);

			float r = radius;
			while (activeList.size() > 0)
			{
				// ���Ѿ��еĲ���������ȡһ����, ���������Χ�����µĲ�����
				auto key = activeList[Random::Range(activeList.size())];

				auto point = list[key];
				bool found = false;

				for (int32_t i = 0; i < max_retry; i++)
				{
					auto direct = Random::InsideUnitCircle();
					if (direct.x == 0 && direct.y == 0)
						continue;

					// ��ԭ�еĲ����� point ����һ������ [r, 2r) �������������Ϊ�µĲ�����
					auto new_point = point + ((glm::normalize(direct) * r) + (direct * r));
					if ((new_point.x < 0 || new_point.x >= width) || (new_point.y < 0 || new_point.y >= height))
						continue;


					col = std::floor(new_point.x / cellSize);
					row = std::floor(new_point.y / cellSize);
					if (grids[row][col] != -1) 
						continue;


					//����²�������Χ�Ƿ����С��radius�ĵ�
					bool success = true;
					int32_t min_r = std::floor((new_point.y - radius) / cellSize);
					int32_t max_r = std::floor((new_point.y + radius) / cellSize);
					int32_t min_c = std::floor((new_point.x - radius) / cellSize);
					int32_t max_c = std::floor((new_point.x + radius) / cellSize);
					[&]() {
						for (int32_t r = min_r; r <= max_r; r++)
						{
							if (r < 0 || r >= rows)
							{
								continue;
							}
							for (int32_t c = min_c; c <= max_c; c++)
							{
								if (c < 0 || c >= cols)
								{
									continue;
								}
								int32_t point_key = grids[r][c];
								if (point_key != -1)
								{
									auto round_point = list[point_key];
									auto dist = glm::length(round_point - new_point);
									if (dist < radius)
									{
										success = false;
										return;		//��������ѭ��
									}
								}
							}
						}
					}();

					// �²�����ɹ�����
					if (success)
					{
						auto new_point_key = list.size();
						list.push_back(new_point);
						grids[row][col] = new_point_key;
						activeList.push_back(new_point_key);
						found = true;
						break;
					}
				}

				if (!found)
				{
					auto iter = std::find(activeList.begin(), activeList.end(), key);
					if (iter != activeList.end())
					{
						activeList.erase(iter);
					}
				}
			}
			return list; 			
		}
	};
}