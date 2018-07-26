/*
 * Copyright (c) 2016 Nexell Co., Ltd.
 * Author: Sungwoo, Park <swpark@nexell.co.kr>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of
 * this software and associated documentation files (the "Software"), to deal in
 * the Software without restriction, including without limitation the rights to
 * use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
 * the Software, and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#ifndef _NX_V4L2_H
#define _NX_V4L2_H

#ifdef __cplusplus
extern "C" {
#endif

enum {
	nx_sensor_subdev = 0,
	nx_clipper_subdev,
	nx_decimator_subdev,
	nx_csi_subdev,
	nx_clipper_video,
	nx_decimator_video,
	nx_v4l2_max
};

int nx_v4l2_open_device(int type, int module);
void nx_v4l2_cleanup(void);
bool nx_v4l2_is_mipi_camera(int module);
int nx_v4l2_link(bool link, int module, int src_type, int src_pad,
		 int sink_type, int sink_pad);
int nx_v4l2_set_format(int fd, int type, uint32_t w, uint32_t h,
		       uint32_t format);
int nx_v4l2_get_format(int fd, int type, uint32_t *w, uint32_t *h,
		       uint32_t *format);
int nx_v4l2_set_crop(int fd, int type, uint32_t x, uint32_t y, uint32_t w,
		     uint32_t h);
int nx_v4l2_get_crop(int fd, int type, uint32_t *x, uint32_t *y, uint32_t *w,
		     uint32_t *h);
int nx_v4l2_set_ctrl(int fd, int type, uint32_t ctrl_id, int value);
int nx_v4l2_get_ctrl(int fd, int type, uint32_t ctrl_id, int *value);
int nx_v4l2_reqbuf(int fd, int type, int count);
int nx_v4l2_qbuf(int fd, int type, int plane_num, int index, int *fds,
		 int *sizes);
int nx_v4l2_dqbuf(int fd, int type, int plane_num, int *index);
int nx_v4l2_dqbuf_with_timestamp(int fd, int type, int plane_num, int *index,
				 struct timeval *timeval);
int nx_v4l2_streamon(int fd, int type);
int nx_v4l2_streamoff(int fd, int type);
int nx_v4l2_set_parm(int fd, int type, struct v4l2_streamparm *parm);

/* API for mmap type */
int nx_v4l2_set_format_mmap(int fd, int type, uint32_t w, uint32_t h,
			    uint32_t format);
int nx_v4l2_set_crop_mmap(int fd, int type, uint32_t x, uint32_t y, uint32_t w,
			  uint32_t h);
int nx_v4l2_reqbuf_mmap(int fd, int type, int count);
int nx_v4l2_qbuf_mmap(int fd, int type, int index);
int nx_v4l2_dqbuf_mmap(int fd, int type, int *index);
int nx_v4l2_dqbuf_mmap_with_timestamp(int fd, int type, int *index,
				      struct timeval *timeval);
int nx_v4l2_streamon_mmap(int fd, int type);
int nx_v4l2_streamoff_mmap(int fd, int type);
int nx_v4l2_query_buf_mmap(int fd, int type, int index,
			   struct v4l2_buffer *v4l2_buf);

#ifdef __cplusplus
}
#endif

#endif
