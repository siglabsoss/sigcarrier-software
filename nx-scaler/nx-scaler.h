/*
 * Copyright (C) 2016  Nexell Co., Ltd.
 * Author: Jongkeun, Choi <jkchoi@nexell.co.kr>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#ifndef __NX_SCALER_H
#define __NX_SCALER_H

#define MAX_PLANE_NUM   3

#if defined(__cplusplus)
extern "C" {
#endif

struct rect {
	int x;
	int y;
	int width;
	int height;
};

struct nx_scaler_context {
	int		src_plane_num;
	int		src_fds[MAX_PLANE_NUM];
	int		src_stride[MAX_PLANE_NUM];
	unsigned int	src_width;
	unsigned int	src_height;
	unsigned int	src_code;

	int		dst_plane_num;
	int		dst_fds[MAX_PLANE_NUM];
	int		dst_stride[MAX_PLANE_NUM];
	unsigned int	dst_width;
	unsigned int	dst_height;
	unsigned int	dst_code;

	struct rect	crop;
};

int scaler_open(void);
int nx_scaler_run(int handle, struct nx_scaler_context *s_ctx);
void nx_scaler_close(int handle);

#if defined(__cplusplus)
}
#endif

#endif
