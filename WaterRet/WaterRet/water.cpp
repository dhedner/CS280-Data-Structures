/*****************************************************************
 * @file   water.cpp
 * @brief  Implement the waterret function
 * @author david.hedner@digipen.edu
 * @date   April 2024
 * 
 * @copyright © 2024 DigiPen (USA) Corporation.
 *****************************************************************/
#include "water.h"
#include <algorithm>
#include <fstream>
#include <iostream>
#include <queue>
#include <vector>

long int waterret(char const* filename)
{
    // Open the file
    std::ifstream file(filename);
    if (!file.is_open())
    {
        std::cerr << "Error: could not open file " << filename << std::endl;
    }

    int rows;
    int cols;

    file >> rows >> cols;

    std::vector<std::vector<int>> grid(rows, std::vector<int>(cols));

    for (int i = 0; i < rows; i++)
    {
        for (int j = 0; j < cols; j++)
        {
            file >> grid[i][j];
        }
    }

    file.close();

    std::vector<std::vector<int>> water(rows, std::vector<int>(cols, 0));
    std::vector<std::vector<int>> cutoff(rows, std::vector<int>(cols, std::numeric_limits<int>::max()));
    std::queue<std::pair<int, int>> q;

    // Initialize the boundary cells of the grid
    for (int i = 1; i < rows - 1; i++)
    {
        cutoff[i][0] = grid[i][0];
        cutoff[i][cols - 1] = grid[i][cols - 1];
        q.push({i, 0});
        q.push({i, cols - 1});
    }
    for (int j = 0; j < cols; j++)
    {
        cutoff[0][j] = grid[0][j];
        cutoff[rows - 1][j] = grid[rows - 1][j];
        q.push({0, j});
        q.push({rows - 1, j});
    }

    // Directions for moving up, down, left, right
    const std::vector<int> dRow = {-1, 1, 0, 0};
    const std::vector<int> dCol = {0, 0, -1, 1};

    while (!q.empty())
    {
        auto [r, c] = q.front();
        q.pop();

        for (int i = 0; i < 4; i++)
        {
            int newRow = r + dRow[i];
            int newCol = c + dCol[i];

            // Check if the new cell is out of bounds
            if (newRow < 0 || newRow >= rows || newCol < 0 || newCol >= cols)
            {
                continue;
            }

            int newCutoff = std::max(cutoff[r][c], grid[newRow][newCol]);
            if (newCutoff < cutoff[newRow][newCol])
            {
                cutoff[newRow][newCol] = newCutoff;
                if (grid[newRow][newCol] > newCutoff)
                {
                    water[newRow][newCol] = 0;
                }
                else
                {
                    water[newRow][newCol] = newCutoff - grid[newRow][newCol];
                }
                q.push({newRow, newCol});
            }
        }
    }

    long int totalWater = 0;
    for (int i = 0; i < rows; i++)
    {
        for (int j = 0; j < cols; j++)
        {
            totalWater += water[i][j];
        }
    }

\.    return totalWater;
}