#include "genetic.hpp"

GeneticAlgorithm::GeneticAlgorithm(Graph& g, int popSize,
	double mutRate, char selElitism)
	: graph(g), populationSize(popSize), mutationRate(mutRate),
	selectElitism(selElitism)
{
	random_device rd;
	for (int i = 0; i < populationSize; ++i)
		engines.emplace_back(rd());
}

float GeneticAlgorithm::fitness(const Chromosome& c)
{
	float total = 0.0f;
	for (int i = 0; i < (int)c.size() - 1; i++)
		total += graph.getDistance(c[i], c[i+1]);
	total += graph.getDistance(c.back(), c[0]);
	return total;
}

void GeneticAlgorithm::tournamentTask(int start, int end,
	vector<Chromosome>& nextPop, int engineIdx)
{
	uniform_int_distribution<int> dist(0, populationSize - 1);
	for (int k = start; k < end; k++)
	{
		int i = dist(engines[engineIdx]);
		int j = dist(engines[engineIdx]);
		nextPop[k] = (fitness(population[i]) < fitness(population[j]))
			? population[i] : population[j];
	}
}

void GeneticAlgorithm::crossoverRangeTask(int start, int end,
	vector<Chromosome>& nextPop, int engineIdx)
{
	int n = graph.size();
	uniform_int_distribution<int> distPos(0, n - 1);

	for (int k = start; k < end - 1; k += 2)
	{
		int s = distPos(engines[engineIdx]);
		int e = distPos(engines[engineIdx]);
		if (s > e)
			swap(s, e);

		Chromosome p1 = nextPop[k], p2 = nextPop[k+1];
		Chromosome c1(n, -1), c2(n, -1);

		for (int i = s; i <= e; i++)
		{
			c1[i] = p1[i];
			c2[i] = p2[i];
		}

		auto fill = [&](Chromosome& child, const Chromosome& parent)
		{
			int curr = (e + 1) % n;
			for (int i = 0; i < n; i++)
			{
				int gene = parent[(e + 1 + i) % n];
				bool exists = false;
				for (int g : child)
					if (g == gene)
					{
						exists = true;
						break;
					}
				if (!exists)
				{
					child[curr] = gene;
					curr = (curr + 1) % n;
				}
			}
		};
		fill(c1, p2);
		fill(c2, p1);
		nextPop[k] = c1;
		nextPop[k+1] = c2;
	}
}

void GeneticAlgorithm::mutationTask(int start, int end,
	vector<Chromosome>& nextPop, int engineIdx)
{
	uniform_real_distribution<double> prob(0.0, 1.0);
	uniform_int_distribution<int> distGene(0, graph.size() - 1);
	for (int k = start; k < end; k++)
	{
		if (prob(engines[engineIdx]) < mutationRate)
			swap(nextPop[k][distGene(engines[engineIdx])],
				nextPop[k][distGene(engines[engineIdx])]);
	}
}

void GeneticAlgorithm::run(int patience)
{
	int n = graph.size();

	for (int i = 0; i < populationSize; i++) {
		Chromosome c(n);
		iota(c.begin(), c.end(), 0);
		shuffle(c.begin(), c.end(), engines[0]);
		population.push_back(c);
	}

	int gen = 0, stayCount = 0;
	int numThreads = min((int)thread::hardware_concurrency(), 8);
	int chunkSize = max(1, populationSize / numThreads);

	while (stayCount < patience && running)
	{
		vector<Chromosome> nextPop(populationSize);
		vector<thread> threads;

		// tourtnament
		for (int i = 0; i < numThreads; i++)
		{
			int s = i * chunkSize;
			int e = (i == numThreads-1) ? populationSize : s + chunkSize;
			if (s < e)
				threads.emplace_back(&GeneticAlgorithm::tournamentTask, this,
					s, e, ref(nextPop), i);
		}
		for (auto& t : threads)
			t.join();
		threads.clear();

		// crossover
		for (int i = 0; i < numThreads; i++)
		{
			int s = i * chunkSize; if (s % 2 != 0) s--;
			int e = (i == numThreads-1) ? populationSize : s + chunkSize;
			if (s < e)
				threads.emplace_back(&GeneticAlgorithm::crossoverRangeTask, this,
					s, e, ref(nextPop), i);
		}
		for (auto& t : threads)
			t.join();
		threads.clear();

		// mutation
		for (int i = 0; i < numThreads; i++)
		{
			int s = i * chunkSize;
			int e = (i == numThreads-1) ? populationSize : s + chunkSize;
			if (s < e)
				threads.emplace_back(&GeneticAlgorithm::mutationTask, this,
					s, e, ref(nextPop), i);
		}
		for (auto& t : threads)
			t.join();
		threads.clear();

		// selection
		Chromosome bestInGen = population[0];
		float minFit = fitness(population[0]);
		float sumFit = 0.0f;
		for (auto& c : population)
		{
			float f = fitness(c);
			sumFit += f;
			if (f < minFit)
			{
				minFit = f;
				bestInGen = c;
			}
		}
		float avgFit = sumFit / populationSize;

		historyAvg.push_back(avgFit);
		historyBest.push_back(minFit);
		lock_guard<mutex> lock(bestMutex);

		if (minFit < bestFitness)
		{
			bestFitness = minFit;
			bestChromosome = bestInGen;
			stayCount   = 0;
		}
		else
			stayCount++;

		population = move(nextPop);
		// elitism
		if (selectElitism == 'Y')
			population[0] = bestInGen;

		cout << "Gen: " << gen << " - Mejor dist: " << bestFitness 
			<< " Estanc: " << stayCount << endl;
		gen++;
	}
	cout << "\nAG terminado\n";
	cout << endl;
}