#ifndef TRANSFORMER_H
#define TRANSFORMER_HA

#include "cuda_runtime.h"
#include "device_launch_parameters.h"

#include "CLS_token.cuh"
#include "PositionalEncoding.cuh"
#include "PatchEmbedding.cuh"
#include "layer_norm.cuh"

#include "data_loader.cuh"

class Transformer
{
public:
    Transformer(Data& in_training_data, float in_learning_rate):
        training_data(in_training_data),
        patch_embedding(training_data),
        CLS(patch_embedding.dim_model, patch_embedding.num_patches, patch_embedding.n_images),
        position_encoding(CLS.num_patches + 1, CLS.dim_model, CLS.n_images),
        layer_norm(position_encoding.sequence_len,position_encoding.dim_model,position_encoding.n_images),
        learning_rate(in_learning_rate)
    { }

    Tensor Transformer::forward()
    {
        patch_embedding.forward();

        std::cout << "\n===== PATCH EMBEDDING =====\n";
        patch_embedding.patches_tensor.print(500);


        CLS.previous = &patch_embedding.projection->output;
        CLS.forward();

        std::cout << "\n===== CLS =====\n";
        CLS.output.print(5);

        position_encoding.previous = &CLS.output;
        position_encoding.forward();

        std::cout << "\n===== POSITION ENCODING =====\n";
        CLS.output.print(5);    // como es in-place

        layer_norm.previous = position_encoding.previous;
        layer_norm.forward();

        std::cout << "\n===== LAYER NORM =====\n";
        layer_norm.output.print(5);

        return layer_norm.output;
    }

    void zero_grad()
    {
        patch_embedding.zero_grad();
        CLS.zero_grad();
        position_encoding.zero_grad();
        layer_norm.zero_grad();
    }

    void backward()
    {
        layer_norm.backward();
        layer_norm.update_weights(learning_rate);

        position_encoding.backward();
        position_encoding.update_weights(learning_rate);

        CLS.backward();
        CLS.update_weights(learning_rate);

        patch_embedding.backward(learning_rate); // Update weights is inside backward here


    }
private:
    Data& training_data;
    float learning_rate;

    PatchEmbedding patch_embedding;
    CLSToken CLS;
    PositionalEncoding position_encoding;
    LayerNorm layer_norm;
};

#endif