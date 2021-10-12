#include "SrcMain.h"
#include "TSP.h"
#include <iostream>
#include <random>
#include "TSP.h"
#include <fstream>
#include <algorithm>
#include <utility>

void ProcessCommandArgs(int argc, const char *argv[])
{
	// TODO
	std::string inputFile(argv[1]);
	int popsize = atoi(argv[2]);
	int generations = atoi(argv[3]);
	double mutationchance = atoi(argv[4]) / 100.0;
	int seed = atoi(argv[5]);
	std::mt19937 randomGenerator(static_cast<unsigned int>(seed));
	auto locations = readInLocations(inputFile);
	Population population;
	population.mMembers.resize(popsize);
	auto numLocations = locations.size();
	std::generate(std::begin(population.mMembers), std::end(population.mMembers), [&randomGenerator, numLocations]()
				  { return InitPopulation(numLocations, randomGenerator); });
	std::ofstream outputfile("log.txt");
	/* io */
	outputfile << "INITIAL POPULATION:" << std::endl;
	for (auto &pop : population.mMembers)
	{
		for (size_t i = 0; i < pop.size(); i++)
		{
			outputfile << pop[i];
			if (i != pop.size() - 1)
			{
				outputfile << ',';
			}
		}
		outputfile << std::endl;
	}
	/* end io */
	for (int generation = 1; generation <= generations; generation++)
	{
		std::vector<std::pair<int, double>> fitnesses;
		auto i = 0;
		std::transform(std::begin(population.mMembers), std::end(population.mMembers), std::back_inserter(fitnesses), [&locations, &i](auto &pop)
					   { return std::make_pair(i++, ComputeFitness(pop, locations)); });
		/* io */
		outputfile << "FITNESS:" << std::endl;
		for (auto &fitness : fitnesses)
		{
			outputfile << fitness.first << ':' << fitness.second << std::endl;
		}
		/* end io */
		auto parents = GenSelections(randomGenerator, fitnesses, popsize);
		/* io */
		outputfile << "SELECTED PAIRS:" << std::endl;
		for (auto &parent : parents)
		{
			outputfile << '(' << parent.first << ',' << parent.second << ')' << std::endl;
		}
		/* end io */
		population.mMembers = CrossOver(randomGenerator, parents, population.mMembers, mutationchance);
		/* io */
		outputfile << "GENERATION: " << generation << std::endl;
		for (auto &pop : population.mMembers)
		{
			for (size_t i = 0; i < pop.size(); i++)
			{
				outputfile << pop[i];
				if (i != pop.size() - 1)
				{
					outputfile << ',';
				}
			}
			outputfile << std::endl;
		}
		/* end io */
	}
	std::vector<std::pair<int, double>> fitnesses;
	auto i = 0;
	std::transform(std::begin(population.mMembers), std::end(population.mMembers), std::back_inserter(fitnesses), [&locations, &i](auto &pop)
				   { return std::make_pair(i++, ComputeFitness(pop, locations)); });
	/* io */
	outputfile << "FITNESS:" << std::endl;
	for (auto &fitness : fitnesses)
	{
		outputfile << fitness.first << ':' << fitness.second << std::endl;
	}
	/* end io */
	auto min = std::min_element(std::begin(fitnesses), std::end(fitnesses), [](auto &a, auto &b)
								{ return a.second < b.second; });
	/* io */
	outputfile << "SOLUTION:" << std::endl;
	for (auto &location : population.mMembers[min->first])
	{
		outputfile << locations[location].mName << std::endl;
	}
	outputfile << locations[0].mName << std::endl;
	outputfile << "DISTANCE: " << min->second << " miles" << std::endl;
	/* end io */
}
