#include "Genetico.h"
#include "config.h"


bool Individuo::operator<(const Individuo& first) const{
	return this->real_value < first.real_value;
}

bool Individuo::operator==(const Individuo& other) const{
	return this->real_value == other.real_value;
}

void Individuo::initialize_fit(int number){
	this->number_bits = this->real_value = number;
	fitness=real_value*real_value;
}

double sumar_fitness(double acc, const Individuo& ind){
    return acc + ind.fitness;
}


Genetico::Genetico():gen(rd()){}

void Genetico::Fill_fitness(int start,int end,std::vector<Individuo> &vec){
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

std::vector<Individuo>::iterator Genetico::Elitism(){
	return std::max_element(population.begin(),population.end());
}


void Genetico::Tournament_Selection(int start,int end,std::vector<Individuo>& new_pop){
	for(int i=start;i<end;i++){
		std::uniform_int_distribution<>dist(0,this->population.size()-1);
		
		int idx_1=dist(gen);
		int idx_2=-1;
		do{
			idx_2=dist(gen);
		}while(idx_2 == idx_1);
		
		Individuo best=this->population[idx_1].fitness > this->population[idx_2].fitness ? this->population[idx_1]: this->population[idx_2];
		new_pop[i]=best;
	}
	
}

void Genetico::Crossover(int start,int end,std::vector<Individuo>& new_population){
	
	for(int i=start;i<end;i+=2){
		Individuo& num_1=new_population[i];
		Individuo& num_2=new_population[i+1];
		
		std::uniform_int_distribution<>dist(0,BITS_SIZE-1);
		int division_point=dist(gen);
		
		std::bitset<BITS_SIZE> temp = num_1.number_bits;
		for(int i=BITS_SIZE-1-division_point;i>=0;i--){
			num_1.number_bits[i]=num_2.number_bits[i];
			num_2.number_bits[i]=temp[i];
		}
		
	}
	
}

void Genetico::printBits(std::bitset<BITS_SIZE> bs) {
    for (int i = BITS_SIZE - 1; i >= 0; i--)
        std::cout << bs[i];
    std::cout << std::endl;
}

void Genetico::Mutation(int start,int end, std::vector<Individuo> &new_population){
	
	for(int i=start;i<end;i++){
		std::uniform_int_distribution<>dist(0.0,1.0);
		if(dist(gen)<PROB_MUT){
			Individuo& num_1=new_population[i];
			std::uniform_int_distribution<>dist_2(0,BITS_SIZE-1);
			int random_pos=dist_2(gen);
			num_1.number_bits^= (1 << random_pos);
		}
	}
	
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
	std::cout << " Best individual -> " << best_indv.real_value << std::endl;
	std::cout << " The mean -> " << total << std::endl;
	std::cout << " -------------------------------------------------------------" << std::endl;
}

void Genetico::Run_Genetics(int size){
	if(size < AMOUNT_THREADS){
		std::cout << " POBLACION MUY PEQUEÑA " << std::endl;
		return;
	}
	Initialize(size);
	int geni=0;
	Individuo old_indv={std::numeric_limits<int>::max(),std::numeric_limits<int>::max(),std::numeric_limits<float>::max()};
	Individuo best_indv=old_indv;
	std::vector<std::thread> threads;
	
	int CHUNK_SIZE=size/AMOUNT_THREADS;
	do{
		old_indv=best_indv;
		for(int i=0;i<AMOUNT_THREADS;i++){
			int start=i*CHUNK_SIZE;
			int end=(i==AMOUNT_THREADS-1)? size : (i+1)*CHUNK_SIZE;
			threads.emplace_back(&Genetico::Fill_fitness,this,start,end,std::ref(this->population));
		}
		
		for(auto &t:threads){
			t.join();
		}
		
		threads.clear();
		
		
		auto best=Elitism();
		best_indv=*best;
		printGen(geni,best_indv);
		this->population.erase(best);
	
		int new_size = population.size();
		std::vector<Individuo> new_population(size-1);
		CHUNK_SIZE = new_size / AMOUNT_THREADS;
	
		for(int i=0;i<AMOUNT_THREADS;i++){
			int start=i*CHUNK_SIZE;
			int end=(i==AMOUNT_THREADS-1)? new_size : (i+1)*CHUNK_SIZE;
			threads.emplace_back(&Genetico::Tournament_Selection,this,start,end,std::ref(new_population));
		}
	
		for(auto &t : threads){
			t.join();
		}
		threads.clear();
		
		
		for(int i=0;i<AMOUNT_THREADS;i++){
			int start=i*CHUNK_SIZE;
			int end=(i==AMOUNT_THREADS-1)? new_size : (i+1)*CHUNK_SIZE;
			if((end - start) % 2 != 0){
				end -= 1;
			}
			if(end > start){
				threads.emplace_back(&Genetico::Crossover,this,start,end,std::ref(new_population));
			}
			
		}
		
		for(auto &t : threads){
			t.join();
		}
		threads.clear();
		
		for(int i=0;i<AMOUNT_THREADS;i++){
			int start=i*CHUNK_SIZE;
			int end=(i==AMOUNT_THREADS-1)? new_size : (i+1)*CHUNK_SIZE;
			threads.emplace_back(&Genetico::Mutation,this,start,end,std::ref(new_population));
		}
	
		for(auto &t : threads){
			t.join();
		}
		threads.clear();
		
		
		new_population.push_back(best_indv);
		this->population=std::move(new_population);
		
		geni++;
	}while(std::abs(best_indv.fitness-old_indv.fitness) > 1e-7);
	
	
}