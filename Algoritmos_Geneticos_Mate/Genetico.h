#pragma once
#include "config.h"

const double TOL=1e-7;
const int BITS_SIZE=7;
const int AMOUNT_THREADS=4;
const float PROB_MUT=0.2;
// TO DO: Mathematic parser -> por ahora se asume x2
// Trabajando con [0,31]
class Individuo{
public:
	int real_value;
	std::bitset<BITS_SIZE> number_bits;
	double fitness;
	
	
	bool operator<(const Individuo& other) const;
	bool operator==(const Individuo& other) const;
	void initialize_fit(double number);
	void update_real_value();
};

class Genetico{
private:
	int population_size;
	std::vector<Individuo> population;
	std::random_device rd;
	std::mutex debug_mutex;
public:
	void Fill_fitness(int start,int end,std::vector<Individuo> &vec);
	void Initialize(int size_pop);
	std::vector<Individuo>::iterator Elitism();
	void Tournament_Selection(int start,int end,std::vector<Individuo>& new_pop,std::mt19937 &local_gen);
	void Crossover(int start,int end,std::vector<Individuo>& new_pop,std::mt19937 &local_gen);
	void Mutation(int start,int end,std::vector<Individuo>& new_pop,std::mt19937 &local_gen);
	void Run_Genetics(int size);
	void printBits(std::bitset<BITS_SIZE> bs);
	void printGen(int gen,Individuo& best_indv);
};