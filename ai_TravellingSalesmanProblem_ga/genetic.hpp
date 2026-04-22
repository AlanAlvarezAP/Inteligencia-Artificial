#ifndef GENETIC_HPP
#define GENETIC_HPP

#include <iostream>
#include <random>
#include <algorithm>
#include <thread>
#include <mutex>
#include <atomic>

#include "graph.hpp"

class GeneticAlgorithm
{
private:
    Graph& graph;
    int populationSize;
    double mutationRate;
	char selectElitism;
 
    using Chromosome = vector<int>;
    vector<Chromosome> population;
    vector<mt19937> engines;

public:
    mutex          bestMutex;
    Chromosome     bestChromosome;
    float          bestFitness = 1e9f;
    atomic<bool>   running{true};
 
    // for graphic
    vector<float> historyBest;
    vector<float> historyAvg;

    GeneticAlgorithm(Graph& g, int popSize, double mutRate, char selElitism);
 
    float fitness(const Chromosome& c);
    void tournamentTask(int start, int end, vector<Chromosome>& nextPop,
		int engineIdx);
    void crossoverRangeTask(int start, int end, vector<Chromosome>& nextPop,
		int engineIdx);
    void mutationTask(int start, int end, vector<Chromosome>& nextPop,
		int engineIdx);
    void run(int patience);
};

#endif