#pragma once
#pragma once

#include "loss.h"

template<typename T>
void Relative_L2_loss(nd_item<1> item,
	const int n_elements,
	const int dims,
	const int stride,
	const float scale,
	const T* preds,
	const float* targets,
	T* grads,
	float* values) {

	// get the index of the element considered by the kernel
	const int idx = item.get_global_linear_id();

	const int intra_idx = idx % stride;
	const int inter_idx = idx / stride;

	const int N_total_elements = n_elements * dims / stride;

	const int target_idx = inter_idx * dims + intra_odx;

	const float pred = (float)preds[i];
	const float target = targets[target_idx];
	const float difference = pred - target;
	const float var = pred * pred + 0.01f;

	values[i] = difference * difference / var / N_total_elements;

	grads[i] = (T)(loss_scale * 2 * difference / var / N_total_elements);

}

template< typename T>
class RelativeL2Loss : public Loss<T> {
public:
	void evaluate(
		queue q,
		const int dims,
		const int stride,
		const float scale,
		const std::vector<T> pred,
		const std::vector<float> target,
		std::vector<T> grads,
		std::vector<float> values
	) const override {
		q.submit([&](handler& h) {
			h.parallel_for<class imatrix>(nd_range<1>(preds.size() / 128, 128), [=](nd_item<1> it) [[intel::reqd_sub_group_size(16)]] {
				Relative_L2_loss<T>(preds.size(),
					dims,
					stride,
					scale,
					preds,
					targets,
					grads,
					values);

				}
				};