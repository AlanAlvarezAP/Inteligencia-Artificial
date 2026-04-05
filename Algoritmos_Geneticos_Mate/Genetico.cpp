#include "Genetico.h"
#include "config.h"


bool Individuo::operator<(const Individuo& first) const{
	return this->fitness < first.fitness;
}

bool Individuo::operator==(const Individuo& other) const{
	return this->real_value == other.real_value;
}

void Individuo::initialize_fit(double number) {
    this->number_bits = (int)number;
    update_real_value();
}

void Individuo::update_real_value() {
    this->real_value = number_bits.to_ulong();
    this->fitness =-(real_value*real_value - 10 * std::cos(2 * M_PI * real_value / 20.0));
}

double sumar_fitness(double acc, const Individuo& ind){
    return acc + ind.fitness;
}


void Genetico::Fill_fitness(int start,int end,std::vector<Individuo> &vec){
	for(int i=start;i<end;i++){
		vec[i].update_real_value();
	}
}

void Genetico::Initialize(int size_pop){
	std::uniform_int_distribution<>dist(0,127);
	std::mt19937 local_gen(rd());
	this->population_size=size_pop;
	
	while(size_pop--){
		Individuo tmp;
		tmp.initialize_fit(dist(local_gen));
		this->population.push_back(tmp);
	}
}

std::vector<Individuo>::iterator Genetico::Elitism(){
	return std::max_element(population.begin(),population.end());
}


void Genetico::Tournament_Selection(int start,int end,std::vector<Individuo>& new_pop,std::mt19937 &local_gen){
	for(int i=start;i<end;i++){
		std::uniform_int_distribution<>dist(0,this->population.size()-1);
		
		int idx_1=dist(local_gen);
		int idx_2=-1;
		do{
			idx_2=dist(local_gen);
		}while(idx_2 == idx_1);
		
		Individuo& ind1 = this->population[idx_1];
        Individuo& ind2 = this->population[idx_2];

        Individuo best = ind1.fitness > ind2.fitness ? ind1 : ind2;
        new_pop[i] = best;
		
		
	}
	
}

void Genetico::Crossover(int start,int end,std::vector<Individuo>& new_population,std::mt19937 &local_gen){
	
	for(int i=start;i<end;i+=2){
		Individuo& num_1=new_population[i];
		Individuo& num_2=new_population[i+1];
		
		std::uniform_int_distribution<>dist(0,BITS_SIZE-1);
		int division_point=dist(local_gen);
		
		std::bitset<BITS_SIZE> original_1 = num_1.number_bits;
        std::bitset<BITS_SIZE> original_2 = num_2.number_bits;
		
		std::bitset<BITS_SIZE> temp = num_1.number_bits;
		for(int j=0;j<division_point;j++){
			num_1.number_bits[j]=num_2.number_bits[j];
			num_2.number_bits[j]=temp[j];
		}
		
		num_1.update_real_value();
        num_2.update_real_value();

		
	}
	
}

void Genetico::printBits(std::bitset<BITS_SIZE> bs) {
    for (int i = BITS_SIZE - 1; i >= 0; i--)
        std::cout << bs[i];
    std::cout << std::endl;
}

void Genetico::Mutation(int start,int end, std::vector<Individuo> &new_population,std::mt19937 &local_gen){
	std::uniform_int_distribution<>dist_2(0,BITS_SIZE-1);
	std::uniform_real_distribution<>dist(0.0,1.0);
	for(int i=start;i<end;i++){
		for (int b = 0; b < BITS_SIZE; b++) {
            if (dist_2(local_gen) < PROB_MUT) {
                new_population[i].number_bits.flip(b);
            }
        }
        new_population[i].update_real_value();
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
	
	std::mt19937 gen(rd());
	
	Initialize(size);
	int geni=1;
	Individuo old_indv{};
	old_indv.fitness = -std::numeric_limits<double>::infinity();
	Individuo best_indv=old_indv;
	
	
	int CHUNK_SIZE=size/AMOUNT_THREADS;
	do{
		old_indv=best_indv;
		
		auto best=Elitism();
		best_indv=*best;
		
		printGen(geni,best_indv);
		this->population.erase(best);
	
		int new_size = population.size();
		std::vector<Individuo> new_population(new_size);
		std::vector<std::thread> threads;
		CHUNK_SIZE = new_size / AMOUNT_THREADS;
	
		
		for(int i=0;i<AMOUNT_THREADS;i++){
			int start=i*CHUNK_SIZE;
			int end=(i==AMOUNT_THREADS-1)? new_size : (i+1)*CHUNK_SIZE;
			threads.emplace_back(&Genetico::Tournament_Selection,this,start,end,std::ref(new_population),std::ref(gen));
		}
	
		for(auto &t : threads){
			t.join();
		}
		threads.clear();
		
		/*int i=0;
		std::cout << " --------------- POST TOURNAMENT -------------------" << std::endl;
		for(const auto &p:new_population){
			std::cout << "Individual: [" << i++ << "] with value -> " << p.real_value << " and bits "; 
			printBits(p.number_bits); 
			std::cout << " and fitness " << p.fitness << std::endl;
		}*/
		
		
		for(int i=0;i<AMOUNT_THREADS;i++){
			int start=i*CHUNK_SIZE;
			int end=(i==AMOUNT_THREADS-1)? new_size : (i+1)*CHUNK_SIZE;
			if((end - start) % 2 != 0){
				end -= 1;
			}
			if(end > start){
				threads.emplace_back(&Genetico::Crossover,this,start,end,std::ref(new_population),std::ref(gen));
			}
			
		}
		
		for(auto &t : threads){
			t.join();
		}
		threads.clear();
		
		/*i=0;
		std::cout << " --------------- POST CROSSOVER -------------------" << std::endl;
		for(const auto &p:new_population){
			std::cout << "Individual: [" << i++ << "] with value -> " << p.number_bits.to_ulong() << " and bits "; 
			printBits(p.number_bits); 
			std::cout << " and fitness " << p.fitness << std::endl;
		}*/
		
		for(int i=0;i<AMOUNT_THREADS;i++){
			int start=i*CHUNK_SIZE;
			int end=(i==AMOUNT_THREADS-1)? new_size : (i+1)*CHUNK_SIZE;
			threads.emplace_back(&Genetico::Mutation,this,start,end,std::ref(new_population),std::ref(gen));
		}
	
		for(auto &t : threads){
			t.join();
		}
		threads.clear();
		
		/*i=0;
		std::cout << " --------------- POST MUTATION -------------------" << std::endl;
		for(const auto &p:new_population){
			std::cout << "Individual: [" << i++ << "] with value -> " << p.number_bits.to_ulong() << " and bits "; 
			printBits(p.number_bits); 
			std::cout << " and fitness " << p.fitness << std::endl;
		}*/
		
		new_population.push_back(best_indv);
		this->population=std::move(new_population);
		CHUNK_SIZE=size/AMOUNT_THREADS;
		for(int i=0;i<AMOUNT_THREADS;i++){
			int start=i*CHUNK_SIZE;
			int end=(i==AMOUNT_THREADS-1)? size : (i+1)*CHUNK_SIZE;
			threads.emplace_back(&Genetico::Fill_fitness,this,start,end,std::ref(this->population));
		}
		
		for(auto &t:threads){
			t.join();
		}
		
		threads.clear();
		
		
		geni++;
	}while(std::abs(best_indv.fitness-old_indv.fitness) > TOL);
	
	
}