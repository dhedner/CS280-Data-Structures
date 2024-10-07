/*****************************************************************
 * @file   rummikub.cpp
 * @brief  
 * @author david.hedner@digipen.edu
 * @date   February 2024
 * 
 * @copyright © 2024 DigiPen (USA) Corporation.
 *****************************************************************/
#include "rummikub.h"

#include <algorithm>

std::ostream& operator<<(std::ostream& os, Tile const& t)
{
    os << "{ " << t.denomination << ",";
    switch (t.color)
    {
    case Red:
        os << "R";
        break;
    case Green:
        os << "G";
        break;
    case Blue:
        os << "B";
        break;
    case Yellow:
        os << "Y";
        break;
    }
    os << " }";
    return os;
}

RummiKub::RummiKub()
{
}

void RummiKub::Add(Tile const& tile)
{
    _hand.push_back(tile);
}

void RummiKub::Solve()
{
    _chosenOptions.resize(_hand.size());

    std::fill(_chosenOptions.begin(), _chosenOptions.end(), true);

    SolveRecursive(0);
}

std::vector<std::vector<Tile>> RummiKub::GetGroups() const
{
    return _groups;
}

std::vector<std::vector<Tile>> RummiKub::GetRuns() const
{
    return _runs;
}

bool RummiKub::SolveRecursive(size_t depth)
{
    if (depth == _hand.size() && (_groups.empty() || _groups.back().size() >= 3) &&
        (_runs.empty() || _runs.back().size() >= 3))
    {
        return true;
    }

    std::vector<Option> availableOptions = CalculateNextOptions();

    for (const auto& option : availableOptions)
    {
        ApplyOption(option);

        if (SolveRecursive(depth + 1))
        {
            return true;
        }

        RevertOption(option);
    }

    return false;
}

std::vector<Option> RummiKub::CalculateNextOptions()
{
    std::vector<Option> nextOptions;

    for (size_t i = 0; i < _hand.size(); i++)
    {
        if (_chosenOptions[i] == true)
        {
            Tile currentTile = _hand[i];

            if (_groups.empty() || (_groups.back().size() <= 4 && _groups.back().size() >= 3))
            {
                nextOptions.emplace_back(i, Action::StartGroup);
            }

            if (!_groups.empty() && _groups.back().size() <= 3 &&
                currentTile.denomination == _groups.back()[0].denomination &&
                std::all_of(
                    _groups.back().begin(), _groups.back().end(), [currentTile](Tile groupTile) {
                        return groupTile.color != currentTile.color;
                    }))
            {
                nextOptions.emplace_back(i, Action::ContinueGroup);
            }

            if (_runs.empty() || (_runs.back().size() <= 4 && _runs.back().size() >= 3))
            {
                nextOptions.emplace_back(i, Action::StartRun);
            }
            if (!_runs.empty() && _runs.back().size() <= 3 &&
                currentTile.color == _runs.back()[0].color &&
                currentTile.denomination == _runs.back().back().denomination + 1)
            {
                nextOptions.emplace_back(i, Action::ContinueRun);
            }
        }
    }

    return nextOptions;
}

void RummiKub::ApplyOption(Option option)
{
    _chosenOptions[option.tileIndex] = false;

    switch (option.actionToTake)
    {
    case Action::StartGroup:
        _groups.emplace_back(std::vector<Tile>{_hand[option.tileIndex]});
        break;
    case Action::ContinueGroup:
        _groups.back().emplace_back(_hand[option.tileIndex]);
        break;
    case Action::StartRun:
        _runs.emplace_back(std::vector<Tile>{_hand[option.tileIndex]});
        break;
    case Action::ContinueRun:
        _runs.back().emplace_back(_hand[option.tileIndex]);
        break;
    }
}

void RummiKub::RevertOption(Option option)
{
    _chosenOptions[option.tileIndex] = true;

    switch (option.actionToTake)
    {
    case Action::StartGroup:
        _groups.pop_back();
        break;
    case Action::ContinueGroup:
        _groups.back().pop_back();
        break;
    case Action::StartRun:
        _runs.pop_back();
        break;
    case Action::ContinueRun:
        _runs.back().pop_back();
        break;
    }
}
