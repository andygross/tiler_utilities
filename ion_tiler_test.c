#include <errno.h>
#include <fcntl.h>
#include <getopt.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdint.h>
#include "ion_user.h"
#include "testlib.h"

int align = 0;
int prot = PROT_READ | PROT_WRITE;
int map_flags = MAP_SHARED;
int alloc_flags = 0;
int test = -1;
//int fmt = TILER_PIXEL_FMT_32BIT;
int tiler_test = 1;
size_t stride;
#undef __WRITE_IN_STRIDE__
#define PAGE_SIZE 0x1000
#define MAX_ALLOCS 512
#if 0
enum pixel_fmt_t {
    TILER_PIXEL_FMT_MIN   = 0,
    TILER_PIXEL_FMT_8BIT  = 0,
    TILER_PIXEL_FMT_16BIT = 1,
    TILER_PIXEL_FMT_32BIT = 2,
    TILER_PIXEL_FMT_PAGE  = 3,
    TILER_PIXEL_FMT_MAX   = 3,
};

typedef enum pixel_fmt_t pixel_fmt_t;

#endif
#define TESTS\
    T(alloc_1D_test(4096, 0))\
    T(alloc_1D_test(176 * 144 * 2, 0))\
    T(alloc_1D_test(640 * 480 * 2, 0))\
    T(alloc_1D_test(848 * 480 * 2, 0))\
    T(alloc_1D_test(1920 * 1080 * 2, 0))\
    T(alloc_2D_test(64, 64, TILER_PIXEL_FMT_8BIT))\
    T(alloc_2D_test(64, 64, TILER_PIXEL_FMT_16BIT))\
    T(alloc_2D_test(64, 64, TILER_PIXEL_FMT_32BIT))\
    T(alloc_2D_test(176, 144, TILER_PIXEL_FMT_8BIT))\
    T(alloc_2D_test(176, 144, TILER_PIXEL_FMT_16BIT))\
    T(alloc_2D_test(176, 144, TILER_PIXEL_FMT_32BIT))\
    T(alloc_2D_test(640, 480, TILER_PIXEL_FMT_16BIT))\
    T(alloc_2D_test(640, 480, TILER_PIXEL_FMT_32BIT))\
    T(alloc_2D_test(848, 480, TILER_PIXEL_FMT_8BIT))\
    T(alloc_2D_test(848, 480, TILER_PIXEL_FMT_16BIT))\
    T(alloc_2D_test(848, 480, TILER_PIXEL_FMT_32BIT))\
    T(alloc_2D_test(1280, 720, TILER_PIXEL_FMT_8BIT))\
    T(alloc_2D_test(1280, 720, TILER_PIXEL_FMT_16BIT))\
    T(alloc_2D_test(1280, 720, TILER_PIXEL_FMT_32BIT))\
    T(alloc_2D_test(1920, 1080, TILER_PIXEL_FMT_8BIT))\
    T(alloc_2D_test(1920, 1080, TILER_PIXEL_FMT_16BIT))\
    T(alloc_2D_test(1920, 1080, TILER_PIXEL_FMT_32BIT))\
    T(alloc_2D_test(8193, 16, TILER_PIXEL_FMT_8BIT))\
    T(alloc_2D_test(8193, 16, TILER_PIXEL_FMT_16BIT))\
    T(alloc_2D_test(4097, 16, TILER_PIXEL_FMT_32BIT))\
    T(alloc_2D_test(16384, 16, TILER_PIXEL_FMT_8BIT))\
    T(alloc_2D_test(16384, 16, TILER_PIXEL_FMT_16BIT))\
    T(alloc_2D_test(8192, 16, TILER_PIXEL_FMT_32BIT))\
    T(!alloc_2D_test(16385, 16, TILER_PIXEL_FMT_8BIT))\
    T(!alloc_2D_test(16385, 16, TILER_PIXEL_FMT_16BIT))\
    T(!alloc_2D_test(8193, 16, TILER_PIXEL_FMT_32BIT))\
    T(maxalloc_1D_test(4096, MAX_ALLOCS))\
    T(maxalloc_1D_test(176 * 144 * 2, MAX_ALLOCS))\
    T(maxalloc_1D_test(640 * 480 * 2, MAX_ALLOCS))\
    T(maxalloc_1D_test(848 * 480 * 2, MAX_ALLOCS))\
    T(maxalloc_1D_test(1280 * 720 * 2, MAX_ALLOCS))\
    T(maxalloc_1D_test(1920 * 1080 * 2, MAX_ALLOCS))\
    T(map_1D_test(4096, 0))\
    T(map_1D_test(176 * 144 * 2, 0))\
    T(map_1D_test(640 * 480 * 2, 0))\
    T(map_1D_test(848 * 480 * 2, 0))\
    T(map_1D_test(1280 * 720 * 2, 0))\
    T(map_1D_test(1920 * 1080 * 2, 0))\
    T(map_1D_test(4096, 0))\
    T(map_1D_test(8192, 0))\
    T(map_1D_test(16384, 0))\
    T(map_1D_test(32768, 0))\
    T(map_1D_test(65536, 0))\
    T(alloc_2D_test(176, 144, TILER_PIXEL_FMT_32BIT))\
    T(alloc_NV12_test(176, 144))\
    T(alloc_NV12_test(640, 480))\
    T(alloc_NV12_test(848, 480))\
    T(alloc_NV12_test(1280, 720))\
   T(alloc_NV12_test(1920, 1080))\
   T(maxalloc_2D_test(2500, 32, TILER_PIXEL_FMT_8BIT, MAX_ALLOCS))\
   T(maxalloc_2D_test(2500, 16, TILER_PIXEL_FMT_16BIT, MAX_ALLOCS))\
   T(maxalloc_2D_test(1250, 16, TILER_PIXEL_FMT_32BIT, MAX_ALLOCS))\
   T(maxalloc_2D_test(5000, 32, TILER_PIXEL_FMT_8BIT, MAX_ALLOCS))\
   T(maxalloc_2D_test(5000, 16, TILER_PIXEL_FMT_16BIT, MAX_ALLOCS))\
   T(maxalloc_2D_test(2500, 16, TILER_PIXEL_FMT_32BIT, MAX_ALLOCS))\
   T(maxalloc_2D_test(64, 64, TILER_PIXEL_FMT_8BIT, MAX_ALLOCS))\
   T(maxalloc_2D_test(64, 64, TILER_PIXEL_FMT_16BIT, MAX_ALLOCS))\
   T(maxalloc_2D_test(64, 64, TILER_PIXEL_FMT_32BIT, MAX_ALLOCS))\
   T(maxalloc_NV12_test(64, 64, MAX_ALLOCS))\
   T(maxalloc_2D_test(176, 144, TILER_PIXEL_FMT_8BIT, MAX_ALLOCS))\
   T(maxalloc_2D_test(176, 144, TILER_PIXEL_FMT_16BIT, MAX_ALLOCS))\
   T(maxalloc_2D_test(176, 144, TILER_PIXEL_FMT_32BIT, MAX_ALLOCS))\
   T(maxalloc_NV12_test(176, 144, MAX_ALLOCS))\
   T(maxalloc_2D_test(640, 480, TILER_PIXEL_FMT_8BIT, MAX_ALLOCS))\
   T(maxalloc_2D_test(640, 480, TILER_PIXEL_FMT_16BIT, MAX_ALLOCS))\
   T(maxalloc_2D_test(640, 480, TILER_PIXEL_FMT_32BIT, MAX_ALLOCS))\
   T(maxalloc_NV12_test(640, 480, MAX_ALLOCS))\
   T(maxalloc_2D_test(848, 480, TILER_PIXEL_FMT_8BIT, MAX_ALLOCS))\
   T(maxalloc_2D_test(848, 480, TILER_PIXEL_FMT_16BIT, MAX_ALLOCS))\
   T(maxalloc_2D_test(848, 480, TILER_PIXEL_FMT_32BIT, MAX_ALLOCS))\
   T(maxalloc_NV12_test(848, 480, MAX_ALLOCS))\
   T(maxalloc_2D_test(1280, 720, TILER_PIXEL_FMT_8BIT, MAX_ALLOCS))\
   T(maxalloc_2D_test(1280, 720, TILER_PIXEL_FMT_16BIT, MAX_ALLOCS))\
   T(maxalloc_2D_test(1280, 720, TILER_PIXEL_FMT_32BIT, MAX_ALLOCS))\
   T(maxalloc_NV12_test(1280, 720, MAX_ALLOCS))\
   T(maxalloc_2D_test(1920, 1080, TILER_PIXEL_FMT_8BIT, MAX_ALLOCS))\
   T(maxalloc_2D_test(1920, 1080, TILER_PIXEL_FMT_16BIT, MAX_ALLOCS))\
   T(maxalloc_2D_test(1920, 1080, TILER_PIXEL_FMT_32BIT, MAX_ALLOCS))\
   T(maxalloc_NV12_test(1920, 1080, 2))\
   T(maxalloc_NV12_test(1920, 1080, MAX_ALLOCS))\
   T(negative_fmt_test(-1)) \
   T(negative_fmt_test(8)) \
   T(negative_2d_test(0, 1080, TILER_PIXEL_FMT_16BIT)) \
   T(negative_2d_test(1920, 0, TILER_PIXEL_FMT_16BIT)) \
   T(negative_1dl_test(0)) \
   T(negative_1dh_test(3)) \
   T(negative_free_2D_test(1920, 1080, TILER_PIXEL_FMT_16BIT)) \
   T(negative_free_1D_test(176 * 144 * 2, 0)) \
   T(negative_arbitvalue_test(176 * 144 * 2, 0)) \
   T(random_alloc_test(1000,10)) \

int check_mem(uint16_t start, uint16_t *ptr, struct omap_ion_tiler_alloc_data *al_data);
void fill_mem(uint16_t start, uint16_t *ptr, struct omap_ion_tiler_alloc_data *al_data);

int _ion_alloc_test(int fd, struct ion_handle **handle, struct omap_ion_tiler_alloc_data *al_data )
{
	int ret;

	if (tiler_test)
		ret = ion_alloc_tiler(fd, &al_data, handle, &stride);
	if (ret)
		printf("%s failed: %s\n", __func__, strerror(ret));
	return ret;
}

int alloc_1D_test(uint32_t length, size_t stride)
{
	int fd, map_fd, ret;
	struct ion_handle *handle;
	uint16_t *ptr;

	struct omap_ion_tiler_alloc_data alloc_data = {
		.w = length,
		.h = 1,
                .fmt = TILER_PIXEL_FMT_PAGE,
        };

	uint16_t val = (uint16_t) rand();
        fd = ion_open();
        if (fd < 0)
                return fd;

	ret = _ion_alloc_test(fd, &handle, &alloc_data);
		if (ret == -ENOMEM)
			goto exit;

	if (tiler_test)
              length = alloc_data.h * alloc_data.stride;
        ret = ion_map(fd, handle, length, prot, map_flags, 0, &ptr, &map_fd);
        if (ret)
                return;

        if (tiler_test)

		 fill_mem(val, ptr, &alloc_data);
		 check_mem(val, ptr, &alloc_data);

	munmap(ptr, length);
	ret = ion_free(fd, handle);
	if (ret) {
		printf("%s failed: %s %p\n", __func__, strerror(ret), handle);
		return;
	}

	close(map_fd);
exit:
	ion_close(fd);
}

int maxalloc_1D_test(uint32_t length, int max_allocs)
{
    printf("Allocate & Free max # of %ub 1D buffers\n", length);

    int fd, ret, map_fd;
    struct ion_handle *handle;
    uint16_t *ptr;

    struct omap_ion_tiler_alloc_data alloc_data = {
                .w = length,
                .h = 1,
                .fmt = TILER_PIXEL_FMT_PAGE,
        };

    struct data {
        unsigned int  val;
        void    *bufPtr;
    } *mem;

      fd = ion_open();
       if (fd < 0)
          return fd;

     uint16_t val = (uint16_t) rand();

    /* allocate as many buffers as we can */
    mem = (struct data *)calloc(max_allocs, sizeof(struct data));
    //void *ptr = (void *)mem;
    int ix, res = 0;
    for (ix = 0;  ix < max_allocs;)
    {
        ret = _ion_alloc_test(fd, &handle, &alloc_data);
	if (ret == -ENOMEM)
		goto exit;

	if (handle)
	   {
            mem[ix].bufPtr = handle;
	    printf("handle allocated count = %d handle =%x\n", ix, mem[ix].bufPtr);

	length = alloc_data.h * alloc_data.stride;
        ret = ion_map(fd, handle, length, prot, map_flags, 0, &ptr, &map_fd);
        if (ret)
                return;

        if (tiler_test)

                 fill_mem(val, ptr, &alloc_data);
                 check_mem(val, ptr, &alloc_data);

        	munmap(ptr, length);
		close(map_fd);
           ix++; 
    	}	
    }
    printf(":: Allocated %d buffers", ix);

exit:
    while (ix--)
        {	
	printf("handle de-allocated count = %d handle =%x\n", ix, mem[ix].bufPtr);
	ret = ion_free(fd,  mem[ix].bufPtr);
	if (ret) {
                printf("%s failed: %s %p\n", __func__, strerror(ret), mem[ix].bufPtr);
                return;
        }
    }
	free(mem);
        ion_close(fd);
    	return res;
 }
int alloc_2D_test(uint32_t width, uint32_t height, int fmt)
{
        int fd, map_fd, ret;
	uint32_t length;
	uint16_t *ptr;
        struct ion_handle *handle;
        struct omap_ion_tiler_alloc_data alloc_data = {
                .w = width,
                .h = height,
                .fmt = 0,
        };

	uint16_t val = (uint16_t) rand();

        fd = ion_open();
        if (fd < 0)
                return fd;


        ret = _ion_alloc_test(fd, &handle, &alloc_data);
		if (ret == -ENOMEM)
		   goto exit;

	if (tiler_test)
              length = height * alloc_data.stride;

	      printf("2d length = %x\n", length);
        ret = ion_map(fd, handle, length, prot, map_flags, 0, &ptr, &map_fd);
        if (ret)
                return;

		fill_mem(val, ptr, &alloc_data);
                check_mem(val, ptr, &alloc_data);


        munmap(ptr, length);

        ret = ion_free(fd, handle);
        if (ret) {
                printf("%s failed: %s %p\n", __func__, strerror(ret), handle);
                return;
        }
	close(map_fd);
exit:
        ion_close(fd);
}

int maxalloc_2D_test(uint32_t width, uint32_t height, int fmt, int max_allocs)
{
   // printf("Allocate & Free max # of %ub 1D buffers\n", length);

    int fd, ret, map_fd;
    struct ion_handle *handle;
    uint16_t *ptr;
    uint32_t length;

    struct omap_ion_tiler_alloc_data alloc_data = {
                .w = width,
                .h = height,
                .fmt = fmt,
        };

    struct data {
        void    *bufPtr;
    } *mem;

      fd = ion_open();
       if (fd < 0)
          return fd;

     uint16_t val = (uint16_t) rand();

    /* allocate as many buffers as we can */
    mem = (struct data *)calloc(max_allocs, sizeof(struct data));
    //void *ptr = (void *)mem;
    int ix, res = 0;
    for (ix = 0;  ix < max_allocs;)
    {
        ret = _ion_alloc_test(fd, &handle, &alloc_data);
        if (ret == -ENOMEM)
                goto exit;

        if (handle)
           {
            mem[ix].bufPtr = handle;
	    printf("handle allocated count = %d handle =%x\n", ix, mem[ix].bufPtr);
	    
	    length = height * alloc_data.stride;
            printf("2d length = %x\n", length);
           ret = ion_map(fd, handle, length, prot, map_flags, 0, &ptr, &map_fd);

        if (ret)
                return;

                fill_mem(val, ptr, &alloc_data);
                check_mem(val, ptr, &alloc_data);

        	munmap(ptr, length);
        	close(map_fd);	    
        	ix++;
        }
    }
    printf(":: Allocated %d buffers", ix);

exit:

    while (ix--)
        {
	 ret = ion_free(fd, mem[ix].bufPtr);
	 printf("handle de-allocated count = %d handle =%x\n", ix, mem[ix].bufPtr);

	if (ret) {
                printf("%s failed: %s %p\n", __func__, strerror(ret), handle);
                return;
        }
    }
	free(mem);
        ion_close(fd);
        return res;
}

int alloc_NV12_test(uint32_t width, uint32_t height)
{
        int fd, map_fd1, map_fd2, ret;
        uint32_t length1, length2;
        uint16_t *ptr1, *ptr2;
        struct ion_handle *handle_y, *handle_uv;

        struct omap_ion_tiler_alloc_data alloc_data_y = {
                .w = width,
                .h = height,
                .fmt = TILER_PIXEL_FMT_8BIT,
        };

	struct omap_ion_tiler_alloc_data alloc_data_uv = {
                .w = width >> 1,
                .h = height >> 1,
                .fmt = TILER_PIXEL_FMT_16BIT,
        };	
	

        fd = ion_open();
        if (fd < 0)
                return fd;
	
	uint16_t val = (uint16_t) rand();


        ret = _ion_alloc_test(fd, &handle_y, &alloc_data_y);
        if (ret == -ENOMEM)
           goto exit;
	ret = _ion_alloc_test(fd, &handle_uv, &alloc_data_uv);
        if (ret == -ENOMEM)
	   goto exit;

        if (tiler_test)
              length1 = alloc_data_y.h * alloc_data_y.stride;
        ret = ion_map(fd, handle_y, length1, prot, map_flags, 0, &ptr1, &map_fd1);
        if (ret)
                return;

	length2 = alloc_data_uv.h * alloc_data_uv.stride;
	ret = ion_map(fd, handle_uv, length2, prot, map_flags, 0, &ptr2, &map_fd2);
	if (ret)
		return;

		fill_mem(val, ptr1, &alloc_data_y);
                check_mem(val, ptr1, &alloc_data_y);
		
		fill_mem(val, ptr2, &alloc_data_uv);
                check_mem(val, ptr2, &alloc_data_uv);

        munmap(ptr1, length1);
	munmap(ptr2, length2);

        ret = ion_free(fd, handle_y);

	if (ret) {
                printf("%s failed: %s %p\n", __func__, strerror(ret), handle_y);
                return;
        }

	ret = ion_free(fd, handle_uv);

        if (ret) {
                printf("%s failed: %s %p\n", __func__, strerror(ret), handle_uv);
                return;
        }

        close(map_fd1);
	close(map_fd2);
exit:
	ion_close(fd);
}

int maxalloc_NV12_test(uint32_t width, uint32_t height, uint32_t max_allocs)
{
        int fd, map_fd1, map_fd2, ret;
        uint32_t length, length1, length2;
        uint16_t *ptr1, *ptr2;
        struct ion_handle *handle_y, *handle_uv;

        struct omap_ion_tiler_alloc_data alloc_data_y = {
                .w = width,
                .h = height,
                .fmt = TILER_PIXEL_FMT_8BIT,
        };

        struct omap_ion_tiler_alloc_data alloc_data_uv = {
                .w = width >> 1,
                .h = height >> 1,
                .fmt = TILER_PIXEL_FMT_16BIT,
        };

	struct data {
        unsigned int  val;
        void    *bufPtr1;
	void    *bufPtr2;
    } *mem;


	fd = ion_open();
        if (fd < 0)
                return fd;

	uint16_t val = (uint16_t) rand();

    /* allocate as many buffers as we can */
    mem = (struct data *)calloc(max_allocs, sizeof(struct data));
    int ix, res = 0;
    
	for (ix = 0;  ix < max_allocs;) {

        ret = _ion_alloc_test(fd, &handle_y, &alloc_data_y);
		if (ret == -ENOMEM)
			goto exit;
		
        ret = _ion_alloc_test(fd, &handle_uv, &alloc_data_uv);
		if (ret == -ENOMEM)
			goto exit;

	if (handle_y && handle_uv)
		{
			mem[ix].bufPtr1 = handle_y;
			mem[ix].bufPtr2 = handle_uv;
			printf("handle allocated count = %d handle_y =%x handle_uv= %x\n", ix, mem[ix].bufPtr1, mem[ix].bufPtr2);
            		ix++;
        	}		
	}
     
        if (tiler_test)
              length1 = alloc_data_y.h * alloc_data_y.stride;
        ret = ion_map(fd, handle_y, length1, prot, map_flags, 0, &ptr1, &map_fd1);
        if (ret)
                return;

        length2 = alloc_data_uv.h * alloc_data_uv.stride;
        ret = ion_map(fd, handle_uv, length2, prot, map_flags, 0, &ptr2, &map_fd2);
        if (ret)
                return;

        munmap(ptr1, length1);
        close(map_fd1);
        munmap(ptr2, length2);
	close(map_fd2);

exit:
	while (ix--) 
	{
            ret = ion_free(fd, mem[ix].bufPtr1);
	
            if (ret) {
                printf("%s failed: %s %p\n", __func__, strerror(ret), mem[ix].bufPtr1);
                return;
            }

            ret = ion_free(fd, mem[ix].bufPtr2);
 	
            if (ret) {
                printf("%s failed: %s %p\n", __func__, strerror(ret), mem[ix].bufPtr2);
                return;
            }
	printf("handle de-allocated count = %d handle_y =%x handle_uv\n", ix, mem[ix].bufPtr1, mem[ix].bufPtr2);
	}

        ion_close(fd);
        //close(map_fd);
}


void _ion_tiler_map_test(unsigned char *ptr, struct omap_ion_tiler_alloc_data *al_data)
{
	size_t row, col;
	uint32_t width, height;

	width = al_data->w;
        height = al_data->h;

	for (row = 0; row < height; row++)
		for (col = 0; col < width; col++) {
			int i = (row * stride) + col;
			ptr[i] = (unsigned char)i;
		}
	for (row = 0; row < height; row++)
		for (col = 0; col < width; col++) {
			int i = (row * stride) + col;
			if (ptr[i] != (unsigned char)i)
				printf("%s failed wrote %d read %d from mapped "
					   "memory\n", __func__, i, ptr[i]);
		}
}

int map_1D_test(unsigned int length, size_t stride)
{
	length = (length + PAGE_SIZE - 1) &~ (PAGE_SIZE - 1);
	printf("Mapping and UnMapping 0x%xb 1D buffer\n", length);
#if 0
#ifdef __MAP_OK__
    /* allocate aligned buffer */
    void *buffer = malloc(length + PAGE_SIZE - 1);
    void *dataPtr = (void *)(((uint32_t)buffer + PAGE_SIZE - 1) &~ (PAGE_SIZE - 1));
    uint16_t val = (uint16_t) rand();
    void *ptr = map_1D(dataPtr, length, stride, val);
    if (!ptr) return 1;
    int res = unmap_1D(dataPtr, length, stride, val, ptr);
    FREE(buffer);
#else
    int res = TESTERR_NOTIMPLEMENTED;
#endif
    return res;
}

#endif

//void ion_map_test()
//{
	struct omap_ion_tiler_alloc_data alloc_data = {
                .w = length,
                .h = 1,
                .fmt = TILER_PIXEL_FMT_PAGE,
        };
	int fd, map_fd, ret;
	size_t i;
	struct ion_handle *handle;
	uint16_t *ptr;
 
        fd = ion_open();
        if (fd < 0)
                return fd;


	if(_ion_alloc_test(fd, &handle, &alloc_data))
		return;

	if (tiler_test)
		length = alloc_data.h * alloc_data.stride;
	ret = ion_map(fd, handle, length, prot, map_flags, 0, &ptr, &map_fd);
	if (ret)
		return;

	if (tiler_test)
	
	/* clean up properly */
	munmap(ptr,length);
	ret = ion_free(fd, handle);
	ion_close(fd);
	close(map_fd);

//	_ion_alloc_test(fd, &handle, &alloc_data);
//	close(fd);

#if 0
	munmap(ptr, len);
	close(map_fd);
	ion_close(fd);

	_ion_alloc_test(len, align, flags, &fd, &handle);
	close(map_fd);
	ret = ion_map(fd, handle, len, prot, flags, 0, &ptr, &map_fd);
	/* don't clean up */
#endif
}

static int def_bpp(int fmt)
{
    return (fmt == TILER_PIXEL_FMT_32BIT ? 4 :
            fmt == TILER_PIXEL_FMT_16BIT ? 2 : 1);
}

void fill_mem(uint16_t start, uint16_t *ptr, struct omap_ion_tiler_alloc_data *al_data)
{
    //uint16_t *ptr = (uint16_t *)ptr,
    uint16_t delta = 1, step = 1;
    uint32_t height, width, stride, i;
    if (al_data->fmt == TILER_PIXEL_FMT_PAGE)
    {
        height = 1;
        stride = width = al_data->stride;
    }
    else
    {
        height = al_data->h;
        width = al_data->w;
        stride = al_data->stride;
    }
    width *= def_bpp(al_data->fmt);
    uint32_t size = height * stride;

    printf("%p,%d*%d,s=%d stval=0x%x", al_data->handle, width, height, stride, start);

   // CHK_I(width,<=,stride);
    uint32_t *ptr32 = (uint32_t *)ptr;
    while (height--)
    {
        if (al_data->fmt == TILER_PIXEL_FMT_32BIT)
        {
            for (i = 0; i < width; i += sizeof(uint32_t))
            {
                uint32_t val = (start & 0xFFFF) | (((uint32_t)(start + delta) & 0xFFFF) << 16);
                *ptr32++ = val;
                start += delta;
                delta += step;
                /* increase step if overflown */
                if (delta < step) delta = ++step;
                start += delta;
                delta += step;
                /* increase step if overflown */
                if (delta < step) delta = ++step;
            }
#ifdef __WRITE_IN_STRIDE__
            while (i < stride && (height || ((PAGE_SIZE - 1) & (uint32_t)ptr32)))
            {
                *ptr32++ = 0;
                i += sizeof(uint32_t);
           }
#else
            ptr32 += (stride - i) / sizeof(uint32_t);
#endif
        }
        else
        {
            for (i = 0; i < width; i += sizeof(uint16_t))
            {
                *ptr++ = start;
                start += delta;
                delta += step;
//		printf("++++++value of ptr=%x start=%d delta=%d\n", ptr, *ptr, delta);
                /* increase step if overflown */
                if (delta < step) delta = ++step;
            }
#ifdef __WRITE_IN_STRIDE__
            while (i < stride && (height || ((PAGE_SIZE - 1) & (uint32_t)ptr)))
            {
                *ptr++ = 0;
                i += sizeof(uint16_t);
            }
#else
            ptr += (stride - i) / sizeof(uint16_t);
#endif

        }
    }
   // CHK_P((block->pixelFormat == PIXEL_FMT_32BIT ? (void *)ptr32 : (void *)ptr),==,
     //     (block->ptr + size));
  //  OUT;
}

int check_mem(uint16_t start, uint16_t *ptr, struct omap_ion_tiler_alloc_data *al_data)
{
   //uint8_t *ptr8 = (uint8_t *)ptr; 
    uint16_t delta = 1, step = 1;
    uint32_t height, width, stride, r, i;
    if (al_data->fmt == TILER_PIXEL_FMT_PAGE)
    {
        height = 1;
        stride = width = al_data->stride;
    }
    else
    {
        height = al_data->h;
        width = al_data->w;
        stride = al_data->stride;
    }
    width *= def_bpp(al_data->fmt);

   // CHK_I(width,<=,stride);
    uint32_t *ptr32 = (uint32_t *)ptr;
    for (r = 0; r < height; r++)
    {
        if (al_data->fmt == TILER_PIXEL_FMT_32BIT)
        {
            for (i = 0; i < width; i += sizeof(uint32_t))
            {
                uint32_t val = (start & 0xFFFF) | (((uint32_t)(start + delta) & 0xFFFF) << 16);
                if (*ptr32++ != val) {
                    printf("assert: val[%u,%u] (=0x%x) != 0x%x", r, i, *--ptr32, val);
                    return -EINVAL;
                }
                start += delta;
                delta += step;
                /* increase step if overflown */
                if (delta < step) delta = ++step;
                start += delta;
                delta += step;
                /* increase step if overflown */
                if (delta < step) delta = ++step;
            }
#ifdef __WRITE_IN_STRIDE__
            while (i < stride && ((r < height - 1) || ((PAGE_SIZE - 1) & (uint32_t)ptr32)))
            {
                if (*ptr32++) {
                    printf("assert: val[%u,%u] (=0x%x) != 0", r, i, *--ptr32);
              return -EINVAL;
                }
                i += sizeof(uint32_t);
            }
#else
            ptr32 += (stride - i) / sizeof(uint32_t);
#endif
        }
        else
        {
            for (i = 0; i < width; i += sizeof(uint16_t))
            {
                if (*ptr++ != start) {
                    printf("assert: val[%u,%u] (=0x%x) != 0x%x", r, i, *--ptr, start);
                    return -EINVAL;
                }
                start += delta;
                delta += step;
                /* increase step if overflown */
                if (delta < step) delta = ++step;
            }
#ifdef __WRITE_IN_STRIDE__
            while (i < stride && ((r < height - 1) || ((PAGE_SIZE - 1) & (uint32_t)ptr)))
            {
                if (*ptr++) {
                    printf("assert: val[%u,%u] (=0x%x) != 0", r, i, *--ptr);
                    return -EINVAL;
                }
                i += sizeof(uint16_t);
            }
#else
            ptr += (stride - i) / sizeof(uint16_t);
#endif
        }
    }
    return -EINVAL;
}

int negative_fmt_test(int fmt)
{
        int fd, map_fd, ret, length;
        struct ion_handle *handle;
        uint16_t *ptr;

        struct omap_ion_tiler_alloc_data alloc_data = {
                .w = length,
                .h = 1,
                .fmt = fmt,
        };

        uint16_t val = (uint16_t) rand();
        fd = ion_open();
        if (fd < 0)
                return fd;

        ret = _ion_alloc_test(fd, &handle, &alloc_data);
		if (ret == -EINVAL) 
		 	goto exit;

        if (tiler_test)
              length = alloc_data.h * alloc_data.stride;
        ret = ion_map(fd, handle, length, prot, map_flags, 0, &ptr, &map_fd);
        if (ret)
                return;

        if (tiler_test)

                 fill_mem(val, ptr, &alloc_data);
                 check_mem(val, ptr, &alloc_data);

        munmap(ptr, length);
        ret = ion_free(fd, handle);
        if (ret) {
                printf("%s failed: %s %p\n", __func__, strerror(ret), handle);
                return;
        }
exit:
        ion_close(fd);
}


int negative_1dl_test(int length)
{
        int fd, map_fd, ret;
        struct ion_handle *handle;
        uint16_t *ptr;

        struct omap_ion_tiler_alloc_data alloc_data = {
                .w = length,
                .h = 1,
                .fmt = TILER_PIXEL_FMT_PAGE,
        };

        uint16_t val = (uint16_t) rand();
        fd = ion_open();
        if (fd < 0)
                return fd;

        ret = _ion_alloc_test(fd, &handle, &alloc_data);
                if (ret == -EINVAL)
                        goto exit;

        if (tiler_test)
              length = alloc_data.h * alloc_data.stride;
        ret = ion_map(fd, handle, length, prot, map_flags, 0, &ptr, &map_fd);
        if (ret)
                return;

        if (tiler_test)

                 fill_mem(val, ptr, &alloc_data);
                 check_mem(val, ptr, &alloc_data);

        munmap(ptr, length);
        ret = ion_free(fd, handle);
        if (ret) {
                printf("%s failed: %s %p\n", __func__, strerror(ret), handle);
                return;
        }
exit:
        ion_close(fd);
}

int negative_1dh_test(int height)
{
        int fd, map_fd, ret, length;
        struct ion_handle *handle;
        uint16_t *ptr;

        struct omap_ion_tiler_alloc_data alloc_data = {
                .w = 4096,
                .h = height,
                .fmt = TILER_PIXEL_FMT_PAGE,
        };

        uint16_t val = (uint16_t) rand();
        fd = ion_open();
        if (fd < 0)
                return fd;

        ret = _ion_alloc_test(fd, &handle, &alloc_data);
                if (ret == -EINVAL)
                        goto exit;

        if (tiler_test)
              length = alloc_data.h * alloc_data.stride;
        ret = ion_map(fd, handle, length, prot, map_flags, 0, &ptr, &map_fd);
        if (ret)
                return;

        if (tiler_test)

                 fill_mem(val, ptr, &alloc_data);
                 check_mem(val, ptr, &alloc_data);

        munmap(ptr, length);
        ret = ion_free(fd, handle);
        if (ret) {
                printf("%s failed: %s %p\n", __func__, strerror(ret), handle);
                return;
        }
exit:
        ion_close(fd);
}

int negative_2d_test(uint32_t width, uint32_t height, int fmt)
{
        int fd, map_fd, ret;
        uint32_t length;
        uint16_t *ptr;
        struct ion_handle *handle;
        struct omap_ion_tiler_alloc_data alloc_data = {
                .w = height,
                .h = height,
                .fmt = fmt,
        };

        uint16_t val = (uint16_t) rand();

        fd = ion_open();
        if (fd < 0)
                return fd;


        ret = _ion_alloc_test(fd, &handle, &alloc_data);
		if (ret == -EINVAL)
			goto exit; 

        if (tiler_test)
              length = height * alloc_data.stride;
        ret = ion_map(fd, handle, length, prot, map_flags, 0, &ptr, &map_fd);
        if (ret)
                return;

        if (tiler_test)
                fill_mem(val, ptr, &alloc_data);
                check_mem(val, ptr, &alloc_data);


        munmap(ptr, length);

        ret = ion_free(fd, handle);
        if (ret) {
                printf("%s failed: %s %p\n", __func__, strerror(ret), handle);
                return;
        }
exit:
        ion_close(fd);
}

int negative_free_2D_test(uint32_t width, uint32_t height, int fmt)
{
        int fd, map_fd, ret;
        uint32_t length;
        uint16_t *ptr;
        struct ion_handle *handle;
        struct omap_ion_tiler_alloc_data alloc_data = {
                .w = height,
                .h = height,
                .fmt = 0,
        };

        uint16_t val = (uint16_t) rand();

        fd = ion_open();
        if (fd < 0)
                return fd;


        if (_ion_alloc_test(fd, &handle, &alloc_data))
                        return;

        if (tiler_test)
              length = height * alloc_data.stride;
        ret = ion_map(fd, handle, length, prot, map_flags, 0, &ptr, &map_fd);
        if (ret)
                return;

        if (tiler_test)
                fill_mem(val, ptr, &alloc_data);
                check_mem(val, ptr, &alloc_data);


        munmap(ptr, length);

        ret = ion_free(fd, handle);
	printf("return value from ion_free = %d\n",ret);
	ret = ion_free(fd, handle);
        if (ret == -EINVAL)
		goto exit;//printf("return value from ion_free = %d\n",ret);
        if (ret) {
                printf("%s failed: %s %p\n", __func__, strerror(ret), handle);
                return;
        }
exit:
        ion_close(fd);
        close(map_fd);
}


int negative_free_1D_test(uint32_t length, size_t stride)
{
        int fd, map_fd, ret;
        struct ion_handle *handle;
        uint16_t *ptr;

        struct omap_ion_tiler_alloc_data alloc_data = {
                .w = length,
                .h = 1,
                .fmt = TILER_PIXEL_FMT_PAGE,
        };

        uint16_t val = (uint16_t) rand();
        fd = ion_open();
        if (fd < 0)
                return fd;

        if (_ion_alloc_test(fd, &handle, &alloc_data))
                        return;

        if (tiler_test)
              length = alloc_data.h * alloc_data.stride;
        ret = ion_map(fd, handle, length, prot, map_flags, 0, &ptr, &map_fd);
        if (ret)
                return;

        if (tiler_test)

                 fill_mem(val, ptr, &alloc_data);
                 check_mem(val, ptr, &alloc_data);

        munmap(ptr, length);
        ret = ion_free(fd, handle);
	ret = ion_free(fd, handle);
	if (ret == -EINVAL)
		goto exit;

        if (ret) {
                printf("%s failed: %s %p\n", __func__, strerror(ret), handle);
                return;
        }
exit:
        ion_close(fd);
}

int negative_arbitvalue_test(uint32_t length, size_t stride)
{
        int fd, map_fd, ret;
        struct ion_handle *handle;
        uint16_t *ptr;

        struct omap_ion_tiler_alloc_data alloc_data = {
                .w = length,
                .h = 1,
                .fmt = TILER_PIXEL_FMT_PAGE,
        };

        uint16_t val = (uint16_t) rand();
        fd = ion_open();
        if (fd < 0)
                return fd;

        if (_ion_alloc_test(fd, &handle, &alloc_data))
                        return;

        if (tiler_test)
              length = alloc_data.h * alloc_data.stride;
        ret = ion_map(fd, handle, length, prot, map_flags, 0, &ptr, &map_fd);
        if (ret)
                return;

        if (tiler_test)

                 fill_mem(val, ptr, &alloc_data);
                 check_mem(val, ptr, &alloc_data);

        munmap(ptr, length);
        ret = ion_free(fd, 0x12345678);
        if (ret == -EINVAL)
		goto exit;
        if (ret) {
                printf("%s failed: %s %p\n", __func__, strerror(ret), handle);
                return;
        }
exit:
        ion_close(fd);
}

int random_alloc_test(uint32_t num_ops, uint16_t num_slots)
{
    printf("Random set of %d Allocs/Maps and Frees/UnMaps for %d slots\n", num_ops, num_slots);
    srand(0x4B72316A);
    struct data {
        int      op;
       uint32_t width, height;
        uint32_t  length;
        void    *bufPtr;
        void    *bufPtr1;
        void    *dataPtr;
	uint16_t stride;
	int fmt;
	int fd1;
    } *mem;

   struct ion_handle *handle, *handle_uv;
   int fd, ret, map_fd;
   uint16_t *ptr, *ptr1, *ptr2;
   uint32_t length, length1, length2;
   uint16_t val;
  
   struct omap_ion_tiler_alloc_data alloc_data;

     fd = ion_open();
        if (fd < 0)
                return fd;

    /* allocate memory state */
    mem = calloc(num_slots, sizeof(struct data));
    if (!mem) return -EFAULT;

    /* perform alloc/free/unmaps */
    int res = 0, ix;
    while (num_ops--)
    {
        ix = rand() % num_slots;
        /* see if we need to free/unmap data */
        if (mem[ix].bufPtr) 
        {
            /* check memory fill */
            switch (mem[ix].op)
            {
            //case 0: //res = ion_free(fd, mem[ix].bufPtr);
                //free(mem[ix].buffer);
           //     break;
            case 1: res = ion_free(fd, mem[ix].bufPtr); 
                    printf("memory-fill check: 1 freing return val =%d\n",res);
		    break;
            case 2: res = ion_free(fd, mem[ix].bufPtr);
		    printf("memory-fill check: 2 freing return val =%d\n",res); 
		    break;
            case 3: res = ion_free(fd, mem[ix].bufPtr);
		    printf("memory-fill check: 3 freing return val =%d\n",res);
		    break;
            case 4: res = ion_free(fd, mem[ix].bufPtr);
                    printf("memory-fill check: 4 freing return val =%d\n",res);
		    break;
           case 5: res = ion_free(fd, mem[ix].bufPtr);
		   printf("memory-fill check:  5-y freing return val =%d\n",res);
		   res = ion_free(fd, mem[ix].bufPtr1);	
		   printf("memory-fill check:  5-uv freing return val =%d\n",res);
	 	   break;
            }
          //  P("%s[%p]", mem[ix].op ? "free" : "unmap", mem[ix].bufPtr);
          /*  ZERO(mem[ix]);*/  memset(&(mem[ix]), 0, sizeof(mem[ix]));
        }
        /* we need to allocate/map data */
        else
        {
            int op = rand();
            /* set width */
        //    uint32_t width, height;
            switch ("AAAABBBBCCCDDEEF"[op & 15]) {
            case 'F': alloc_data.w = 1920; alloc_data.h = 1080; break;
            case 'E': alloc_data.w = 1280; alloc_data.h = 720; break;
            case 'D': alloc_data.w = 640; alloc_data.h = 480; break;
            case 'C': alloc_data.w = 648; alloc_data.h = 480; break;
            case 'B': alloc_data.w = 176; alloc_data.h = 144; break;
            case 'A': alloc_data.w = 64; alloc_data.h = 64; break;
            }
        
	  //  mem[ix].length = (uint32_t)width * height;
          //    mem[ix].width = alloc_data.w;
          //   mem[ix].height = alloc_data.h;
            val = ((uint16_t)rand());

            /* perform operation */
            mem[ix].op = "BBBBCCCCDDDDEEEFF"[(op >> 4) & 15] - 'A';
            switch (mem[ix].op)
            {
            case 0: /* map 1D buffer */
                mem[ix].op = 1;
            case 1:
		alloc_data.fmt = TILER_PIXEL_FMT_PAGE;
		alloc_data.h = 1;
	        ret = _ion_alloc_test(fd, &handle, &alloc_data);
		if (ret == -EINVAL) 
			goto exit;
                mem[ix].bufPtr = handle;
		mem[ix].stride = alloc_data.stride;
		mem[ix].height = alloc_data.h;
                mem[ix].width = alloc_data.w;
		length = mem[ix].height * mem[ix].stride;
		ret = ion_map(fd, mem[ix].bufPtr, length, prot, map_flags, 0, &ptr, &map_fd);
                                if (ret)
                                        return;

                                fill_mem(val, ptr, &alloc_data);
                                check_mem(val, ptr, &alloc_data);
                                munmap(ptr, length);
                                ion_close(map_fd);

		printf("value of mem[ix].bufPtr=%x, ix=%x, stride=%d fmt=%d h=%d w=%d \n", mem[ix].bufPtr, ix,mem[ix].stride, alloc_data.fmt, mem[ix].height, mem[ix].width);
        //        P("alloc[l=0x%x] = %p", mem[ix].length, mem[ix].bufPtr);
                break;
            case 2:
		alloc_data.fmt = TILER_PIXEL_FMT_8BIT;
		ret = _ion_alloc_test(fd, &handle, &alloc_data);
                if (ret == -EINVAL)
			goto exit;
		mem[ix].bufPtr = handle;
		mem[ix].stride = alloc_data.stride;
		mem[ix].height = alloc_data.h;
                mem[ix].width = alloc_data.w;
		length = mem[ix].height * mem[ix].stride;
		
		ret = ion_map(fd, mem[ix].bufPtr, length, prot, map_flags, 0, &ptr, &map_fd);
                                if (ret)
                                        return;

                                fill_mem(val, ptr, &alloc_data);
                                check_mem(val, ptr, &alloc_data);
                                munmap(ptr, length);
                                ion_close(map_fd);

		printf("value of mem[ix].bufPtr=%x, ix=%x, stride=%d fmt=%d h=%d w=%d \n", mem[ix].bufPtr, ix, mem[ix].stride, alloc_data.fmt, mem[ix].height, mem[ix].width);
        //        P("alloc[%d*%d*8] = %p", mem[ix].width, mem[ix].height, mem[ix].bufPtr);
                break;
            case 3:
		alloc_data.fmt = TILER_PIXEL_FMT_16BIT;
                ret = _ion_alloc_test(fd, &handle, &alloc_data);
                if (ret == -EINVAL)
			goto exit;
	 	mem[ix].bufPtr = handle;
		mem[ix].stride = alloc_data.stride;
		mem[ix].height = alloc_data.h;
		mem[ix].width = alloc_data.w;
		length = mem[ix].height * mem[ix].stride;
		ret = ion_map(fd, mem[ix].bufPtr, length, prot, map_flags, 0, &ptr, &map_fd);
                                if (ret)
                                        return;

                                fill_mem(val, ptr, &alloc_data);
                                check_mem(val, ptr, &alloc_data);
                                munmap(ptr, length);
                                ion_close(map_fd);

		printf("value of mem[ix].bufPtr=%x, ix=%x, stride=%d fmt=%d h=%d w=%d \n", mem[ix].bufPtr, ix, mem[ix].stride, alloc_data.fmt, mem[ix].height, mem[ix].width);
              //  P("alloc[%d*%d*16] = %p", mem[ix].width, mem[ix].height, mem[ix].bufPtr);
                break;
            case 4:
		alloc_data.fmt = TILER_PIXEL_FMT_32BIT;
                ret = _ion_alloc_test(fd, &handle, &alloc_data);
		if (ret == -EINVAL)
			goto exit;
                mem[ix].bufPtr = handle;
		mem[ix].stride = alloc_data.stride;
		mem[ix].height = alloc_data.h;
                mem[ix].width = alloc_data.w;
		length = mem[ix].height * mem[ix].stride;
		ret = ion_map(fd, mem[ix].bufPtr, length, prot, map_flags, 0, &ptr, &map_fd);
                                if (ret)
                                        return;

                                fill_mem(val, ptr, &alloc_data);
                                check_mem(val, ptr, &alloc_data);
                                munmap(ptr, length);
                                ion_close(map_fd);

          	printf("value of mem[ix].bufPtr=%x, ix=%x, stride=%d fmt=%d h=%d w=%d \n", mem[ix].bufPtr, ix, mem[ix].stride, alloc_data.fmt, mem[ix].height, mem[ix].width);
              //  P("alloc[%d*%d*32] = %p", mem[ix].width, mem[ix].height, mem[ix].bufPtr);
                break;
            case 5:
		printf("+++++++++Inside case 5 for NV12 Buffer++++++++\n");
		alloc_data.fmt = TILER_PIXEL_FMT_8BIT;
		mem[ix].height = alloc_data.h;
                mem[ix].width = alloc_data.w;
		struct omap_ion_tiler_alloc_data alloc_data_uv = {
                .w = mem[ix].width >> 1,
                .h = mem[ix].height >> 1,
                .fmt = TILER_PIXEL_FMT_16BIT,
        	};

        	ret = _ion_alloc_test(fd, &handle, &alloc_data);
              	if (ret == -EINVAL)
			goto exit;
			
        	ret = _ion_alloc_test(fd, &handle_uv, &alloc_data_uv);
                if (ret == -EINVAL)
			goto exit;
		mem[ix].bufPtr = handle;
		mem[ix].bufPtr1 = handle_uv;
		printf("value of NV12 Y bufPtr=%x, NV12 UV bufPtr=%x \n", mem[ix].bufPtr, mem[ix].bufPtr1);

              length1 = alloc_data.h * alloc_data.stride;
        	ret = ion_map(fd, handle, length1, prot, map_flags, 0, &ptr1, &map_fd);
        	if (ret)
                	return;

              length2 = alloc_data_uv.h * alloc_data_uv.stride;
               ret = ion_map(fd, handle_uv, length2, prot, map_flags, 0, &ptr2, &map_fd);
        	if (ret)
                	return;

        if (tiler_test)
                fill_mem(val, ptr1, &alloc_data);
                check_mem(val, ptr1, &alloc_data);

                fill_mem(val, ptr2, &alloc_data_uv);
                check_mem(val, ptr2, &alloc_data_uv);

         	munmap(ptr1, length1);
        	munmap(ptr2, length2);
		ion_close(map_fd);

        //  P("alloc[%d*%d*NV12] = %p", mem[ix].width, mem[ix].height, mem[ix].bufPtr);
                break;
            }




           /* check all previous buffers */
#if 0
            for (ix = 0; ix < num_slots; ix++)
            {
                if (mem[ix].bufPtr)
                {
                   #if 0 
		   if(0) P("ptr=%p, op=%d, w=%d, h=%d, l=%x, val=%x",
                      mem[ix].bufPtr, mem[ix].op, mem[ix].width, mem[ix].height,
                      mem[ix].length, mem[ix].val);
		  #endif
                    switch (mem[ix].op)
                    {
                    case 0: 
		    case 1:
                    case 5:
                    case 2:
		    case 3:
		    case 4:
		//	if (tiler_test)
				printf("++++++++++++++++++value of memixop =%d numslot=%d bufPtr=%x\n", mem[ix].op, ix, mem[ix].bufPtr);
              			length = mem[ix].height * mem[ix].stride;
				alloc_data.h = mem[ix].height;
				alloc_data.w = mem[ix].width;
 				alloc_data.fmt = mem[ix].fmt;
			printf("Inside trace: mem[ix].bufPtr=%x, ix=%x, stride=%d fmt=%d,h=%d w=%d \n", mem[ix].bufPtr, ix, mem[ix].stride, mem[ix].fmt, alloc_data.h, alloc_data.w);
        			ret = ion_map(mem[ix].fd1, mem[ix].bufPtr, length, prot, map_flags, 0, &ptr, &map_fd);
        			if (ret)
                			return;

        	//	if (tiler_test)

                 		fill_mem(val, ptr, &alloc_data);
                 		check_mem(val, ptr, &alloc_data);
				munmap(ptr, length);
				ion_free(mem[ix].fd1, mem[ix].bufPtr);
	//			ion_close(mem[ix].fd1);
				ion_close(map_fd);			
                        break;
                    }
                }
            }
#endif
        }
    }

exit:

    /* unmap and free everything */
    for (ix = 0; ix < num_slots; ix++)
    {
        if (mem[ix].bufPtr)
        {
            /* check memory fill */
            switch (mem[ix].op)
            {
            case 0: printf("hello\n");printf("freed in case 0\n"); break;
            case 1: ion_free(fd, mem[ix].bufPtr);printf("freed in case 1\n"); break;
            case 2: ion_free(fd, mem[ix].bufPtr);printf("freed in case 2\n"); break;
            case 3: ion_free(fd, mem[ix].bufPtr);printf("freed in case 3\n"); break;
            case 4: ion_free(fd, mem[ix].bufPtr);printf("freed in case 4\n"); break;
            case 5: ion_free(fd, mem[ix].bufPtr);ion_free(fd, mem[ix].bufPtr1);
		    printf("freed in case 5\n");
			break;
            }
        }
    }
    free(mem);

	ion_close(fd);

    return res;
}

#if 0
int negative_arbitvalue_test(uint32_t length, size_t stride)
{
        int fd, map_fd, ret;
        struct ion_handle *handle;
        uint16_t *ptr;

        struct omap_ion_tiler_alloc_data alloc_data = {
                .w = length,
                .h = 1,
                .fmt = TILER_PIXEL_FMT_PAGE,
        };

        uint16_t val = (uint16_t) rand();
        fd = ion_open();
        if (fd < 0)
                return fd;

        if (_ion_alloc_test(fd, &handle, &alloc_data))
                        return;

        if (tiler_test)
              length = alloc_data.h * alloc_data.stride;
        ret = ion_map(fd, handle, length, prot, map_flags, 0, &ptr, &map_fd);
        if (ret)
                return;

        if (tiler_test)

                 fill_mem(val, ptr, &alloc_data);
                 check_mem(val, ptr, &alloc_data);

        munmap(ptr, length);
        ret = ion_free(fd, 0x12345678);
        if (ret == -EINVAL)
                goto exit;
        if (ret) {
                printf("%s failed: %s %p\n", __func__, strerror(ret), handle);
                return;
        }
exit:
        ion_close(fd);
        close(map_fd);
}

#endif

#if 0
void ion_share_test()

{
	struct ion_handle *handle;
	int sd[2];
	int num_fd = 1;
	struct iovec count_vec = {
		.iov_base = &num_fd,
		.iov_len = sizeof num_fd,
	};
	char buf[CMSG_SPACE(sizeof(int))];
	socketpair(AF_UNIX, SOCK_STREAM, 0, sd);
	if (fork()) {
		struct msghdr msg = {
			.msg_control = buf,
			.msg_controllen = sizeof buf,
			.msg_iov = &count_vec,
			.msg_iovlen = 1,
		};

		struct cmsghdr *cmsg;
		int fd, share_fd, ret;
		char *ptr;
		/* parent */
		if(_ion_alloc_test(&fd, &handle))
			return;
		ret = ion_share(fd, handle, &share_fd);
		if (ret)
			printf("share failed %s\n", strerror(errno));
		ptr = mmap(NULL, len, prot, map_flags, share_fd, 0);
		if (ptr == MAP_FAILED) {
			return;
		}
		strcpy(ptr, "master");
		cmsg = CMSG_FIRSTHDR(&msg);
		cmsg->cmsg_level = SOL_SOCKET;
		cmsg->cmsg_type = SCM_RIGHTS;
		cmsg->cmsg_len = CMSG_LEN(sizeof(int));
		*(int *)CMSG_DATA(cmsg) = share_fd;
		/* send the fd */
		printf("master? [%10s] should be [master]\n", ptr);
		printf("master sending msg 1\n");
		sendmsg(sd[0], &msg, 0);
		if (recvmsg(sd[0], &msg, 0) < 0)
			perror("master recv msg 2");
		printf("master? [%10s] should be [child]\n", ptr);

		/* send ping */
		sendmsg(sd[0], &msg, 0);
		printf("master->master? [%10s]\n", ptr);
		if (recvmsg(sd[0], &msg, 0) < 0)
			perror("master recv 1");
	} else {
		struct msghdr msg;
		struct cmsghdr *cmsg;
		char* ptr;
		int fd, recv_fd;
		char* child_buf[100];
		/* child */
		struct iovec count_vec = {
			.iov_base = child_buf,
			.iov_len = sizeof child_buf,
		};

		struct msghdr child_msg = {
			.msg_control = buf,
			.msg_controllen = sizeof buf,
			.msg_iov = &count_vec,
			.msg_iovlen = 1,
		};

		if (recvmsg(sd[1], &child_msg, 0) < 0)
			perror("child recv msg 1");
		cmsg = CMSG_FIRSTHDR(&child_msg);
		if (cmsg == NULL) {
			printf("no cmsg rcvd in child");
			return;
		}
		recv_fd = *(int*)CMSG_DATA(cmsg);
		if (recv_fd < 0) {
			printf("could not get recv_fd from socket");
			return;
		}
		printf("child %d\n", recv_fd);
		fd = ion_open();
		ptr = mmap(NULL, len, prot, map_flags, recv_fd, 0);
		if (ptr == MAP_FAILED) {
			return;
		}
		printf("child? [%10s] should be [master]\n", ptr);
		strcpy(ptr, "child");
		printf("child sending msg 2\n");
		sendmsg(sd[1], &child_msg, 0);
	}
}

int main(int argc, char* argv[]) {
	int c;
	enum tests {
		ALLOC_TEST = 0, MAP_TEST, SHARE_TEST,
	};

	while (1) {
		static struct option opts[] = {
			{"alloc", no_argument, 0, 'a'},
			{"alloc_flags", required_argument, 0, 'f'},
			{"map", no_argument, 0, 'm'},
			{"share", no_argument, 0, 's'},
			{"len", required_argument, 0, 'l'},
			{"align", required_argument, 0, 'g'},
			{"map_flags", required_argument, 0, 'z'},
			{"prot", required_argument, 0, 'p'},
			{"alloc_tiler", no_argument, 0, 't'},
			{"width", required_argument, 0, 'w'},
			{"height", required_argument, 0, 'h'},
			{"fmt", required_argument, 0, 'r'},
		};
		int i = 0;
		c = getopt_long(argc, argv, "af:h:l:mr:stw:", opts, &i);
		if (c == -1)
			break;

		switch (c) {
		case 'l':
			len = atol(optarg);
			break;
		case 'g':
			align = atol(optarg);
			break;
		case 'z':
			map_flags = 0;
			map_flags |= strstr(optarg, "PROT_EXEC") ?
				PROT_EXEC : 0;
			map_flags |= strstr(optarg, "PROT_READ") ?
				PROT_READ: 0;
			map_flags |= strstr(optarg, "PROT_WRITE") ?
				PROT_WRITE: 0;
			map_flags |= strstr(optarg, "PROT_NONE") ?
				PROT_NONE: 0;
			break;
		case 'p':
			prot = 0;
			prot |= strstr(optarg, "MAP_PRIVATE") ?
				MAP_PRIVATE	 : 0;
			prot |= strstr(optarg, "MAP_SHARED") ?
				MAP_PRIVATE	 : 0;
			break;
		case 'f':
			alloc_flags = atol(optarg);
			break;
		case 'a':
			test = ALLOC_TEST;
			break;
		case 'm':
			test = MAP_TEST;
			break;
		case 'r':
			fmt = atol(optarg);
			break;
		case 's':
			test = SHARE_TEST;
			break;
		case 'w':
			width = atol(optarg);
			break;
		case 'h':
			height = atol(optarg);
			break;
		case 't':
			tiler_test = 1;
			break;
		}
	}
	printf("test %d, len %u, width %u, height %u fmt %u align %u, "
		   "map_flags %d, prot %d, alloc_flags %d\n", test, len, width,
		   height, fmt, align, map_flags, prot, alloc_flags);
	switch (test) {
		case ALLOC_TEST:
			ion_alloc_test();
			break;
		case MAP_TEST:
			ion_map_test();
			break;
		case SHARE_TEST:
			ion_share_test();
			break;
		default:
			printf("must specify a test (alloc, map, share)\n");
	}
	return 0;
}

#endif

DEFINE_TESTS(TESTS)

/**
 * We run the same identity check before and after running the
 * tests.
 *
 * @author a0194118 (9/12/2009)
 */
void memmgr_identity_test(void *ptr)
{
    /* also execute internal unit tests - this also verifies that we did not
       keep any references */
    //__test__MemMgr();
}

/**
 * Main test function. Checks arguments for test case ranges,
 * runs tests and prints usage or test list if required.
 *
 * @author a0194118 (9/7/2009)
 *
 * @param argc   Number of arguments
 * @param argv   Arguments
 *
 * @return -1 on usage or test list, otherwise # of failed
 *         tests.
 */
int main(int argc, char **argv)
{
    return TestLib_Run(argc, argv,
                       memmgr_identity_test, memmgr_identity_test, NULL);
}

