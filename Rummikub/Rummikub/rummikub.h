#ifndef RUMMIKUB_H
#define RUMMIKUB_H
#include <fstream>
#include <iostream>
#include <vector>

enum Color
{
    Red,
    Green,
    Blue,
    Yellow
};

enum class Action
{
    StartGroup,
    StartRun,
    ContinueGroup,
    ContinueRun
};

struct Tile
{
    int denomination;
    Color color;
};

struct Option
{
    Option(size_t tile, Action action) : tileIndex(static_cast<int>(tile)), actionToTake(action)
    {
    }

    int tileIndex;
    Action actionToTake;
};

std::ostream& operator<<(std::ostream& os, Tile const& t);

class RummiKub
{
public:
    RummiKub();            // empty hand
    void Add(Tile const&); // add a tile to hand

    void Solve(); // solve

    // get solution - groups
    std::vector<std::vector<Tile>> GetGroups() const;
    // get solution - runs
    std::vector<std::vector<Tile>> GetRuns() const;
    // if both vectors are empty - no solution possible
private:
    bool SolveRecursive(size_t depth);
    std::vector<Option> CalculateNextOptions();
    void ApplyOption(Option option);
    void RevertOption(Option option);

    std::vector<Tile> _hand;
    std::vector<std::vector<Tile>> _groups;
    std::vector<std::vector<Tile>> _runs;

    std::vector<bool> _chosenOptions;
};

#endif
