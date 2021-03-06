#ifndef __GPU_H__
#define __GPU_H__
#ifdef __cplusplus
extern "C" {
#endif

#include <stdlib.h>
#include <stdint.h>

/* GPU info descriptor */
typedef struct {
    int     id;         // CUDA device ID
    char    name[256];  // CUDA device name
    int     domain;     // PCI domain
    int     bus;        // PCI bus
    int     device;     // PCI device
} gpu_info_t;


/* Get the GPU info for a given GPU
 *
 * gpu_device_id        - the local GPU id
 * info                 - is set to the GPU info on success
 *
 * Returns 1 and sets info on success, 0 on error
 */
int gpu_info(int gpu_device_id, gpu_info_t* info);


/* Allocate GPU device buffer
 *
 * gpu_device_id        - local GPU to allocate device memory from
 * buf_size             - the size of the buffer
 *
 * Returns a CUDA device memory pointer on success or NULL on error
 */
void* gpu_malloc(int gpu_device_id, size_t buf_size);


/* Free GPU device buffer
 *
 * gpu_device_id        - local GPU where the device memory is allocated
 * buf_ptr              - pointer to the buffer that is to be free'd
 *
 * No return value
 */
void gpu_free(int gpu_device_id, void* buf_ptr);


/* Do memset on a GPU device buffer
 *
 * gpu                  - local GPU where the memory is allocated
 * buf                  - pointer to the buffer
 * len                  - size of the buffer
 * val                  - the value to set
 *
 * No return value
 */
void gpu_memset(int gpu, void* buf, size_t len, uint8_t val);


/* Do memcmp on a GPU device buffer and a RAM memory buffer
 *
 * gpu                  - local GPU where the device buffer is allocated
 * gpu_ptr              - pointer to the GPU buffer
 * ram_ptr              - pointer to the RAM buffer
 * len                  - size of the buffer
 *
 * Returns 0 if the buffers are equal or a non-zero value otherwise
 */
int gpu_memcmp(int gpu, void* gpu_ptr, volatile void* remote_ptr, size_t len);


/* Copy data from GPU device buffer to RAM buffer
 *
 * gpu                  - local GPU where the device buffer is allocated
 * gpu_ptr              - device buffer pointer
 * local_ptr            - RAM buffer pointer
 * len                  - size of the buffers
 *
 * No return value
 */
void gpu_memcpy_buffer_to_local(int gpu, void* gpu_ptr, void* local_ptr, size_t len);


/* Prepare a copy between remote buffer and local GPU device buffer
 *
 * gpu                  - local GPU where the device buffer is allocated
 * remote_ptr           - pointer to remote memory
 * size                 - size of remote memory
 *
 * Returns 0 if success
 */
int gpu_prepare_memcpy(int gpu, unsigned flags, volatile void* remote_ptr, size_t size);


uint64_t gpu_memcpy_remote_to_local(volatile void* gpu_ptr, volatile void* remote_ptr, size_t len, int clear);

uint64_t gpu_memcpy_local_to_remote(volatile void* gpu_ptr, volatile void* remote_ptr, size_t len, int clear);



/* Get the device pointer for a device buffer
 *
 * Translates a CUDA device memory pointer into a device pointer
 *
 * gpu_device_id        - local GPU where the device memory is allocated
 * buf_ptr              - pointer to the CUDA device buffer
 *
 * Returns the devicePointer associated with the CUDA memory pointer
 */
void* gpu_devptr(int gpu_device_id, void* buf_ptr);


/* Set the CU_POINTER_ATTRIBUTE_SYNC_MEMOPS flag on a device pointer
 *
 * dev_ptr              - the device pointer
 *
 * No return value
 */
void devptr_set_sync_memops(void* dev_ptr);


#ifdef __cplusplus
}
#endif
#endif
