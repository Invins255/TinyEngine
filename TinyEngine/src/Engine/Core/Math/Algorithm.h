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
		/// 从单位圆中获取随机点
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
				
			//计算CellSize，一个Cell内部不能出现多个点，且两点间距离大于radius，Cell内部最长距离为对角线，因此以radius为对角线计算CellSize
			auto cellSize = radius / std::sqrt(2);
				
			//将区域划分为grids，-1表示cell内没有点
			int32_t cols = std::ceil(width / cellSize);
			int32_t rows = std::ceil(height / cellSize);
			std::vector<std::vector<int32_t>> grids;
			grids.resize(rows);
			for (auto& row : grids)
			{
				row.resize(cols, -1);
			}

			// 随机选取初始点
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
				// 在已经有的采样集合中取一个点, 在这个点周围生成新的采样点
				auto key = activeList[Random::Range(activeList.size())];

				auto point = list[key];
				bool found = false;

				for (int32_t i = 0; i < max_retry; i++)
				{
					auto direct = Random::InsideUnitCircle();
					if (direct.x == 0 && direct.y == 0)
						continue;

					// 给原有的采样点 point 加上一个距离 [r, 2r) 的随机向量，成为新的采样点
					auto new_point = point + ((glm::normalize(direct) * r) + (direct * r));
					if ((new_point.x < 0 || new_point.x >= width) || (new_point.y < 0 || new_point.y >= height))
						continue;


					col = std::floor(new_point.x / cellSize);
					row = std::floor(new_point.y / cellSize);
					if (grids[row][col] != -1) 
						continue;


					//检查新采样点周围是否存在小于radius的点
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
										return;		//跳出两层循环
									}
								}
							}
						}
					}();

					// 新采样点成功采样
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