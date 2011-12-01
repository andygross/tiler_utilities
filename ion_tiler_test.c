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
 /*   T(alloc_1D_test(4096, 0))\
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
   */ T(maxalloc_1D_test(4096, MAX_ALLOCS))\
  /*  T(maxalloc_1D_test(176 * 144 * 2, MAX_ALLOCS))\
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
 */ T(maxalloc_2D_test(1280, 720, TILER_PIXEL_FMT_8BIT, MAX_ALLOCS))\
   T(maxalloc_2D_test(1280, 720, TILER_PIXEL_FMT_16BIT, MAX_ALLOCS))\
   T(maxalloc_2D_test(1280, 720, TILER_PIXEL_FMT_32BIT, MAX_ALLOCS))\
   T(maxalloc_NV12_test(1280, 720, MAX_ALLOCS))\
/*   T(maxalloc_2D_test(1920, 1080, TILER_PIXEL_FMT_8BIT, MAX_ALLOCS))\
   T(maxalloc_2D_test(1920, 1080, TILER_PIXEL_FMT_16BIT, MAX_ALLOCS))\
   T(maxalloc_2D_test(1920, 1080, TILER_PIXEL_FMT_32BIT, MAX_ALLOCS))\
   T(maxalloc_NV12_test(1920, 1080, 2))\
*/   T(maxalloc_NV12_test(1920, 1080, MAX_ALLOCS))\
/*   T(negative_fmt_test(-1)) \
   T(negative_fmt_test(8)) \
   T(negative_2d_test(0, 1080, TILER_PIXEL_FMT_16BIT)) \
   T(negative_2d_test(1920, 0, TILER_PIXEL_FMT_16BIT)) \
   T(negative_1dl_test(0)) \
   T(negative_1dh_test(3)) \
   T(negative_free_2D_test(1920, 1080, TILER_PIXEL_FMT_16BIT)) \
   T(negative_free_1D_test(176 * 144 * 2, 0)) \
   T(negative_arbitvalue_test(176 * 144 * 2, 0)) \
*/
//typedef enum pixel_fmt_t pixel_fmt_t;
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

#if 0
void ion_alloc_test()
{
	int fd, ret;
	struct ion_handle *handle;

	if(_ion_alloc_test(&fd, &handle))
			return;

	ret = ion_free(fd, handle);
	if (ret) {
		printf("%s failed: %s %p\n", __func__, strerror(ret), handle);
		return;
	}
	ion_close(fd);
	printf("ion alloc test: passed\n");
}
#endif

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

	if (_ion_alloc_test(fd, &handle, &alloc_data))
			return;

	if (tiler_test)
              length = alloc_data.h * alloc_data.stride;
        ret = ion_map(fd, handle, length, prot, map_flags, 0, &ptr, &map_fd);
        if (ret)
                return;

        if (tiler_test)
                _ion_tiler_map_test(ptr, &alloc_data);

		 fill_mem(val, ptr, &alloc_data);
		 check_mem(val, ptr, &alloc_data);

	munmap(ptr, length);
	ret = ion_free(fd, handle);
	if (ret) {
		printf("%s failed: %s %p\n", __func__, strerror(ret), handle);
		return;
	}
	ion_close(fd);
	close(map_fd);
}

int maxalloc_1D_test(uint32_t length, int max_allocs)
{
    printf("Allocate & Free max # of %ub 1D buffers\n", length);

    int fd, ret;
    struct ion_handle *handle;
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


    /* allocate as many buffers as we can */
    mem = (struct data *)calloc(max_allocs, sizeof(struct data));
    void *ptr = (void *)mem;
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
 //               _ion_tiler_map_test(ptr, &alloc_data);
		fill_mem(val, ptr, &alloc_data);
                check_mem(val, ptr, &alloc_data);


        munmap(ptr, length);

        ret = ion_free(fd, handle);
        if (ret) {
                printf("%s failed: %s %p\n", __func__, strerror(ret), handle);
                return;
        }
        ion_close(fd);
	close(map_fd);
}

int maxalloc_2D_test(uint32_t width, uint32_t height, int fmt, int max_allocs)
{
   // printf("Allocate & Free max # of %ub 1D buffers\n", length);

    int fd, ret;
    struct ion_handle *handle;
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


    /* allocate as many buffers as we can */
    mem = (struct data *)calloc(max_allocs, sizeof(struct data));
    void *ptr = (void *)mem;
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
        int fd, map_fd, ret;
        uint32_t length;
        unsigned char *ptr1, *ptr2;
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


        if (_ion_alloc_test(fd, &handle_y, &alloc_data_y))
                        return;

	if (_ion_alloc_test(fd, &handle_uv, &alloc_data_uv))
                        return;
/*
        if (tiler_test)
              length = alloc_data_y.h * alloc_data_y.stride;
        ret = ion_map(fd, handle_y, length, prot, map_flags, 0, &ptr1, &map_fd);
        if (ret)
                return;

	length = alloc_data_uv.h * alloc_data_uv.stride;
	ret = ion_map(fd, handle_uv, length, prot, map_flags, 0, &ptr2, &map_fd);
	if (ret)
		return;

        if (tiler_test)
                _ion_tiler_map_test(ptr1, &alloc_data_y);
		_ion_tiler_map_test(ptr2, &alloc_data_uv);

        munmap(ptr1, length);
	munmap(ptr2, length);
*/
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
        ion_close(fd);
      //  close(map_fd);
}

int maxalloc_NV12_test(uint32_t width, uint32_t height, uint32_t max_allocs)
{
        int fd, map_fd, ret;
        uint32_t length;
        unsigned char *ptr1, *ptr2;
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

    /* allocate as many buffers as we can */
    mem = (struct data *)calloc(max_allocs, sizeof(struct data));
    void *ptr = (void *)mem;
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
     /*
        if (tiler_test)
              length = alloc_data_y.h * alloc_data_y.stride;
        ret = ion_map(fd, handle_y, length, prot, map_flags, 0, &ptr1, &map_fd);
        if (ret)
                return;

        length = alloc_data_uv.h * alloc_data_uv.stride;
        ret = ion_map(fd, handle_uv, length, prot, map_flags, 0, &ptr2, &map_fd);
        if (ret)
                return;

        if (tiler_test)
                _ion_tiler_map_test(ptr1, &alloc_data_y);
                _ion_tiler_map_test(ptr2, &alloc_data_uv);

      munmap(ptr1, length);
        munmap(ptr2, length);
*/
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
	unsigned char *ptr;
 
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
		_ion_tiler_map_test(ptr, &alloc_data);
	
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

    printf("%p,0x%x*0x%x,s=0x%x=0x%x", al_data->handle, width, height, stride, start);

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
                _ion_tiler_map_test(ptr, &alloc_data);

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
     //   close(map_fd);
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
                _ion_tiler_map_test(ptr, &alloc_data);

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
     //   close(map_fd);
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
                _ion_tiler_map_test(ptr, &alloc_data);

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
     //   close(map_fd);
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
 //               _ion_tiler_map_test(ptr, &alloc_data);
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
     //   close(map_fd);
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
 //               _ion_tiler_map_test(ptr, &alloc_data);
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
                _ion_tiler_map_test(ptr, &alloc_data);

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
        close(map_fd);
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
                _ion_tiler_map_test(ptr, &alloc_data);

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
                _ion_tiler_map_test(ptr, &alloc_data);

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

