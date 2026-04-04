#include "Genetico.h"
#include "config.h"


bool Individuo::operator<(const Individuo& first) const{
	return this->real_value < first.real_value;
}

void Individuo::initialize_fit(int number){
	this->number_bits = this->real_value = number;
	fitness=real_value*real_value;
}

double sumar_fitness(double acc, const Individuo& ind){
    return acc + ind.fitness;
}


Genetico::Genetico():gen(rd()){}

void Genetico::Fill_fitness(int start,int end,int size_pop,std::vector<Individuo> &vec){
	for(int i=start;i<end;i++){
		vec[i].initialize_fit(vec[i].number_bits.to_ulong());
	}
}

void Genetico::Initialize(int size_pop){
	std::uniform_int_distribution<>dist(0,31);
	
	this->population_size=size_pop;
	
	while(size_pop--){
		Individuo tmp;
		tmp.initialize_fit(dist(gen));
		this->population.push_back(tmp);
	}
}

Individuo& Genetico::Elitism(){
	return *std::max_element(population.begin(),population.end());
}


void Genetico::Tournament_Selection(std::vector<Individuo>& new_pop){
	while(static_cast<int>(new_pop.size()) < this->population_size-1){
		std::uniform_int_distribution<>dist(0,this->population.size()-1);
		
		int idx_1=dist(gen);
		int idx_2=-1;
		do{
			idx_2=dist(gen);
		}while(idx_2 == idx_1);
		
		Individuo best=this->population[idx_1].fitness > this->population[idx_2].fitness ? this->population[idx_1]: this->population[idx_2];
		new_pop.push_back(best);
	
	}
	
}

void Genetico::Crossover(Individuo &first,Individuo &second){
	std::uniform_int_distribution<>dist(0,BITS_SIZE-1);
	int division_point=dist(gen);
	
	std::bitset<BITS_SIZE> temp = first.number_bits;
	for(int i=BITS_SIZE-1-division_point;i>=0;i--){
		first.number_bits[i]=second.number_bits[i];
		second.number_bits[i]=temp[i];
	}
	
}

void Genetico::printBits(std::bitset<BITS_SIZE> bs) {
    for (int i = BITS_SIZE - 1; i >= 0; i--)
        std::cout << bs[i];
    std::cout << std::endl;
}

void Genetico::Mutation(Individuo& first){
	std::uniform_int_distribution<>dist(0,BITS_SIZE-1);
	int random_pos=dist(gen);
	first.number_bits^= (1 << random_pos);
}

void Genetico::printGen(int gen,Individuo& best_indv){
	double total=std::accumulate(this->population.begin(),this->population.end(),0.0,sumar_fitness);
	total/=(int)this->population.size();
	int i=1;
	std::cout << " --------------- GENERATION " << gen << " -------------------" << std::endl;
	for(const auto &p:this->population){
		std::cout << "Individual: [" << i++ << "] with value -> " << p.real_value << " and bits "; 
		printBits(p.number_bits); 
		std::cout << " and fitness " << p.fitness << std::endl;
	}
	std::cout << " Best individual " << best_indv.real_value << std::endl;
	std::cout << " -------------------------------------------------------------" << std::endl;
}

void Genetico::Run_Genetics(int size){
	
	Initialize(size);
	int geni=0;
	Individuo old_indv={std::numeric_limits<int>::max(),std::numeric_limits<int>::max(),std::numeric_limits<float>::max()};
	Individuo best_indv=old_indv;
	std::vector<std::thread> threads;
	
	const int CHUNK_SIZE=size/4;
	
	do{
		old_indv=best_indv;
		for(int i=0;i<4;i++){
			int start=i*CHUNK_SIZE;
			int end=i*CHUNK_SIZE+4;
			threads.emplace_back(&Genetico::Fill_fitness,this,start,end,size,std::ref(this->population));
		}
		
		for(auto &t:threads){
			t.join();
		}
		
		threads.clear();
		
		std::vector<Individuo> new_population;
		best_indv=Elitism();
	
		printGen(geni,best_indv);
	
		Tournament_Selection(new_population);
		
		for(int i=0;i<new_population.size();i+=2){
			Individuo& num_1=new_population[i];
			Individuo& num_2=new_population[i+1];
			Crossover(num_1,num_2);
		}
		
		std::uniform_int_distribution<>dist(0.0,1.0);
		for(int i=0;i<new_population.size();i++){
			if(dist(gen)<PROB_MUT){
				Individuo& num_1=new_population[i];
				Mutation(num_1);
			}
		}
		new_population.push_back(best_indv);
		this->population=std::move(new_population);
		
		geni++;
	}while(std::abs(best_indv.fitness-old_indv.fitness) > 1e-7);
	
	
}