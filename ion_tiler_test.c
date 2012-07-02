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
size_t stride;

static int verbose;

#define PAGE_SIZE 0x1000
#define MAX_ALLOCS 512

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
    T(alloc_2D_test(1080, 1920, TILER_PIXEL_FMT_32BIT))\
    T(alloc_2D_test(8193, 16, TILER_PIXEL_FMT_8BIT))\
    T(alloc_2D_test(8193, 16, TILER_PIXEL_FMT_16BIT))\
    T(alloc_2D_test(4097, 16, TILER_PIXEL_FMT_32BIT))\
    T(alloc_2D_test(16384, 16, TILER_PIXEL_FMT_8BIT))\
    T(alloc_2D_test(16384, 16, TILER_PIXEL_FMT_16BIT))\
    T(alloc_2D_test(8192, 16, TILER_PIXEL_FMT_32BIT))\
    T(!alloc_2D_test(16385, 16, -1))\
    T(!alloc_2D_test(16385, 16, 8))\
    T(!alloc_2D_test(16385, 16, TILER_PIXEL_FMT_8BIT))\
    T(!alloc_2D_test(16385, 16, TILER_PIXEL_FMT_16BIT))\
    T(!alloc_2D_test(8193, 16, TILER_PIXEL_FMT_32BIT))\
    T(!alloc_2D_test(0, 16, TILER_PIXEL_FMT_32BIT))\
    T(!alloc_2D_test(1920, 0, TILER_PIXEL_FMT_32BIT))\
    T(!alloc_2D_test(1920, 0xffffffff, TILER_PIXEL_FMT_32BIT))\
    T(!alloc_2D_test(0xffffffff, 16, TILER_PIXEL_FMT_32BIT))\
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
   T(negative_1dl_test(0)) \
   T(negative_1dh_test(3)) \
   T(negative_free_2D_test(1920, 1080, TILER_PIXEL_FMT_16BIT)) \
   T(negative_free_1D_test(176 * 144 * 2, 0)) \
   T(negative_arbitvalue_test(176 * 144 * 2, 0)) \
   T(random_alloc_test(1000,10))
//   T(maxalloc_test(1920, 1080, TILER_PIXEL_FMT_16BIT, MAX_ALLOCS))

int check_mem(uint16_t start, uint16_t *ptr, 
	      struct omap_ion_tiler_alloc_data *al_data);
void fill_mem(uint16_t start, uint16_t *ptr, 
	      struct omap_ion_tiler_alloc_data *al_data);

int _ion_alloc_test(int fd, struct ion_handle **handle,
		    struct omap_ion_tiler_alloc_data *al_data)
{
	int ret;

	ret = ion_alloc_tiler(fd, &al_data, handle, &stride);

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
	if (ret)
		goto exit;

	length = alloc_data.h * alloc_data.stride;

	if (verbose)
	        printf("mapping %d \n", length);
	ret = ion_map(fd, handle, length, prot, map_flags, 0, &ptr, &map_fd);
        if (ret)
		return ret;

	fill_mem(val, ptr, &alloc_data);
	ret  = check_mem(val, ptr, &alloc_data);
	if (ret) {
		printf("%s: memory validation failed\n", __func__);
		munmap(ptr, length);
		goto exit;
	}

	munmap(ptr, length);
	ret = ion_free(fd, handle);
	if (ret) {
		printf("%s failed: %s %p\n", __func__, strerror(ret), handle);
		return ret;
	}
	close(map_fd);
exit:
	ion_close(fd);
	return ret;
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
		.out_align = PAGE_SIZE,
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
	int ix, res = 0;

	for (ix = 0;  ix < max_allocs;)
	{
        	ret = _ion_alloc_test(fd, &handle, &alloc_data);
		if (ret)
			goto exit;

		if (handle) {
			mem[ix].bufPtr = handle;
			if (verbose)
			    printf("handle allocated count = %d handle =%p\n",
					ix, mem[ix].bufPtr);
			length = alloc_data.h * alloc_data.stride;
        		ret = ion_map(fd, handle, length, prot, map_flags, 0, 
				&ptr, &map_fd);
        		if (ret)
                		return ret;

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
		if (verbose)
		     printf("handle de-allocated count = %d handle =%p\n", ix, 
				mem[ix].bufPtr);
		ret = ion_free(fd,  mem[ix].bufPtr);
		if (ret) {
                	printf("%s failed: %s %p\n", __func__, strerror(ret),
				 mem[ix].bufPtr);
                	return ret;
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
		.fmt = fmt,
		.out_align = PAGE_SIZE,
	};

	uint16_t val = (uint16_t) rand();
	fd = ion_open();
	if (fd < 0)
		return fd;

	ret = _ion_alloc_test(fd, &handle, &alloc_data);
	if (ret)
		goto exit;

	length = height * alloc_data.stride;
	ret = ion_map(fd, handle, length, prot, map_flags, 0, &ptr, &map_fd);
	if (ret)
                goto exit;

	fill_mem(val, ptr, &alloc_data);
	ret = check_mem(val, ptr, &alloc_data);
	if (ret) {
		printf("%s: memory validation failed\n", __func__);
		munmap(ptr, length);
		goto exit;

	}

	munmap(ptr, length);
        ret = ion_free(fd, handle);
	if (ret) {
		printf("%s failed: %s %p\n", __func__, strerror(ret), handle);
		return ret;
	}
	close(map_fd);
exit:
        ion_close(fd);
	return ret;
}

int maxalloc_2D_test(uint32_t width, uint32_t height, int fmt, int max_allocs)
{
	int fd, ret, map_fd;
	struct ion_handle *handle;
	uint16_t *ptr;
	uint32_t length;

	struct omap_ion_tiler_alloc_data alloc_data = {
		.w = width,
		.h = height,
		.fmt = fmt,
		.out_align = PAGE_SIZE,
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
	int ix, res = 0;

	for (ix = 0;  ix < max_allocs;)
    	{
        	ret = _ion_alloc_test(fd, &handle, &alloc_data);
        	if (ret)
                	goto exit;

		if (handle) {
            		mem[ix].bufPtr = handle;
			if (verbose)
	    		    printf("handle allocated count = %d handle =%p\n",
					ix, mem[ix].bufPtr);
			length = height * alloc_data.stride;
           		ret = ion_map(fd, handle, length, prot, map_flags, 0, 
					&ptr, &map_fd);

        		if (ret)
                		return ret;

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
		if (verbose)
		    printf("handle de-allocated count = %d handle =%p\n", ix, 
				mem[ix].bufPtr);
		if (ret) {
			printf("%s failed: %s %p\n", __func__, strerror(ret), 
				handle);
			return ret;
		}
	}
	free(mem);
	ion_close(fd);
	return res;
}

int maxalloc_test(uint32_t width, uint32_t height, int fmt, int max_allocs)
{
        int fd, ret, map_fd;
        struct ion_handle *handle;
        uint16_t *ptr;
        uint32_t length;

        struct omap_ion_tiler_alloc_data alloc_data = {
                .w = width,
                .h = 1,
                .fmt = TILER_PIXEL_FMT_PAGE,
        };

        struct omap_ion_tiler_alloc_data alloc_data_2d = {
                .w = width,
                .h = height,
                .fmt = fmt,
		.out_align = PAGE_SIZE,
        };

        struct data {
        void    *bufPtr;
        } *mem_1d, *mem_2d;

        fd = ion_open();
        if (fd < 0)
                return fd;

        uint16_t val = (uint16_t) rand();

        /* allocate as many buffers as we can */
        mem_1d = (struct data *)calloc(max_allocs, sizeof(struct data));
        int ix, ix_2d, res = 0;

        for (ix = 0;  ix < max_allocs;)
        {
                ret = _ion_alloc_test(fd, &handle, &alloc_data);
                if (ret)
                        goto exit_1d;

                if (handle) {
                        mem_1d[ix].bufPtr = handle;
			if (verbose)
                              printf("handle allocated count = %d handle =%p\n",
				     ix, mem_1d[ix].bufPtr);
                        length = alloc_data.h * alloc_data.stride;
                        ret = ion_map(fd, handle, length, prot, map_flags, 0,
                                        &ptr, &map_fd);

                        if (ret)
                                return ret;

                        fill_mem(val, ptr, &alloc_data);
                        check_mem(val, ptr, &alloc_data);

                        munmap(ptr, length);
                        close(map_fd);
                        ix++;
                }
        }
        printf(":: Allocated %d buffers", ix);


      mem_2d = (struct data *)calloc(max_allocs, sizeof(struct data));
      for (ix_2d = 0;  ix_2d < max_allocs;)
        {
                ret = _ion_alloc_test(fd, &handle, &alloc_data_2d);
                if (ret)
                        goto exit_2d;

                if (handle) {
                        mem_2d[ix_2d].bufPtr = handle;
			if (verbose)
                              printf("handle allocated count = %d handle =%p\n",
					 ix_2d, mem_2d[ix_2d].bufPtr);
                        length = height * alloc_data_2d.stride;
                        ret = ion_map(fd, handle, length, prot, map_flags, 0,
                                        &ptr, &map_fd);

                        if (ret)
                                return ret;

                        fill_mem(val, ptr, &alloc_data_2d);
                        check_mem(val, ptr, &alloc_data_2d);

                        munmap(ptr, length);
                        close(map_fd);
                        ix_2d++;
                }
        }
        printf(":: Allocated %d 2d buffers", ix_2d);
exit_1d:
        while (ix--)
        {
                ret = ion_free(fd, mem_1d[ix].bufPtr);
                if (ret) {
                        printf("%s failed: %s %p\n", __func__, strerror(ret),
                                handle);
                        return ret;
                }

		if (verbose)
                    printf("handle de-allocated count = %d handle =%p\n", ix,
                   	     mem_1d[ix].bufPtr);
        }
        free(mem_1d);
exit_2d:
        while (ix_2d--)
        {
                ret = ion_free(fd, mem_2d[ix].bufPtr);
		if (verbose)
                    printf("handle de-allocated count = %d handle =%p\n", ix,
                   	     mem_2d[ix].bufPtr);
                if (ret) {
                        printf("%s failed: %s %p\n", __func__, strerror(ret),
                                handle);
                        return ret;
                }
        }
        free(mem_2d);

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
		.out_align = PAGE_SIZE,
        };

	struct omap_ion_tiler_alloc_data alloc_data_uv = {
                .w = width >> 1,
                .h = height >> 1,
                .fmt = TILER_PIXEL_FMT_16BIT,
		.out_align = PAGE_SIZE,
        };	
	

        fd = ion_open();
        if (fd < 0)
                return fd;
	
	uint16_t val = (uint16_t) rand();


        ret = _ion_alloc_test(fd, &handle_y, &alloc_data_y);
        if (ret)
           goto exit;
	ret = _ion_alloc_test(fd, &handle_uv, &alloc_data_uv);
        if (ret)
	   goto exit;

        length1 = alloc_data_y.h * alloc_data_y.stride;

        ret = ion_map(fd, handle_y, length1, prot, map_flags, 0, &ptr1, &map_fd1);
        if (ret)
                return ret;

	length2 = alloc_data_uv.h * alloc_data_uv.stride;
	ret = ion_map(fd, handle_uv, length2, prot, map_flags, 0, &ptr2, &map_fd2);
	if (ret)
		return ret;

	fill_mem(val, ptr1, &alloc_data_y);
        check_mem(val, ptr1, &alloc_data_y);
		
	fill_mem(val, ptr2, &alloc_data_uv);
        check_mem(val, ptr2, &alloc_data_uv);

        munmap(ptr1, length1);
	munmap(ptr2, length2);

	ret = ion_free(fd, handle_y);

	if (ret) {
                printf("%s failed: %s %p\n", __func__, strerror(ret), handle_y);
                return ret;
        }

	ret = ion_free(fd, handle_uv);

        if (ret) {
                printf("%s failed: %s %p\n", __func__, strerror(ret), handle_uv);
                return ret;
        }

        close(map_fd1);
	close(map_fd2);
exit:
	ion_close(fd);
	return ret;
}

int maxalloc_NV12_test(uint32_t width, uint32_t height, uint32_t max_allocs)
{
        int fd, map_fd1, map_fd2, ret;
        uint32_t length1, length2;
        uint16_t *ptr1, *ptr2;
        struct ion_handle *handle_y, *handle_uv;

        struct omap_ion_tiler_alloc_data alloc_data_y = {
                .w = width,
                .h = height,
                .fmt = TILER_PIXEL_FMT_8BIT,
		.out_align = PAGE_SIZE,
        };

        struct omap_ion_tiler_alloc_data alloc_data_uv = {
                .w = width >> 1,
                .h = height >> 1,
                .fmt = TILER_PIXEL_FMT_16BIT,
		.out_align = PAGE_SIZE,
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
	int ix;
    
	for (ix = 0;  ix < max_allocs;) {

        ret = _ion_alloc_test(fd, &handle_y, &alloc_data_y);
		if (ret)
			goto exit;
		
        ret = _ion_alloc_test(fd, &handle_uv, &alloc_data_uv);
		if (ret)
			goto exit;

	if (handle_y && handle_uv)
		{
			mem[ix].bufPtr1 = handle_y;
			mem[ix].bufPtr2 = handle_uv;
			if (verbose)
			      printf("handle allocated count = %d handle_y =%p \
				     handle_uv= %p\n", ix, mem[ix].bufPtr1,
                                     mem[ix].bufPtr2);
			ix++;
		}
	}
     
        length1 = alloc_data_y.h * alloc_data_y.stride;
        ret = ion_map(fd, handle_y, length1, prot, map_flags, 0, &ptr1, &map_fd1);
        if (ret)
                return ret;

        length2 = alloc_data_uv.h * alloc_data_uv.stride;
        ret = ion_map(fd, handle_uv, length2, prot, map_flags, 0, &ptr2, &map_fd2);
        if (ret)
                return ret;

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
			return ret;
		}
		ret = ion_free(fd, mem[ix].bufPtr2);
		if (ret) {
                	printf("%s failed: %s %p\n", __func__, strerror(ret), mem[ix].bufPtr2);
                	return ret;
            	}
		if (verbose)
		    printf("handle de-allocate count=%d handle_y=%p \
				handle_uv=%p\n",
				 ix, mem[ix].bufPtr1, mem[ix].bufPtr2);
	}
        ion_close(fd);
	return ret;
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

	struct omap_ion_tiler_alloc_data alloc_data = {
                .w = length,
                .h = 1,
                .fmt = TILER_PIXEL_FMT_PAGE,
        };
	int fd, map_fd, ret;
	struct ion_handle *handle;
	uint16_t *ptr;
 
        fd = ion_open();
        if (fd < 0)
                return fd;


	ret = _ion_alloc_test(fd, &handle, &alloc_data);
	if (ret)
		return ret;

	length = alloc_data.h * alloc_data.stride;
	ret = ion_map(fd, handle, length, prot, map_flags, 0, &ptr, &map_fd);
	if (ret)
		return ret;

	
	/* clean up properly */
	munmap(ptr,length);
	ret = ion_free(fd, handle);
	ion_close(fd);
	close(map_fd);

	return ret;
}

static int def_bpp(int fmt)
{
	return (fmt == TILER_PIXEL_FMT_32BIT ? 4 :
		fmt == TILER_PIXEL_FMT_16BIT ? 2 : 1);
}

void fill_mem(uint16_t start, uint16_t *ptr, struct omap_ion_tiler_alloc_data *al_data)
{
	uint16_t delta = 1, step = 1;
	uint32_t height, width, stride, i;
	if (al_data->fmt == TILER_PIXEL_FMT_PAGE)
    	{
        	height = 1;
        	stride = width = al_data->stride;
    	}
    	else {
        	height = al_data->h;
       		width = al_data->w;
        	stride = al_data->stride;
    	}
    	width *= def_bpp(al_data->fmt);

	if (verbose)
		printf("%p,%d*%d,s=%d stval=0x%x\n", al_data->handle, width,
			height, stride, start);

	uint32_t *ptr32 = (uint32_t *)ptr;
	while (height--)
    	{
        	if (al_data->fmt == TILER_PIXEL_FMT_32BIT)
        	{
            		for (i = 0; i < width; i += sizeof(uint32_t))
            		{
                		uint32_t val = (start & 0xFFFF) | 
					(((uint32_t)(start + delta) & 0xFFFF) << 16);
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
			ptr32 += (stride - i) / sizeof(uint32_t);
		}
		else {
            		for (i = 0; i < width; i += sizeof(uint16_t))
            		{
                		*ptr++ = start;
                		start += delta;
                		delta += step;
                /* increase step if overflown */
                	if (delta < step) delta = ++step;
            		}
            		ptr += (stride - i) / sizeof(uint16_t);
		}	
    	}
}

int check_mem(uint16_t start, uint16_t *ptr, struct omap_ion_tiler_alloc_data *al_data)
{
   //uint8_t *ptr8 = (y validation failed\n", __func__);uint8_t *)ptr; 
	uint16_t delta = 1, step = 1;
	uint32_t height, width, stride, r, i;
	if (al_data->fmt == TILER_PIXEL_FMT_PAGE)
    		{
        		height = 1;
        		stride = width = al_data->stride;
    	}
    	else {
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
            		ptr32 += (stride - i) / sizeof(uint32_t);
        	}
        	else {
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
		ptr += (stride - i) / sizeof(uint16_t);
        	}
	}
    	return 0;
}


int negative_1dl_test(int length)
{
        int fd, ret;
        struct ion_handle *handle;

        struct omap_ion_tiler_alloc_data alloc_data = {
                .w = length,
                .h = 1,
                .fmt = TILER_PIXEL_FMT_PAGE,
        };

        fd = ion_open();
        if (fd < 0)
                return fd;

        ret = _ion_alloc_test(fd, &handle, &alloc_data);

	/* Failure is expected here.  So if an error is set, return 0 */
	ret = (ret) ? 0 : 1;

        ion_close(fd);
	return ret;
}

int negative_1dh_test(int height)
{
        int fd, ret;
        struct ion_handle *handle;

        struct omap_ion_tiler_alloc_data alloc_data = {
                .w = 4096,
                .h = height,
                .fmt = TILER_PIXEL_FMT_PAGE,
        };

        fd = ion_open();
        if (fd < 0)
                return fd;

        ret = _ion_alloc_test(fd, &handle, &alloc_data);

        ion_close(fd);
	return !ret;
}

int negative_2d_test(uint32_t width, uint32_t height, int fmt)
{
        int fd, ret;
        struct ion_handle *handle;
        struct omap_ion_tiler_alloc_data alloc_data = {
                .w = height,
                .h = height,
                .fmt = fmt,
		.out_align = PAGE_SIZE,
        };

        fd = ion_open();
        if (fd < 0)
                return fd;

        ret = _ion_alloc_test(fd, &handle, &alloc_data);

        ion_close(fd);
	return !ret;
}

int negative_free_2D_test(uint32_t width, uint32_t height, int fmt)
{
        int fd, ret;
        struct ion_handle *handle;
        struct omap_ion_tiler_alloc_data alloc_data = {
                .w = width,
                .h = height,
                .fmt = 0,
        };

        fd = ion_open();
        if (fd < 0)
                return fd;

	ret = _ion_alloc_test(fd, &handle, &alloc_data);
	if (ret)
		goto exit;

        ret = ion_free(fd, handle);
	if (ret)
		goto exit;

	/* double free the handle to get a failure */
	ret = ion_free(fd, handle);
	ret = (ret == -EINVAL) ? 0 : ret;

exit:
	if (ret)
                printf("%s: %s\n", __func__, strerror(ret));

        ion_close(fd);
	return ret;
}


int negative_free_1D_test(uint32_t length, size_t stride)
{
        int fd, ret;
        struct ion_handle *handle;

        struct omap_ion_tiler_alloc_data alloc_data = {
                .w = length,
                .h = 1,
                .fmt = TILER_PIXEL_FMT_PAGE,
        };

        fd = ion_open();
        if (fd < 0)
                return fd;

	ret = _ion_alloc_test(fd, &handle, &alloc_data);
	if (ret)
        	goto exit;

        ret = ion_free(fd, handle);
	if (ret)
		goto exit;

	ret = ion_free(fd, handle);
	ret = (ret == -EINVAL) ? 0 : ret;

exit:
        if (ret)
                printf("%s: %s\n", __func__, strerror(ret));

        ion_close(fd);
	return ret;
}

int negative_arbitvalue_test(uint32_t length, size_t stride)
{
        int fd, ret;
        struct ion_handle *handle;

        struct omap_ion_tiler_alloc_data alloc_data = {
                .w = length,
                .h = 1,
                .fmt = TILER_PIXEL_FMT_PAGE,
        };

        fd = ion_open();
        if (fd < 0)
                return fd;

	ret = _ion_alloc_test(fd, &handle, &alloc_data);
	if (ret)
		goto exit;

        ret = ion_free(fd, (struct ion_handle *)0x12345678);
	ret = (ret == -EINVAL) ? 0 : ret;

exit:
        if (ret)
                printf("%s: %s\n", __func__, strerror(ret));

        ion_close(fd);
	return ret;
}

int random_alloc_test(uint32_t num_ops, uint16_t num_slots)
{
	printf("Random set of %d Allocs/Maps and Frees/UnMaps for %d slots\n", num_ops, num_slots);
	srand(0x4B72316A);
	struct data {
		int op;
		uint32_t width, height;
		uint32_t  length;
		void *bufPtr;
		void *bufPtr1;
		void *dataPtr;
		uint16_t stride;
		int fmt;
		int fd1;
	} *mem;

	struct ion_handle *handle, *handle_uv;
	int fd, ret, map_fd;
	uint16_t *ptr, *ptr1, *ptr2;
	uint32_t length, length1, length2;
	uint16_t val;
 
	struct omap_ion_tiler_alloc_data alloc_data = {0};

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
			switch (mem[ix].op) {
            		//case 0: //res = ion_free(fd, mem[ix].bufPtr);
                        	//free(mem[ix].buffer);
           			//      break;
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
          		/*  ZERO(mem[ix]);*/  
			memset(&(mem[ix]), 0, sizeof(mem[ix]));
        	}

        	/* we need to allocate/map data */
        	else {
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
            		switch (mem[ix].op) {
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
			      ret = ion_map(fd, mem[ix].bufPtr, length, prot,
						 map_flags, 0, &ptr, &map_fd);
                              if (ret)
                              	  	return ret;
			      fill_mem(val, ptr, &alloc_data);
                              check_mem(val, ptr, &alloc_data);
                              munmap(ptr, length);
                              ion_close(map_fd);
 
			      printf("value of bufPtr=%p, h=%d w=%d \n", mem[ix].bufPtr, 
					mem[ix].height, mem[ix].width);
        		      //        P("alloc[l=0x%x] = %p", mem[ix].length, mem[ix].bufPtr);
                	      break;
            		case 2:
			      alloc_data.fmt = TILER_PIXEL_FMT_8BIT;
                              alloc_data.out_align = PAGE_SIZE;
			      ret = _ion_alloc_test(fd, &handle, &alloc_data);
                	      if (ret == -EINVAL)
					goto exit;
			      mem[ix].bufPtr = handle;
			      mem[ix].stride = alloc_data.stride;
			      mem[ix].height = alloc_data.h;
                	      mem[ix].width = alloc_data.w;
			      length = mem[ix].height * mem[ix].stride;
		
			      ret = ion_map(fd, mem[ix].bufPtr, length, prot,
						 map_flags, 0, &ptr, &map_fd);
                              if (ret)
                                       return ret;
                              fill_mem(val, ptr, &alloc_data);
                              check_mem(val, ptr, &alloc_data);
                              munmap(ptr, length);
                              ion_close(map_fd);
   
    	    		      printf("value of bufPtr=%p, h=%d w=%d \n", mem[ix].bufPtr, 
						mem[ix].height, mem[ix].width);
                              //P("alloc[%d*%d*8] = %p", mem[ix].width, mem[ix].height, mem[ix].bufPtr);
                	      break;
			case 3:
			      alloc_data.fmt = TILER_PIXEL_FMT_16BIT;
                              alloc_data.out_align = PAGE_SIZE;
                	      ret = _ion_alloc_test(fd, &handle, &alloc_data);
                	      if (ret == -EINVAL)
					goto exit;
	 		      mem[ix].bufPtr = handle;
			      mem[ix].stride = alloc_data.stride;
			      mem[ix].height = alloc_data.h;
			      mem[ix].width = alloc_data.w;
			      length = mem[ix].height * mem[ix].stride;
			      ret = ion_map(fd, mem[ix].bufPtr, length, prot,
						 map_flags, 0, &ptr, &map_fd);
                              if (ret)
					return ret;
                              fill_mem(val, ptr, &alloc_data);
                              check_mem(val, ptr, &alloc_data);
                              munmap(ptr, length);
                              ion_close(map_fd);
	   		      printf("value of bufPtr=%p, h=%d w=%d \n", 
					mem[ix].bufPtr, mem[ix].height, 
					mem[ix].width);
             		      //  P("alloc[%d*%d*16] = %p", mem[ix].width, mem[ix].height, mem[ix].bufPtr);
                	      break;
			case 4:
			      alloc_data.fmt = TILER_PIXEL_FMT_32BIT;
                              alloc_data.out_align = PAGE_SIZE;
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
					return ret;
			      fill_mem(val, ptr, &alloc_data);
			      check_mem(val, ptr, &alloc_data);
                              munmap(ptr, length);
                              ion_close(map_fd);
    	                      printf("value of bufPtr=%p,h=%d w=%d \n",
					mem[ix].bufPtr, mem[ix].height,
					 mem[ix].width);
             		      //  P("alloc[%d*%d*32] = %p", mem[ix].width, mem[ix].height, mem[ix].bufPtr);
                	      break;
            		case 5:
			      alloc_data.fmt = TILER_PIXEL_FMT_8BIT;
                              alloc_data.out_align = PAGE_SIZE;
			      mem[ix].height = alloc_data.h;
                	      mem[ix].width = alloc_data.w;
			      struct omap_ion_tiler_alloc_data alloc_data_uv = {
                			.w = mem[ix].width >> 1,
                		        .h = mem[ix].height >> 1,
                	 	        .fmt = TILER_PIXEL_FMT_16BIT,
					.out_align = PAGE_SIZE,
			      };

        		      ret = _ion_alloc_test(fd, &handle, &alloc_data);
              		      if (ret == -EINVAL)
					goto exit;
		       	      ret = _ion_alloc_test(fd, &handle_uv, &alloc_data_uv);
                	      if (ret == -EINVAL)
					goto exit;
			      mem[ix].bufPtr = handle;
			      mem[ix].bufPtr1 = handle_uv;
			      printf("value of NV12 Y bufPtr=%p, NV12 UV bufPtr=%p \n", 
					mem[ix].bufPtr, mem[ix].bufPtr1);

              		      length1 = alloc_data.h * alloc_data.stride;
        		      ret = ion_map(fd, handle, length1, prot,
						 map_flags, 0, &ptr1, &map_fd);
        		      if (ret)
                			return ret;
	                      length2 = alloc_data_uv.h * alloc_data_uv.stride;
              		      ret = ion_map(fd, handle_uv, length2, prot,
						 map_flags, 0, &ptr2, &map_fd);
        		      if (ret)
                			return ret;
	
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
	}
}

exit:

    /* unmap and free everything */
	for (ix = 0; ix < num_slots; ix++)
	{
		if (mem[ix].bufPtr)
        	{
            	/* check memory fill */
            		switch (mem[ix].op) {
            		case 0: printf("hello\n");
			      break;
            		case 1: 
			      ion_free(fd, mem[ix].bufPtr);
                              break;
            		case 2:
                              ion_free(fd, mem[ix].bufPtr);
                              break;
            		case 3:
			      ion_free(fd, mem[ix].bufPtr);
			      break;
            		case 4:
			      ion_free(fd, mem[ix].bufPtr);
			      break;
            		case 5:
			       ion_free(fd, mem[ix].bufPtr);
			      ion_free(fd, mem[ix].bufPtr1);
		    	      break;
            		}
        	}
    	}
    	free(mem);
	ion_close(fd);

	return res;
}

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

