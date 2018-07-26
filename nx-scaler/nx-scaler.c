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

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>

#include <sys/ioctl.h>
#include <sys/types.h>

#include "scaler.h"
#include "nx-scaler.h"

void convert_scaling_data(struct nx_scaler_ioctl_data *data,
	struct nx_scaler_context *ctx)
{
	data->src_plane_num	= ctx->src_plane_num;
	memcpy(data->src_fds, ctx->src_fds, sizeof(ctx->src_fds));
	memcpy(data->src_stride, ctx->src_stride,
		sizeof(ctx->src_stride));
	data->src_width	= ctx->src_width;
	data->src_height	= ctx->src_height;
	data->src_code	= ctx->src_code;

	data->dst_plane_num	= ctx->dst_plane_num;
	memcpy(data->dst_fds, ctx->dst_fds, sizeof(ctx->dst_fds));
	memcpy(data->dst_stride, ctx->dst_stride,
		sizeof(ctx->dst_stride));
	data->dst_width	= ctx->dst_width;
	data->dst_height	= ctx->dst_height;
	data->dst_code	= ctx->dst_code;

	data->crop		= ctx->crop;
}

int scaler_open(void)
{
	int handle;

	handle = open("/dev/scaler", O_RDWR);
	if (handle < 0)
		return -ENODEV;

	return handle;
}

int nx_scaler_run(int handle, struct nx_scaler_context *ctx)
{
	struct nx_scaler_ioctl_data data;

	convert_scaling_data(&data, ctx);

	return ioctl(handle, IOCTL_SCALER_SET_AND_RUN, &data);
}

void nx_scaler_close(int handle)
{
	if (handle)
		close(handle);
}
