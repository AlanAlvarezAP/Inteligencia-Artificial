#pragma once
#include "config.h"

const double TOL=1e-7;

template<int BITS>
class Individuo{
public:
	int real_value;
	std::bitset<BITS> number_bits;
	double fitness;
	
	bool operator<(const Individuo& other) const;
	bool operator==(const Individuo& other) const;
	void initialize_fit(double number);
	void update_real_value(double x,double y);
	void printBits(std::bitset<BITS> bs);
};

using par_indv=std::pair<Individuo<5>,Individuo<6>>;

class Genetico{
private:
	int population_size;
	std::vector<par_indv> population;
	std::random_device rd;
	
	float PROB_MUT = 0.02;
	int limit = 10;
	int size = 10;
	
public:
	std::vector<std::pair<double,double>> best_and_avg;
	void Fill_fitness(int start,int end,std::vector<par_indv> &vec);
	void Initialize();
	std::vector<par_indv>::iterator Elitism();
	void Tournament_Selection(int start,int end,std::vector<par_indv>& new_pop,std::mt19937 &local_gen);
	void Crossover(int start,int end,std::vector<par_indv>& new_pop,std::mt19937 &local_gen);
	void Mutation(int start,int end,std::vector<par_indv>& new_pop,std::mt19937 &local_gen);
	void Run_Genetics();
	void printGen(int gen,par_indv& best_indv);
	
	void setProbMut(float new_prob);
	void setLimitIt(int new_limit);
	void setSize(int new_size);
};
