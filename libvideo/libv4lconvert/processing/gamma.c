/*
#             (C) 2008-2009 Hans de Goede <hdegoede@redhat.com>

# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU Lesser General Public License as published by
# the Free Software Foundation; either version 2.1 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU Lesser General Public License for more details.
#
# You should have received a copy of the GNU Lesser General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 51 Franklin Street, Suite 500, Boston, MA  02110-1335  USA
 */

#include <math.h>
#include "libv4lprocessing.h"
#include "libv4lprocessing-priv.h"

#define CLIP(value) ((u8) ((value) > 254.5f ? 255 : ((value) < 0 ? 0 : ((value) + 0.5f))))
#ifndef UNUSED
#define UNUSED(x) ((void)x)
#endif

static int gamma_active(struct v4lprocessing_data *data) {
	int gamma = v4lcontrol_get_ctrl(data->control, V4LCONTROL_GAMMA);

	return gamma && gamma != 1000;
}

static int gamma_calculate_lookup_tables(struct v4lprocessing_data *data, u8 *buf, const struct v4l2_format *fmt) {
	UNUSED(buf);
	UNUSED(fmt);
	int gamma = v4lcontrol_get_ctrl(data->control, V4LCONTROL_GAMMA);

	if (gamma != data->last_gamma) {
		//Build & cache gamma table
		const float invGamma = 1000.0f / ((float) gamma);
		for (unsigned int i = 0; i < 256; i++) {
			float val = powf((float)i / 255.0f, invGamma) * 255.0f;
			data->gamma_table[i] = CLIP(val);
		}
		data->last_gamma = gamma;
	}

	for (unsigned int i = 0; i < 256; i++) {
		data->comp1[i] = data->gamma_table[data->comp1[i]];
		data->green[i] = data->gamma_table[data->green[i]];
		data->comp2[i] = data->gamma_table[data->comp2[i]];
	}

	return 1;
}

struct v4lprocessing_filter gamma_filter = {
	gamma_active, gamma_calculate_lookup_tables
};
#undef CLIP