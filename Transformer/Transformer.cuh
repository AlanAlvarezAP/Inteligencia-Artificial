#ifndef TRANSFORMER_H
#define TRANSFORMER_HA

#include <fstream>

#include "cuda_runtime.h"
#include "device_launch_parameters.h"

#include "CLS_token.cuh"
#include "PositionalEncoding.cuh"
#include "PatchEmbedding.cuh"

#include "EncoderBlock.cuh"
#include "ClassificationHead.cuh"

void write_tensor(std::ofstream& file, Tensor& t)
{
    auto data = t.get_data_CPU();
    for (float v : data)
        file << v << " ";
    file << "\n";
}

void read_tensor(std::ifstream& file, Tensor& t)
{
    std::vector<float> v(t.size);
    for (auto& x : v)
        file >> x;
    t.load(v);
}

class Transformer
{
public:
    float learning_rate;

    PatchEmbedding patch_embedding;
    CLSToken CLS;
    PositionalEncoding position_encoding;
    ClassificationHead class_head;

    std::vector<EncoderBlock> encoders;


    Transformer(int in_batch_size, float in_learning_rate, int n_encoder_blocks = 2):
        patch_embedding(in_batch_size),
        CLS(patch_embedding.dim_model, patch_embedding.num_patches, patch_embedding.n_images),
        position_encoding(CLS.num_patches + 1, CLS.dim_model, CLS.n_images),
        class_head(in_batch_size, position_encoding.dim_model, 10),
        learning_rate(in_learning_rate)
    {
        for (int i = 0; i < n_encoder_blocks; i++)
            encoders.push_back(EncoderBlock(position_encoding.n_images, position_encoding.sequence_len,
                                            position_encoding.dim_model, 4, in_learning_rate));
        
    }

    Tensor Transformer::forward()
    {
        patch_embedding.forward();

        //std::cout << "\n===== PATCH EMBEDDING =====\n";
        //patch_embedding.patches_tensor.print(500);


        CLS.previous = &patch_embedding.projection->output;
        CLS.forward();

        //std::cout << "\n===== CLS =====\n";
        //CLS.output.print(5);

        position_encoding.previous = &CLS.output;
        position_encoding.forward();

        //std::cout << "\n===== POSITION ENCODING =====\n";
        //CLS.output.print(5);

        //std::cout << "\n===== ENCODER =====\n";
        encoders[0].previous = position_encoding.previous;
        for (int i = 1; i < encoders.size(); i++)
            encoders[i].previous = &encoders[i - 1].output;

        for (auto& enc : encoders)
            enc.forward();

        //std::cout << "\n===== CLASSIFICATION HEAD =====\n";
        class_head.previous = &encoders.back().output;
        class_head.forward(position_encoding.sequence_len);

        return class_head.output;
    }

    void save(const std::string& path, float accuracy_training = 0.0f, float accuracy_eval = 0.0f)
    {
        std::ofstream file(path, std::ios::out);

        file << accuracy_training << "\n";
        file << accuracy_eval << "\n";
        file << learning_rate << "\n";

        file << patch_embedding.n_images << "\n";
        file << encoders.size() << "\n";

        write_tensor(file, patch_embedding.projection->weights);
        write_tensor(file, patch_embedding.projection->bias);

        write_tensor(file, CLS.cls);

        write_tensor(file, position_encoding.pos_emb);

        for (auto& enc : encoders)
        {
            write_tensor(file, enc.ln1.gamma);
            write_tensor(file, enc.ln1.beta);

            write_tensor(file, enc.mha.Wq);
            write_tensor(file, enc.mha.bq);
            write_tensor(file, enc.mha.Wk);
            write_tensor(file, enc.mha.bk);
            write_tensor(file, enc.mha.Wv);
            write_tensor(file, enc.mha.bv);
            write_tensor(file, enc.mha.Wo);
            write_tensor(file, enc.mha.bo);

            write_tensor(file, enc.ln2.gamma);
            write_tensor(file, enc.ln2.beta);

            write_tensor(file, enc.ff1.weights);
            write_tensor(file, enc.ff1.bias);
            write_tensor(file, enc.ff2.weights);
            write_tensor(file, enc.ff2.bias);
        }

        write_tensor(file, class_head.linear.weights);
        write_tensor(file, class_head.linear.bias);

        file.close();
    }

    void set_batch(std::vector<float>& batch_images)
    {
        patch_embedding.set_batch(batch_images);
    }

    void zero_grad()
    {
        patch_embedding.zero_grad();
        CLS.zero_grad();
        position_encoding.zero_grad();

        for (auto& enc : encoders)
            enc.zero_grad();

        class_head.zero_grad();
    }

    void backward(Tensor& expected)
    {
        class_head.backward(expected, learning_rate, position_encoding.sequence_len);

        for (int i = encoders.size() - 1; i >= 0; i--)
            encoders[i].backward(learning_rate);

        position_encoding.backward();
        position_encoding.update_weights(learning_rate);

        CLS.backward();
        CLS.update_weights(learning_rate);

        patch_embedding.backward(learning_rate); // Update weights is inside backward here


    }
};

Transformer* load_transformer(const std::string& path, float& accuracy_train, float& accuracy_eval)
{
    std::ifstream file(path);

    file >> accuracy_train;
    file >> accuracy_eval;

    float learning_rate;
    file >> learning_rate;

    int n_images;
    size_t n_encoder_blocks;
    file >> n_images >> n_encoder_blocks;

    Transformer* model = new Transformer(n_images, learning_rate, (int)n_encoder_blocks);

    read_tensor(file, model->patch_embedding.projection->weights);
    read_tensor(file, model->patch_embedding.projection->bias);

    read_tensor(file, model->CLS.cls);

    read_tensor(file, model->position_encoding.pos_emb);

    for (auto& enc : model->encoders)
    {
        read_tensor(file, enc.ln1.gamma);
        read_tensor(file, enc.ln1.beta);

        read_tensor(file, enc.mha.Wq);
        read_tensor(file, enc.mha.bq);
        read_tensor(file, enc.mha.Wk);
        read_tensor(file, enc.mha.bk);
        read_tensor(file, enc.mha.Wv);
        read_tensor(file, enc.mha.bv);
        read_tensor(file, enc.mha.Wo);
        read_tensor(file, enc.mha.bo);

        read_tensor(file, enc.ln2.gamma);
        read_tensor(file, enc.ln2.beta);

        read_tensor(file, enc.ff1.weights);
        read_tensor(file, enc.ff1.bias);
        read_tensor(file, enc.ff2.weights);
        read_tensor(file, enc.ff2.bias);
    }

    read_tensor(file, model->class_head.linear.weights);
    read_tensor(file, model->class_head.linear.bias);

    return model;
}

#endif
