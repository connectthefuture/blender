#include "kernel_split.h"

/*
 * Note on kernel_ocl_path_trace_ShaderEvaluation_SPLIT_KERNEL kernel
 * This kernel is the 5th kernel in the ray tracing logic. This is
 * the 4rd kernel in path iteration. This kernel sets up the ShaderData
 * structure from the values computed by the previous kernels. It also identifies
 * the rays of state RAY_TO_REGENERATE and enqueues them in QUEUE_HITBG_BUFF_UPDATE_TOREGEN_RAYS queue.
 *
 * The input and output of the kernel is as follows,
 * rng_coop -------------------------------------------|--- kernel_ocl_path_trace_ShaderEvaluation_SPLIT_KERNEL ---|--- shader_data
 * Ray_coop -------------------------------------------|                                                           |--- Queue_data (QUEUE_HITBG_BUFF_UPDATE_TOREGEN_RAYS)
 * PathState_coop -------------------------------------|                                                           |--- Queue_index (QUEUE_HITBG_BUFF_UPDATE_TOREGEN_RAYS)
 * Intersection_coop ----------------------------------|                                                           |
 * Queue_data (QUEUE_ACTIVE_AND_REGENERATD_RAYS)-------|                                                           |
 * Queue_index(QUEUE_HITBG_BUFF_UPDATE_TOREGEN_RAYS)---|                                                           |
 * ray_state ------------------------------------------|                                                           |
 * kg (globals + data) --------------------------------|                                                           |
 * queuesize ------------------------------------------|                                                           |
 *
 * Note on Queues :
 * This kernel reads from the QUEUE_ACTIVE_AND_REGENERATED_RAYS queue and processes
 * only the rays of state RAY_ACTIVE;
 * State of queues when this kernel is called,
 * at entry,
 * QUEUE_ACTIVE_AND_REGENERATED_RAYS will be filled with RAY_ACTIVE and RAY_REGENERATED rays
 * QUEUE_HITBG_BUFF_UPDATE_TOREGEN_RAYS will be empty.
 * at exit,
 * QUEUE_ACTIVE_AND_REGENERATED_RAYS will be filled with RAY_ACTIVE and RAY_REGENERATED rays
 * QUEUE_HITBG_BUFF_UPDATE_TOREGEN_RAYS will be filled with RAY_TO_REGENERATE rays
 */

__kernel void kernel_ocl_path_trace_ShaderEvaluation_SPLIT_KERNEL(
	ccl_global char *globals,
	ccl_constant KernelData *data,
	ccl_global char *shader_data,               /* Output ShaderData structure to be filled */
	ccl_global uint *rng_coop,                  /* Required for rbsdf calculation */
	ccl_global Ray *Ray_coop,                   /* Required for setting up shader from ray */
	ccl_global PathState *PathState_coop,       /* Required for all functions in this kernel */
	ccl_global Intersection *Intersection_coop, /* Required for setting up shader from ray */
	ccl_global char *ray_state,                 /* Denotes the state of each ray */
	ccl_global int *Queue_data,                 /* queue memory */
	ccl_global int *Queue_index,                /* Tracks the number of elements in each queue */
	int queuesize                               /* Size (capacity) of each queue */
	)
{
	int ray_index = get_global_id(1) * get_global_size(0) + get_global_id(0);
	/* Enqeueue RAY_TO_REGENERATE rays into QUEUE_HITBG_BUFF_UPDATE_TOREGEN_RAYS queue */
	__local unsigned int local_queue_atomics;
	if(get_local_id(0) == 0 && get_local_id(1) == 0) {
		local_queue_atomics = 0;
	}
	barrier(CLK_LOCAL_MEM_FENCE);

	char enqueue_flag = (IS_STATE(ray_state, ray_index, RAY_TO_REGENERATE)) ? 1 : 0;

	enqueue_ray_index_local(ray_index, QUEUE_HITBG_BUFF_UPDATE_TOREGEN_RAYS, enqueue_flag, queuesize, &local_queue_atomics, Queue_data, Queue_index);

	ray_index = get_ray_index(ray_index, QUEUE_ACTIVE_AND_REGENERATED_RAYS, Queue_data, queuesize, 0);

	if(ray_index == QUEUE_EMPTY_SLOT)
		return;

    /* Continue on with shader evaluation */
	if(IS_STATE(ray_state, ray_index, RAY_ACTIVE)) {
	    ccl_global KernelGlobals *kg = (ccl_global KernelGlobals *)globals;
	    ccl_global ShaderData *sd = (ccl_global ShaderData *)shader_data;
		ccl_global Intersection *isect = &Intersection_coop[ray_index];
		ccl_global uint *rng = &rng_coop[ray_index];
		ccl_global PathState *state = &PathState_coop[ray_index];
		Ray ray = Ray_coop[ray_index];

		shader_setup_from_ray(kg, sd, isect, &ray, state->bounce, state->transparent_bounce);
		float rbsdf = path_state_rng_1D_for_decision(kg, rng, state, PRNG_BSDF);
		shader_eval_surface(kg, sd, rbsdf, state->flag, SHADER_CONTEXT_MAIN);
	}
}