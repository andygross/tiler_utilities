/*
 *  ion.c
 *
 * Memory Allocator functions for ion
 *
 *   Copyright 2011 Google, Inc
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 */
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdint.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <sys/types.h>

//#define LOG_TAG "ion"
//#include <cutils/log.h>

#include "ion_user.h"

static int usage_count;

int ion_open()
{
//	int fd;
  //      if (usage_count == 0) {
              int fd = open("/dev/ion", O_RDWR);
                if (fd < 0) {
                printf("open /dev/ion failed!\n");
        }
        else {
		printf("open: /dev/ion success\n");
        }
        return fd;
}

int ion_close(int fd)
{
	//if (usage_count == 1) {
	  //   usage_count--;
             return close(fd);
//	}
//	else {
//		usage_count--;
//		return 0;
//	}
}

int ion_ioctl(int fd, int req, void *arg)
{
        int ret = ioctl(fd, req, arg);
        if (ret < 0) {
                printf("ioctl %d failed with code %d: %s\n", req,
                       ret, strerror(errno));
                return -errno;
        }
        return ret;
}

int ion_alloc(int fd, size_t len, size_t align, unsigned int flags,
              struct ion_handle **handle)
{
        int ret;
        struct ion_allocation_data data = {
                .len = len,
                .align = align,
                .flags = flags,
        };

        ret = ion_ioctl(fd, ION_IOC_ALLOC, &data);
        if (ret < 0)
                return ret;
        *handle = data.handle;
        return ret;
}

int ion_alloc_tiler(int fd, struct omap_ion_tiler_alloc_data **al_data, struct ion_handle **handle, size_t *stride)
{
        int ret;
      //  struct omap_ion_tiler_alloc_data alloc_data = {
        //        .w = w,
          //      .h = h,
            //    .fmt = fmt,
              //  .flags = flags,
       // };

        struct ion_custom_data custom_data = {
                .cmd = OMAP_ION_TILER_ALLOC,
                .arg = (unsigned long)(*al_data),
        };
//	printf("value of width = %d height = %d fmt = %d\n", (*al_data)->w, (*al_data)->h, (*al_data)->fmt);
        ret = ion_ioctl(fd, ION_IOC_CUSTOM, &custom_data);
        if (ret < 0)
                return ret;
        *stride = (*al_data)->stride;
        *handle = (*al_data)->handle;
//	 printf("++++++++++++++++handle value =%x++++++++++++++++\n", *handle);
        return ret;
}

int ion_free(int fd, struct ion_handle *handle)
{
        struct ion_handle_data data = {
                .handle = handle,
        };
        return ion_ioctl(fd, ION_IOC_FREE, &data);
}

int ion_map(int fd, struct ion_handle *handle, size_t length, int prot,
            int flags, off_t offset, uint16_t **ptr, int *map_fd)
{
        struct ion_fd_data data = {
                .handle = handle,
        };

        int ret = ion_ioctl(fd, ION_IOC_MAP, &data);
        if (ret < 0)
                return ret;
        *map_fd = data.fd;
        if (*map_fd < 0) {
                printf("map ioctl returned negative fd\n");
                return -EINVAL;
        }
        *ptr = mmap(NULL, length, prot, flags, *map_fd, offset);
        if (*ptr == MAP_FAILED) {
                printf("mmap failed: %s\n", strerror(errno));
                return -errno;
        }
        return ret;
}

int ion_share(int fd, struct ion_handle *handle, int *share_fd)
{
        int map_fd;
        struct ion_fd_data data = {
                .handle = handle,
        };
        int ret = ion_ioctl(fd, ION_IOC_SHARE, &data);
        if (ret < 0)
                return ret;
        *share_fd = data.fd;
        if (*share_fd < 0) {
                printf("map ioctl returned negative fd\n");
                return -EINVAL;
        }
        return ret;
}

int ion_import(int fd, int share_fd, struct ion_handle **handle)
{
        struct ion_fd_data data = {
                .fd = share_fd,
        };
        int ret = ion_ioctl(fd, ION_IOC_IMPORT, &data);
        if (ret < 0)
                return ret;
        *handle = data.handle;
        return ret;
}
