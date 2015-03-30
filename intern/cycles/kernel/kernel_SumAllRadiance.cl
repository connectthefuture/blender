#include "kernel_compat_opencl.h"
#include "kernel_math.h"
#include "kernel_types.h"
#include "kernel_globals.h"

/*
* Since we process various samples in parallel; The output radiance of different samples
* are stored in different locations; This kernel combines the output radiance contributed
* by all different samples and stores them in the RenderTile's output buffer.
*/

__kernel void kernel_ocl_path_trace_SumAllRadiance_SPLIT_KERNEL(
	ccl_constant KernelData *data,               /* To get pass_stride to offet into buffer */
	ccl_global float *buffer,                    /* Output buffer of RenderTile */
	ccl_global float *per_sample_output_buffer,  /* Radiance contributed by all samples */
	int parallel_samples, int sw, int sh, int stride)
{
	int x = get_global_id(0);
	int y = get_global_id(1);

	if(x < sw && y < sh) {
		buffer += (x + y * stride) * (data->film.pass_stride);
		per_sample_output_buffer += ((x + y * stride) * parallel_samples) * (data->film.pass_stride);

		int sample_stride = (data->film.pass_stride);

		int sample_iterator = 0;
		int pass_stride_iterator = 0;
		int num_floats = data->film.pass_stride;

		for(sample_iterator = 0; sample_iterator < parallel_samples; sample_iterator++) {
			for(pass_stride_iterator = 0; pass_stride_iterator < num_floats; pass_stride_iterator++) {
				*(buffer + pass_stride_iterator) = (sample_iterator == 0) ? *(per_sample_output_buffer + pass_stride_iterator)
				: *(buffer + pass_stride_iterator) + *(per_sample_output_buffer + pass_stride_iterator);
			}
			per_sample_output_buffer += sample_stride;
		}

	}
}