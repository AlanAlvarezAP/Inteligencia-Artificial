#include "Genetico.h"
#include "config.h"

template<int BITS>
bool Individuo<BITS>::operator<(const Individuo& other) const {
    return this->fitness < other.fitness;
}

template<int BITS>
bool Individuo<BITS>::operator==(const Individuo& other) const {
    return this->real_value == other.real_value;
}

template<int BITS>
void Individuo<BITS>::initialize_fit(double number) {
    this->number_bits = (int)number;
}

// Funcion es -> x*x - 2*x*y + y*y pero es minimizando 
template<int BITS>
void Individuo<BITS>::update_real_value(double x, double y) {
    this->real_value = number_bits.to_ulong();
    //this->fitness = x*x - 2*x*y + y*y;
    this->fitness = (x - 5)*(x - 3)+ (y - 1)*(y - 5) + 10 * (sin(x) + cos(y));

}

template<int BITS>
void Individuo<BITS>::printBits(std::bitset<BITS> bs) {
    for (int i = BITS - 1; i >= 0; i--)
        std::cout << bs[i];
    std::cout << std::endl;
}

template class Individuo<5>;
template class Individuo<6>;

double sumar_fitness(double acc, const par_indv& ind) {
    return acc + ind.first.fitness + ind.second.fitness;
}

void Genetico::Fill_fitness(int start, int end, std::vector<par_indv>& vec) {
    for (int i = start; i < end; i++) {
        Individuo<5>& first_elem  = vec[i].first;
        Individuo<6>& second_elem = vec[i].second;
        double x = first_elem.number_bits.to_ulong();
        double y = second_elem.number_bits.to_ulong();
        first_elem.update_real_value(x, y);
        second_elem.update_real_value(x, y);
    }
}

void Genetico::Initialize(int size_pop) {
    std::uniform_int_distribution<> dist_x(0, 31);
    std::uniform_int_distribution<> dist_y(0, 63);
    std::mt19937 local_gen(rd());
    this->population_size = size_pop;

    while (size_pop--) {
        Individuo<5> tmp;
        Individuo<6> tmp_2;
        int x_value = dist_x(local_gen), y_value = dist_y(local_gen);
        tmp.initialize_fit(x_value);
        tmp_2.initialize_fit(y_value);
        tmp.update_real_value(x_value, y_value);
        tmp_2.update_real_value(x_value, y_value);
        this->population.push_back({tmp, tmp_2});
    }
}

std::vector<par_indv>::iterator Genetico::Elitism() {
    return std::min_element(population.begin(), population.end(),[](const par_indv& a, const par_indv& b) {
            return (a.first.fitness + a.second.fitness) < (b.first.fitness + b.second.fitness);});
}

void Genetico::Tournament_Selection(int start, int end, std::vector<par_indv>& new_pop, std::mt19937& local_gen) {
    for (int i = start; i < end; i++) {
        std::uniform_int_distribution<> dist(0, this->population.size()-1);

        int idx_1 = dist(local_gen);
        int idx_2 = -1;
        do {
            idx_2 = dist(local_gen);
        } while (idx_2 == idx_1);

        par_indv& ind1 = this->population[idx_1];
        par_indv& ind2 = this->population[idx_2];

        par_indv best = (ind1.first.fitness + ind1.second.fitness) < (ind2.first.fitness + ind2.second.fitness) ? ind1 : ind2;
        new_pop[i] = best;
    }
}

void Genetico::Crossover(int start, int end, std::vector<par_indv>& new_population, std::mt19937& local_gen) {
    for (int i = start; i < end; i += 2) {
        par_indv& num_1 = new_population[i];
        par_indv& num_2 = new_population[i+1];

        std::uniform_int_distribution<> dist_x(0, 4);
        std::uniform_int_distribution<> dist_y(0, 5);
        int div_x = dist_x(local_gen);
        int div_y = dist_y(local_gen);

        std::bitset<5> temp_x = num_1.first.number_bits;
        std::bitset<6> temp_y = num_1.second.number_bits;

        for (int j = 0; j < div_x; j++) {
            num_1.first.number_bits[j]  = num_2.first.number_bits[j];
            num_2.first.number_bits[j]  = temp_x[j];
        }
        for (int j = 0; j < div_y; j++) {
            num_1.second.number_bits[j] = num_2.second.number_bits[j];
            num_2.second.number_bits[j] = temp_y[j];
        }

        double x1 = num_1.first.number_bits.to_ulong();
        double y1 = num_1.second.number_bits.to_ulong();
        num_1.first.update_real_value(x1, y1);
        num_1.second.update_real_value(x1, y1);

        double x2 = num_2.first.number_bits.to_ulong();
        double y2 = num_2.second.number_bits.to_ulong();
        num_2.first.update_real_value(x2, y2);
        num_2.second.update_real_value(x2, y2);
    }
}



void Genetico::Mutation(int start, int end, std::vector<par_indv>& new_population, std::mt19937& local_gen) {
    std::uniform_real_distribution<> dist(0.0, 1.0);
    for (int i = start; i < end; i++) {
        for (int b = 0; b < 5; b++) {
            if (dist(local_gen) < PROB_MUT)
                new_population[i].first.number_bits.flip(b);
        }
        for (int b = 0; b < 6; b++) {
            if (dist(local_gen) < PROB_MUT)
                new_population[i].second.number_bits.flip(b);
        }
        auto& ind = new_population[i];
        double x = ind.first.number_bits.to_ulong();
        double y = ind.second.number_bits.to_ulong();
        ind.first.update_real_value(x, y);
        ind.second.update_real_value(x, y);
    }
}

void Genetico::printGen(int gen, par_indv& best_indv) {
    double total = std::accumulate(this->population.begin(), this->population.end(), 0.0, sumar_fitness);
    total /= (int)this->population.size();
	
	best_and_avg.push_back({
		best_indv.first.fitness + best_indv.second.fitness,
		total
	});
	
    int i = 1;
    std::cout << " --------------- GENERATION " << gen << " -------------------" << std::endl;
	Individuo<5> tmp;
	Individuo<6> tmp_2;
    for (const auto& p : this->population) {
        std::cout << "Individual: [" << i++ << "] with value in x -> " << p.first.real_value
                  << " and value in y -> " << p.second.real_value << " and bits for x -> ";
        tmp.printBits(p.first.number_bits);
        std::cout << " and bits for y -> ";
        tmp_2.printBits(p.second.number_bits);
        std::cout << " and fitness for x -> " << p.first.fitness
                  << " and fitness for y -> " << p.second.fitness << std::endl;
    }
    std::cout << " Best individual in x -> " << best_indv.first.real_value
              << " and in y -> " << best_indv.second.real_value << std::endl;
    std::cout << " The mean -> " << total << std::endl;
    std::cout << " -------------------------------------------------------------" << std::endl;
}

void Genetico::Run_Genetics(int size) {
    if (size < AMOUNT_THREADS) { 
		std::cout << " POBLACION MUY PEQUENA" << std::endl; 
		return; 
	}

    std::vector<std::mt19937> generators;
/*     for (int i = 0; i < AMOUNT_THREADS; i++)
        generators.emplace_back(rd()); */
	
	for (int i = 0; i < size; i++)
        generators.emplace_back(rd());

    Initialize(size);
    int geni = 1, countdown = 0;
    par_indv old_indv{};
    old_indv.first.fitness  = -std::numeric_limits<double>::infinity();
    old_indv.second.fitness = -std::numeric_limits<double>::infinity();
    par_indv best_indv = old_indv;

    //int CHUNK_SIZE = size / AMOUNT_THREADS;
    do {
        old_indv = best_indv;
        auto best = Elitism();
        best_indv = *best;
        printGen(geni, best_indv);
		
		
        this->population.erase(best);

        int new_size = population.size();
        std::vector<par_indv> new_population(new_size);
        std::vector<std::thread> threads;
        // CHUNK_SIZE = new_size / AMOUNT_THREADS;

        /* for (int i = 0; i < AMOUNT_THREADS; i++) {
            int start = i * CHUNK_SIZE;
            int end = (i == AMOUNT_THREADS-1) ? new_size : (i+1)*CHUNK_SIZE;
            threads.emplace_back(&Genetico::Tournament_Selection, this, start, end, std::ref(new_population), std::ref(generators[i]));
        } */
		
		for (int i = 0; i < new_size; i++){
			threads.emplace_back(&Genetico::Tournament_Selection, this, i, i+1, std::ref(new_population),std::ref(generators[i]));
		}
		
        for (auto& t : threads){
			t.join();
		}
        threads.clear();

        /* for (int i = 0; i < AMOUNT_THREADS; i++) {
            int start = i * CHUNK_SIZE;
            int end = (i == AMOUNT_THREADS-1) ? new_size : (i+1)*CHUNK_SIZE;
            if ((end - start) % 2 != 0){
				end--;
			}
            if (end > start){
                threads.emplace_back(&Genetico::Crossover, this, start, end, std::ref(new_population), std::ref(generators[i]));
			}
        } */
		
		for(int i = 0; i < new_size - 1; i += 2){
			threads.emplace_back(&Genetico::Crossover, this, i, i+2, std::ref(new_population),std::ref(generators[i]));
		}
		
        for (auto& t : threads){
			t.join();
		}
        threads.clear();

        /* for (int i = 0; i < AMOUNT_THREADS; i++) {
            int start = i * CHUNK_SIZE;
            int end = (i == AMOUNT_THREADS-1) ? new_size : (i+1)*CHUNK_SIZE;
            threads.emplace_back(&Genetico::Mutation, this, start, end, std::ref(new_population), std::ref(generators[i]));
        } */
		
		for (int i = 0; i < new_size; i++) {
			threads.emplace_back(&Genetico::Mutation, this, i, i+1, std::ref(new_population), std::ref(generators[i]));
		}
		
        for (auto& t : threads){
			t.join();
		}
        threads.clear();

        new_population.push_back(best_indv);
        this->population = std::move(new_population);
        // CHUNK_SIZE = size / AMOUNT_THREADS;

        /* for (int i = 0; i < AMOUNT_THREADS; i++) {
            int start = i * CHUNK_SIZE;
            int end = (i == AMOUNT_THREADS-1) ? size : (i+1)*CHUNK_SIZE;
            threads.emplace_back(&Genetico::Fill_fitness, this, start, end, std::ref(this->population));
        } */
		
		for (int i = 0; i < size; i++) {
			threads.emplace_back(&Genetico::Fill_fitness, this, i, i+1, std::ref(this->population));
		}
		
        for (auto& t : threads){
			t.join();
		}
        threads.clear();

        double delta = std::abs(best_indv.first.fitness - old_indv.first.fitness);
        if (delta < TOL){
			countdown++;
		}
        else{
			countdown = 0;
		}

        geni++;
    } while (countdown < 10 && geni < 1000);
}
