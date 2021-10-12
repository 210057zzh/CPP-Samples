#include "TSP.h"
#include <fstream>
#include <algorithm>
#include <cmath>
#include <iterator>
#include <numeric>
#include <iostream>

/* io */
std::vector<std::string> SplitSanjay(const std::string &str, char delim)
{
    std::vector<std::string> retVal;
    size_t start = 0;
    size_t delimLoc = str.find_first_of(delim, start);
    while (delimLoc != std::string::npos)
    {
        retVal.emplace_back(str.substr(start, delimLoc - start));
        start = delimLoc + 1;
        delimLoc = str.find_first_of(delim, start);
    }
    retVal.emplace_back(str.substr(start));
    return retVal;
}

std::vector<Location> readInLocations(std::string_view fileName)
{
    std::ifstream file(fileName.data());
    std::string line;
    std::vector<Location> result;
    while (std::getline(file, line))
    {
        auto split = SplitSanjay(line, ',');
        Location current;
        current.mName = split[0];
        current.mLatitude = stod(split[1]) * degree2Radians;
        current.mLongitude = stod(split[2]) * degree2Radians;
        result.emplace_back(std::move(current));
    }
    return result;
}
/* end io */

std::vector<int> InitPopulation(int numLocations, std::mt19937 &generator)
{
    std::vector<int> result(numLocations);
    std::iota(std::begin(result), std::end(result), 0);
    std::shuffle(std::begin(result) + 1, std::end(result), generator);
    return result;
}

double ComputeFitness(const std::vector<int> &population, const std::vector<Location> &locations)
{
    std::vector<double> distances;
    std::adjacent_difference(std::begin(population), std::end(population), std::back_inserter(distances), [&locations](int a, int b)
                             { return HaversineDistance(locations[a], locations[b]); });
    distances.push_back(HaversineDistance(locations[population.back()], locations[0]));
    return std::accumulate(std::begin(distances), std::end(distances), 0.0, [](auto a, auto b)
                           { return a + b; });
}

double HaversineDistance(const Location &a, const Location &b)
{
    auto dlon = b.mLongitude - a.mLongitude;
    auto dlat = b.mLatitude - a.mLatitude;
    auto apple = std::pow(std::sin(dlat / 2.0), 2) + std::cos(a.mLatitude) * std::cos(b.mLatitude) * std::pow(std::sin(dlon / 2.0), 2);
    auto c = 2 * std::atan2(std::sqrt(apple), std::sqrt(1 - apple));
    return 3961.0 * c;
}

std::vector<std::pair<int, int>> GenSelections(std::mt19937 &generator, std::vector<std::pair<int, double>> &fitnesses, int popsize)
{
    std::sort(std::begin(fitnesses), std::end(fitnesses), [](auto &a, auto &b)
              { return a.second < b.second; });
    std::vector<double> probability(fitnesses.size());
    size_t i = 0;
    std::for_each(std::begin(fitnesses), std::end(fitnesses), [popsize, &i, &probability, &fitnesses](auto &a)
                  {
                      double scale = 1.0;
                      if (i > 1 && i <= fitnesses.size() / 2 - 1)
                      {
                          scale = 3.0;
                      }
                      else if (i <= 1)
                      {
                          scale = 6.0;
                      }
                      ++i;
                      probability[a.first] = 1.0 * scale / popsize;
                  });
    double sum = std::accumulate(std::begin(probability), std::end(probability), 0.0);
    std::transform(std::begin(probability), std::end(probability), std::begin(probability), [sum](auto &a)
                   { return a / sum; });
    return GenParents(generator, probability);
}

std::vector<std::pair<int, int>> GenParents(std::mt19937 &generator, std::vector<double> &probability)
{
    std::vector<std::pair<int, int>> parents(probability.size());
    std::generate(std::begin(parents), std::end(parents), [&generator, &probability]()
                  {
                      std::uniform_real_distribution<double> urd(0.0, 1.0);
                      double threshold = urd(generator);
                      double sum = 0.0;
                      int firstParent = 0;
                      auto result = std::find_if(std::begin(probability), std::end(probability), [&firstParent, &sum, threshold](auto &a)
                                                 {
                                                     sum += a;
                                                     if (sum >= threshold)
                                                     {
                                                         return true;
                                                     }
                                                     firstParent++;
                                                     return false;
                                                 });
                      threshold = urd(generator);
                      sum = 0.0;
                      int secondParent = 0;
                      result = std::find_if(std::begin(probability), std::end(probability), [&secondParent, &sum, threshold](auto &a)
                                            {
                                                sum += a;
                                                if (sum >= threshold)
                                                {
                                                    return true;
                                                }
                                                secondParent++;
                                                return false;
                                            });
                      return std::make_pair(firstParent, secondParent);
                  });
    return parents;
}

std::vector<std::vector<int>> CrossOver(std::mt19937 &generator, const std::vector<std::pair<int, int>> &parents, const std::vector<std::vector<int>> &lastGen, double mutationchance)
{
    std::vector<std::vector<int>> thisGen;
    std::transform(std::begin(parents), std::end(parents), std::back_inserter(thisGen), [&generator, &lastGen, mutationchance](auto &parent)
                   {
                       std::uniform_int_distribution uid1(1, static_cast<int>(lastGen[0].size() - 2));
                       auto crossIndex = uid1(generator);
                       std::uniform_int_distribution uid3(0, 1);
                       auto order = uid3(generator);
                       std::vector<int> child;
                       if (order)
                       {
                           std::copy_n(std::begin(lastGen[parent.first]), crossIndex + 1, std::back_inserter(child));
                           std::copy_if(std::begin(lastGen[parent.second]), std::end(lastGen[parent.second]), std::back_inserter(child), [&child](int a)
                                        { return std::find(std::begin(child), std::end(child), a) == std::end(child); });
                       }
                       else
                       {
                           std::copy_n(std::begin(lastGen[parent.second]), crossIndex + 1, std::back_inserter(child));
                           std::copy_if(std::begin(lastGen[parent.first]), std::end(lastGen[parent.first]), std::back_inserter(child), [&child](int a)
                                        { return std::find(std::begin(child), std::end(child), a) == std::end(child); });
                       }
                       std::uniform_real_distribution<double> urd;
                       auto chance = urd(generator);
                       if (chance <= mutationchance)
                       {
                           std::uniform_int_distribution uid8(1, static_cast<int>(lastGen[0].size() - 1));
                           auto firstIndex = uid8(generator);
                           auto secondIndex = uid8(generator);
                           std::swap(child[firstIndex], child[secondIndex]);
                       }
                       return child;
                   });
    return thisGen;
}
