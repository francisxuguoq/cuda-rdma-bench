#include <cuda.h>
#include <sisci_api.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "gpu.h"
#include "reporting.h"


__global__ void gpu_memset_kernel(void* buf, size_t len, uint8_t val)
{
    const int num = gridDim.x * gridDim.y * blockDim.x * blockDim.y;
    const int x = blockIdx.x * blockDim.x + threadIdx.x;
    const int y = blockIdx.y * blockDim.y + threadIdx.y;
    const int pos = y * (gridDim.x * blockDim.x) + x;

    uint8_t* ptr = (uint8_t*) buf;

    for (size_t i = pos * (len / num), n = (pos + 1) * (len / num); i < n && i < len; ++i)
    {
        ptr[i] = val;
    }

    __syncthreads();
}


extern "C"
void gpu_memset(int gpu, void* ptr, size_t len, uint8_t val)
{
    cudaError_t err = cudaSetDevice(gpu);
    if (err != cudaSuccess)
    {
        log_error("Failed to set GPU: %s", cudaGetErrorString(err));
        return;
    }

    dim3 grid;
    grid.x = 4;
    grid.y = 4;

    dim3 block;
    block.x = 4;
    block.y = 4;

    log_debug("Filling buffer with value %02x...", val);

    gpu_memset_kernel<<<grid, block>>>(ptr, len, val);

    cudaDeviceSynchronize();
}


extern "C"
int gpu_memcmp(int gpu, void* gpuptr, volatile void* ramptr, size_t len)
{
    cudaError_t err = cudaSetDevice(gpu);
    if (err != cudaSuccess)
    {
        log_error("Failed to set GPU: %s", cudaGetErrorString(err));
        return 0;
    }

    cudaDeviceSynchronize();

    uint8_t* buf = NULL;
    err = cudaHostAlloc(&buf, len, cudaHostAllocDefault);
    if (err != cudaSuccess)
    {
        log_error("Failed to allocate host memory: %s", cudaGetErrorString(err));
        return 0;
    }

    err = cudaMemcpy(buf, gpuptr, len, cudaMemcpyDeviceToHost);
    if (err != cudaSuccess)
    {
        log_error("Failed to memcpy: %s", cudaGetErrorString(err));
        return 0;
    }

    cudaDeviceSynchronize();

    log_debug("Comparing local GPU memory %p to remote memory %p", gpuptr, ramptr);
    int equality = memcmp(buf, (void*) ramptr, len);

    cudaFreeHost(buf);
    return equality;
}


extern "C"
int gpu_device_count()
{
    cudaError_t err;
    int count = 0;

    err = cudaGetDeviceCount(&count);
    if (err != cudaSuccess)
    {
        log_error("Something went wrong: %s", cudaGetErrorString(err));
        return -1;
    }

    return count;
}


extern "C"
void* gpu_malloc(int gpu, size_t len)
{
    cudaError_t err = cudaSetDevice(gpu);
    if (err != cudaSuccess)
    {
        log_error("Failed to set GPU: %s", cudaGetErrorString(err));
        return NULL;
    }

    void* buf = NULL;
    err = cudaMalloc(&buf, len);
    if (err != cudaSuccess)
    {
        log_error("Failed to allocate device memory: %s", cudaGetErrorString(err));
        return NULL;
    }

    log_debug("Allocated device buffer %p", buf);
    return buf;
}


extern "C"
void gpu_free(int gpu, void* ptr)
{
    log_debug("Freing buffer %p", ptr);
    cudaSetDevice(gpu);
    cudaFree(ptr);
}


extern "C"
void devptr_set_sync_memops(void* dev_ptr)
{
    unsigned flag = 1;
    
    CUresult err = cuPointerSetAttribute(&flag, CU_POINTER_ATTRIBUTE_SYNC_MEMOPS, (CUdeviceptr) dev_ptr);

    if (err != CUDA_SUCCESS)
    {
        log_error("Failed to set pointer attribute CU_POINTER_ATTRIBYTE_SYNC_MEMOPS");
    }
}


extern "C"
void* gpu_devptr(int gpu, void* ptr)
{
    cudaPointerAttributes attrs;

    cudaError_t err = cudaSetDevice(gpu);
    if (err != cudaSuccess)
    {
        log_error("Failed to set GPU: %s", cudaGetErrorString(err));
        return NULL;
    }
        
    err = cudaPointerGetAttributes(&attrs, ptr);
    if (err != cudaSuccess)
    {
        log_error("Failed to get pointer attributes: %s", cudaGetErrorString(err));
        return NULL;
    }

    log_debug("CUDA device buffer %p has device ptr %p", ptr, attrs.devicePointer);
    return attrs.devicePointer;
}


extern "C"
void gpu_memcpy_buffer_to_local(int gpu, void* gpu_buf, void* ram_buf, size_t len)
{
    cudaError_t err = cudaSetDevice(gpu);
    if (err != cudaSuccess)
    {
        log_error("Failed to set GPU: %s", cudaGetErrorString(err));
        return;
    }

    cudaDeviceSynchronize();

    err = cudaMemcpy(ram_buf, gpu_buf, len, cudaMemcpyDeviceToHost);
    if (err != cudaSuccess)
    {
        log_error("Failed to memcpy: %s", cudaGetErrorString(err));
        return;
    }

    cudaDeviceSynchronize();
}



extern "C"
void gpu_prepare_memcpy(int gpu)
{
    cudaError_t err = cudaSetDevice(gpu);

    if (err != cudaSuccess)
    {
        log_error("Failed to set GPU: %s", cudaGetErrorString(err));
    }

    cudaDeviceSynchronize();
}



extern "C"
void gpu_memcpy_remote_to_local(void* local_buf, volatile void* remote_buf, size_t len)
{
    cudaDeviceSynchronize();

    cudaError_t err = cudaMemcpy(local_buf, (void*) remote_buf, len, cudaMemcpyHostToDevice);

    if (err != cudaSuccess)
    {
        log_error("%s", cudaGetErrorString(err));
    }
}


extern "C"
void gpu_memcpy_local_to_remote(void* local_buf, volatile void* remote_buf, size_t len)
{
    cudaDeviceSynchronize();

    cudaError_t err = cudaMemcpy((void*) remote_buf, local_buf, len, cudaMemcpyDeviceToHost);

    if (err != cudaSuccess)
    {
        log_error("%s", cudaGetErrorString(err));
    }
}


extern "C"
int gpu_info(int gpu, gpu_info_t* info)
{
    cudaError_t err;
    cudaDeviceProp prop;

    err = cudaGetDeviceProperties(&prop, gpu);
    if (err != cudaSuccess)
    {
        log_error("Unknown GPU %d: %s", gpu, cudaGetErrorString(err));
        return 0;
    }

    info->id = gpu;
    strncpy(info->name, prop.name, 256);
    info->domain = prop.pciBusID;
    info->bus = prop.pciDomainID;
    info->device = prop.pciDeviceID;

    return 1;
}
