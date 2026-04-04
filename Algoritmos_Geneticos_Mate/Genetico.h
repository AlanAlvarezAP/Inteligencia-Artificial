#pragma once
#include "config.h"

const float TOL=1e-7;
const int BITS_SIZE=5;
const float PROB_MUT=0.1;

// TO DO: Mathematic parser -> por ahora se asume x2
// Trabajando con [0,31]
class Individuo{
public:
	int real_value;
	std::bitset<BITS_SIZE> number_bits;
	float fitness;
	
	
	bool operator<(const Individuo& other) const;
	void initialize_fit(int number);
	double sumar_fitness(double acc, const Individuo& ind);
};

class Genetico{
private:
	int population_size;
	std::vector<Individuo> population;
	std::random_device rd;
	std::mt19937 gen; 
public:
	Genetico();
	void Fill_fitness(int start,int end,int size_pop,std::vector<Individuo> &vec);
	void Initialize(int size_pop);
	Individuo& Elitism();
	void Tournament_Selection(std::vector<Individuo>& new_pop);
	void Crossover(Individuo &first,Individuo &second);
	void Mutation(Individuo &first);
	void Run_Genetics(int size);
	void printBits(std::bitset<BITS_SIZE> bs);
	void printGen(int gen,Individuo& best_indv);
};