#pragma once
#include <string>
#include <vector>
#include <random>
#include <utility>

struct Location
{
	std::string mName;
	double mLatitude = 0.0;
	double mLongitude = 0.0;
};

struct Population
{
	std::vector<std::vector<int>> mMembers;
};

const double degree2Radians = 0.0174533;
std::vector<Location> readInLocations(std::string_view fileName);
std::vector<std::string> SplitSanjay(const std::string &str, char delim);
std::vector<int> InitPopulation(int numLocations, std::mt19937 &generator);
double ComputeFitness(const std::vector<int> &population, const std::vector<Location> &locations);
double HaversineDistance(const Location &a, const Location &b);
std::vector<std::pair<int, int>> GenSelections(std::mt19937 &generator, std::vector<std::pair<int, double>> &fitnesses, int popsize);
std::vector<std::pair<int, int>> GenParents(std::mt19937 &generator, std::vector<double> &probability);
std::vector<std::vector<int>> CrossOver(std::mt19937 &generator, const std::vector<std::pair<int, int>> &parents, const std::vector<std::vector<int>> &lastGen, double mutationchance);