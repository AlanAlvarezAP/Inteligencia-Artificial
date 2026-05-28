#include "cuda_runtime.h"
#include "device_launch_parameters.h"
#include <stdio.h>
#include <fstream>
#include <sstream>
#include <cstdint>
#include <iostream>
#include <ctime>
#include <cstdlib>

#include <random>


using namespace std;

#define IMG_SIZE (28 * 28)
#define IMG_SIZE_PLUS_BIAS (IMG_SIZE + 1)


// Perceptron Classes

class Data {
public:
    int n_samples;
    uint8_t* images;

    Data() : n_samples(0), images(nullptr) {}

    ~Data() {
        if (images) cudaFreeHost(images);
    }
};

class Perceptron {
public:
    const float learning_rate = 0.001f;
    float* weights;
    uint8_t* waited_output;
    uint8_t* output;

    Perceptron() : waited_output(nullptr), output(nullptr) {
        cudaMallocHost(&weights, 10 * IMG_SIZE_PLUS_BIAS * sizeof(float));
        reset_weights();
    }

    void reset_weights() {
        std::random_device rd;
        std::mt19937 gen(rd());

        std::uniform_real_distribution<float> dist(-0.5f, 0.5f);

        for (int i = 0; i < 10 * IMG_SIZE_PLUS_BIAS; i++){
            weights[i] = dist(gen);
        }
        for (int n = 0; n < 10; n++){
            weights[n * IMG_SIZE_PLUS_BIAS + IMG_SIZE] = 0.0f;
        }
    }

    ~Perceptron() {
        if (weights) 
            cudaFreeHost(weights);
        if (waited_output) 
            cudaFreeHost(waited_output);
        if (output) 
            cudaFreeHost(output);
    }
};

// Aux function for MINST dataset

uint32_t readBE32(ifstream& file) {
    uint8_t value[4];
    file.read(reinterpret_cast<char*>(value), 4);
    return (value[0] << 24) | (value[1] << 16) | (value[2] << 8) | value[3];
}

__device__ uint8_t activationFunction(float sum) {
    return (sum >= 0) ? 1 : 0;
}

void print_ascii_number(const Data* data, int index) {
    const uint8_t* img = &data->images[index * IMG_SIZE_PLUS_BIAS];

    for (int y = 0; y < 28; y++) {
        for (int x = 0; x < 28; x++) {
            int p = y * 28 + x;
            char c = (img[p] < 1) ? '#' : ' ';
            cout << c;
        }
        cout << endl;
    }
    cout << endl;
}


//File Functions (weights)

void save_weights(const Perceptron* perceptron, const char* path) {
    ofstream file(path);
    for (int n = 0; n < 10; n++) {
        for (int p = 0; p < IMG_SIZE_PLUS_BIAS; p++) {
            file << perceptron->weights[n * IMG_SIZE_PLUS_BIAS + p];
            if (p < IMG_SIZE_PLUS_BIAS - 1) file << ",";
        }
        file << endl;
    }
    cout << "Guardando pesos" << endl;
}

void load_weights(Perceptron* perceptron, const char* path) {
    ifstream file(path);
    string line;
    int n = 0;
    while (getline(file, line) && n < 10) {
        stringstream ss(line);
        string val;
        int p = 0;
        while (getline(ss, val, ',') && p < IMG_SIZE_PLUS_BIAS) {
            perceptron->weights[n * IMG_SIZE_PLUS_BIAS + p] = stof(val);
            p++;
        }
        n++;
    }
    cout << "Cargando Pesos" << endl;
}

//File Function (Dataset)

void load_data(const char* images_path, const char* labels_path, Data* data, Perceptron* perceptron) {
    ifstream labels_file(labels_path, ios::binary);
    readBE32(labels_file);
    uint32_t n_samples = readBE32(labels_file);
    data->n_samples = n_samples;

    cudaMallocHost(&perceptron->waited_output, n_samples);
    labels_file.read((char*)perceptron->waited_output, n_samples);

    ifstream images_file(images_path, ios::binary);
    readBE32(images_file);
    readBE32(images_file);
    readBE32(images_file);
    readBE32(images_file);

    cudaMallocHost(&data->images, n_samples * IMG_SIZE_PLUS_BIAS);
    cudaMallocHost(&perceptron->output, n_samples * 10 * sizeof(uint8_t));

    for (int s = 0; s < (int)n_samples; s++) {
        images_file.read((char*)&data->images[s * IMG_SIZE_PLUS_BIAS], IMG_SIZE);
        data->images[s * IMG_SIZE_PLUS_BIAS + IMG_SIZE] = 0;
    }
}

__global__ void normalize(uint8_t* images, int n_samples) {
    int sample = blockIdx.x * blockDim.x + threadIdx.x;
    if (sample >= n_samples) return;

    for (int p = 0; p < IMG_SIZE; p++)
        images[sample * IMG_SIZE_PLUS_BIAS + p] = images[sample * IMG_SIZE_PLUS_BIAS + p] > 120 ? 1 : 0;

    images[sample * IMG_SIZE_PLUS_BIAS + IMG_SIZE] = 1;
}

void normalize_gpu(Data* data) {
    uint8_t* d_images;
    int total = data->n_samples * IMG_SIZE_PLUS_BIAS;
    cudaMalloc(&d_images, total * sizeof(uint8_t));
    cudaMemcpy(d_images, data->images, total, cudaMemcpyDefault);

    int threads = 256;
    int blocks = (data->n_samples + threads - 1) / threads;
    normalize << <blocks, threads >> > (d_images, data->n_samples);
    cudaDeviceSynchronize();

    cudaMemcpy(data->images, d_images, total, cudaMemcpyDefault);
    cudaFree(d_images);
}


void prepare_data(const char* images_path, const char* labels_path, const char* weights_file, bool reset, Data*& data, Perceptron*& perceptron) {
    data = new Data();
    perceptron = new Perceptron();

    load_data(images_path, labels_path, data, perceptron);
    normalize_gpu(data);

    if (reset) perceptron->reset_weights();
    else load_weights(perceptron, weights_file);
}

// Perceptron Functions

__global__ void forwardPass(uint8_t* images, float* weights, uint8_t* output, int n_samples) {
    int sample = blockIdx.x;
    int neuron = blockIdx.y;
    int pixel = threadIdx.x;
    if (sample >= n_samples || neuron >= 10) return;

    __shared__ float partial[1024];

    if (pixel < IMG_SIZE_PLUS_BIAS) {
        partial[pixel] = (float)images[sample * IMG_SIZE_PLUS_BIAS + pixel] * weights[neuron * IMG_SIZE_PLUS_BIAS + pixel];
    }
    else {
        partial[pixel] = 0.0f;
    }
    __syncthreads();
   
    for (int stride = blockDim.x / 2; stride > 0; stride /= 2) {
        if (pixel < stride) partial[pixel] += partial[pixel + stride];
        __syncthreads();
    }

    if (pixel == 0) {
        output[sample * 10 + neuron] = activationFunction(partial[0]);
    }
}

__global__ void updateWeight(uint8_t* images, float* weights, float learning_rate, uint8_t* output, uint8_t* waited_output, int n_samples) {
    int sample = blockIdx.x;
    int neuron = blockIdx.y;
    int pixel = threadIdx.x;
    if (sample >= n_samples || neuron >= 10 || pixel >= IMG_SIZE_PLUS_BIAS) return;

    int error = (neuron == waited_output[sample] ? 1 : 0) - output[sample * 10 + neuron];
    if (error != 0) {
        float px = (float)images[sample * IMG_SIZE_PLUS_BIAS + pixel];
        atomicAdd(&weights[neuron * IMG_SIZE_PLUS_BIAS + pixel], learning_rate * error * px);
    }
}


// For test Functions

uint8_t predict_single(const Data* data, const Perceptron* perceptron, int index) {
    uint8_t predicted = 0;
    float best = -1e9f;
    for (int n = 0; n < 10; n++) {
        float sum = 0.0f;
        for (int p = 0; p < IMG_SIZE_PLUS_BIAS; p++)
            sum += (float)data->images[index * IMG_SIZE_PLUS_BIAS + p] * perceptron->weights[n * IMG_SIZE_PLUS_BIAS + p];
        if (sum > best) {
            best = sum;
            predicted = (uint8_t)n;
        }
    }
    return predicted;
}


void run_test(const char* weights_file, int n_test) {
    Data* data = nullptr;
    Perceptron* perceptron = nullptr;
    prepare_data("t10k-images.idx3-ubyte", "t10k-labels.idx1-ubyte", weights_file, false, data, perceptron);

    srand((unsigned)time(nullptr));
    for (int i = 0; i < n_test; i++) {
        int index = rand() % data->n_samples;
        uint8_t predicted = predict_single(data, perceptron, index);
        uint8_t expected = perceptron->waited_output[index];

        cout << "Resultado esperado: " << (int)expected << endl;
        print_ascii_number(data, index);
        cout << "Numero deducido: " << (int)predicted << endl;
        cout << (predicted == expected ? "[CORRECTO]" : "[INCORRECTO]") << endl;
    }

    delete data;
    delete perceptron;
}

// For training Functions
void run_training(bool reset, const char* weights_file, int n_epochs) {
    Data* data = nullptr;
    Perceptron* perceptron = nullptr;
    prepare_data("train-images.idx3-ubyte", "train-labels.idx1-ubyte", weights_file, reset, data, perceptron);

    int total_size = data->n_samples * IMG_SIZE_PLUS_BIAS;
    uint8_t* d_images;
    uint8_t* d_output;
    uint8_t* d_waited_output;
    float* d_weights;

    cudaMalloc(&d_images, total_size * sizeof(uint8_t));
    cudaMalloc(&d_output, data->n_samples * 10 * sizeof(uint8_t));
    cudaMalloc(&d_waited_output, data->n_samples * sizeof(uint8_t));
    cudaMalloc(&d_weights, 10 * IMG_SIZE_PLUS_BIAS * sizeof(float));

    cudaMemcpy(d_images, data->images, total_size, cudaMemcpyDefault);
    cudaMemcpy(d_waited_output, perceptron->waited_output, data->n_samples, cudaMemcpyDefault);
    cudaMemcpy(d_weights, perceptron->weights, 10 * IMG_SIZE_PLUS_BIAS * sizeof(float), cudaMemcpyDefault);

    dim3 blocks(data->n_samples, 10);
    dim3 threads(1024);

    for (int epoch = 0; epoch < n_epochs; epoch++) {
        forwardPass <<<blocks, threads>>> (d_images, d_weights, d_output, data->n_samples);
        cudaDeviceSynchronize();

        updateWeight <<<blocks, dim3(IMG_SIZE_PLUS_BIAS)>>> (d_images, d_weights, perceptron->learning_rate, d_output, d_waited_output, data->n_samples);
        cudaDeviceSynchronize();

        cudaMemcpy(perceptron->weights, d_weights, 10 * IMG_SIZE_PLUS_BIAS * sizeof(float), cudaMemcpyDefault);
        cudaMemcpy(perceptron->output, d_output, data->n_samples * 10, cudaMemcpyDefault);

        int correct = 0;
        int total_error = 0;
        for (int s = 0; s < data->n_samples; s++) {
            uint8_t predicted = predict_single(data, perceptron, s);
            uint8_t expected = perceptron->waited_output[s];
            if (predicted == expected) correct++;
            for (int n = 0; n < 10; n++) {
                int e = (n == expected ? 1 : 0) - perceptron->output[s * 10 + n];
                total_error += abs(e);
            }
        }

        float accuracy = (float)correct / data->n_samples * 100.0f;
        float avg_error = (float)total_error / data->n_samples;

        cout << "------------------------------" << endl;
        cout << "Epoca:" << epoch + 1 << " / " << n_epochs << endl;
        cout << "Accuracy: " << accuracy << "%" << endl;
        cout << "Error:" << avg_error << endl;

        cout << "Pesos promedio por neurona:" << endl;
        for (int n = 0; n < 10; n++) {
            float sum = 0.0f;
            for (int p = 0; p < IMG_SIZE; p++) sum += perceptron->weights[n * IMG_SIZE_PLUS_BIAS + p];
            cout << "  Neurona " << n << ": " << sum / IMG_SIZE << "  (bias=" << perceptron->weights[n * IMG_SIZE_PLUS_BIAS + IMG_SIZE] << ")" << endl;
        }
        cout << "------------------------------" << endl;
    }

    save_weights(perceptron, weights_file);

    cudaFree(d_images);
    cudaFree(d_output);
    cudaFree(d_waited_output);
    cudaFree(d_weights);

    delete data;
    delete perceptron;
}

int main() {
    int n_epochs = 3;
    int n_test = 3;

    const char* WEIGHTS_FILE = "weights.csv";
    int mode = -1;

    while (mode != 0) {
        cout << "------------------------------------------" << endl;
        cout << "  Perceptron MNIST - CUDA" << endl;
        cout << "------------------------------------------" << endl;
        cout << endl;
        cout << "Selecciona un modo:" << endl;
        cout << "  0. Salir" << endl;
        cout << "  1. Entrenamiento desde 0" << endl;
        cout << "  2. Continuar entrenamiento" << endl;
        cout << "  3. Pruebas" << endl;
        cout << "  4. Cambiar numero de epocas (Actual: " << n_epochs << ")" << endl;
        cout << "  5. Cambiar numero de pruebas (Actual: " << n_test << ")" << endl;


        cout << endl;
        cout << "Modo: ";
        cin >> mode;
        cout << endl;

        if (mode == 0) {
            return 0;
        }
        else if (mode == 1) {
            run_training(true, WEIGHTS_FILE, n_epochs);
        }
        else if (mode == 2) {
            run_training(false, WEIGHTS_FILE, n_epochs);
        }
        else if (mode == 3) {
            run_test(WEIGHTS_FILE, n_test);
        }
        else if (mode == 4) {
            cout << "Nueva cantidad de epocas: ";
            cin >> n_epochs;
        }
        else if (mode == 5) {
            cout << "Nueva cantidad de pruebas: ";
            cin >> n_test;
        }
    }

    return 0;
}
