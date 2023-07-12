#pragma once
#include "optimizer.h"
#include <vector>
#include "common.h"

template<int WIDTH>
void sgd_step(id<1> idx,
	const int n_elements,
	int output_width,
	int n_hidden_layers,
	const float loss_scale,
	const float learning_rate,
	const float l2_reg,
	bf16* weights,
	bf16* gradients
) {
	const int matrices_number = idx / (WIDTH * WIDTH);
	const int matrices_offset = idx % (WIDTH * WIDTH);
	int packed_idx_matrices = 0;

	if (matrices_number < n_hidden_layers) {
		packed_idx_matrices = toPackedLayoutCoord(idx, WIDTH, WIDTH);
	}
	else {
		packed_idx_matrices = toPackedLayoutCoord(idx, WIDTH, output_width);
	}

	const int packed_idx = matrices_number * WIDTH * WIDTH + packed_idx_matrices;
	const bf16 weight = weights[idx];
	float gradient = gradients[packed_idx] / loss_scale;

	gradient += l2_reg * weight;

	const bf16 new_weight = weight - learning_rate * gradient;

	weights[idx] = new_weight;
}

template<int WIDTH>
void sgd_stepT(id<1> idx,
	const int n_elements,
	int output_width,
	int n_hidden_layers,
	const float loss_scale,
	const float learning_rate,
	const float l2_reg,
	bf16* weightsT,
	bf16* gradients
) {
	const int i = idx / WIDTH;
	const int j = idx % WIDTH;

	const int T_idx = WIDTH * j + i;

	const int matrices_number = T_idx / (WIDTH * WIDTH);
	const int matrices_offset = T_idx % (WIDTH * WIDTH);
	int packed_idx_matrices = 0;

	if (matrices_number < n_hidden_layers) {
		int packed_idx_matrices = toPackedLayoutCoord(idx, WIDTH, WIDTH);
	}
	else {
		int packed_idx_matrices = toPackedLayoutCoord(idx, WIDTH, output_width);
	}

	const int packed_idx = matrices_number * WIDTH * WIDTH + packed_idx_matrices;
	const bf16 weightT = weightsT[idx];
	float gradient = gradients[packed_idx] / loss_scale;

	gradient += l2_reg * weightT;

	const bf16 new_weightT = weightT - learning_rate * gradient;

	weightsT[idx] = new_weightT;
}

template <int WIDTH>
class SGDOptimizer : public Optimizer {
public:

	SGDOptimizer(int output_rows,int n_hidden_layers, float learning_rate, float l2_reg) {
		m_output_rows =output_rows ;
		m_n_hidden_layers = n_hidden_layers;
		m_learning_rate = learning_rate ;
		m_l2_reg= l2_reg;
	}

	void step(queue q, float loss_scale, DeviceMem<bf16>& weights, DeviceMem<bf16>& weightsT, DeviceMem<bf16>& gradients) const  override {

		const int n_elements = weights.size();
		float learning_rate = m_learning_rate;
		float l2_reg = m_l2_reg;
		const int output_rows = m_output_rows;
		const int n_hidden_layers = m_n_hidden_layers;

		q.parallel_for<>(range<1>(n_elements), [=](id<1> idx) {
			sgd_step<WIDTH>(idx, n_elements, output_rows, n_hidden_layers, loss_scale, learning_rate, l2_reg, weights.data(), gradients.data());
			}).wait();

		q.parallel_for<>(range<1>(n_elements), [=](id<1> idx) {
			sgd_stepT<WIDTH>(idx, n_elements, output_rows, n_hidden_layers, loss_scale, learning_rate, l2_reg, weightsT.data(), gradients.data());
			}).wait();


	}

	void set_learning_rate(const float learning_rate) {
		m_learning_rate = learning_rate;
	}

private:

	int m_output_rows;
	int m_n_hidden_layers;
	float m_learning_rate = 1e-3f;
	float m_l2_reg = 1e-8f;
};
