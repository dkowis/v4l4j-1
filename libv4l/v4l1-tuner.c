/*
* Copyright (C) 2007-2008 Gilles Gigan (gilles.gigan@gmail.com)
* eResearch Centre, James Cook University (eresearch.jcu.edu.au)
*
* This program was developed as part of the ARCHER project
* (Australian Research Enabling Environment) funded by a
* Systemic Infrastructure Initiative (SII) grant and supported by the Australian
* Department of Innovation, Industry, Science and Research
*
* This program is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public  License as published by the
* Free Software Foundation, either version 3 of the License, or
* (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
* or FITNESS FOR A PARTICULAR PURPOSE.
* See the GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program.  If not, see <http://www.gnu.org/licenses/>.
*
*/
#include <sys/ioctl.h>
#include "libv4l-err.h"
#include "log.h"

int set_tuner_freq_v4l1(struct video_device *vdev, int idx, unsigned int f){
	if(-1 == ioctl(vdev->fd, VIDIOCSFREQ, &f)){
		dprint(LIBV4L_LOG_SOURCE_TUNER, LIBV4L_LOG_LEVEL_ERR, "TUN: Failed to set tuner frequency on device %s\n", vdev->file);
		return LIBV4L_ERR_IOCTL;
	}
	return 0;
}

int get_tuner_freq_v4l1(struct video_device *vdev, int idx, unsigned int *f){
	if(-1 == ioctl(vdev->fd, VIDIOCGFREQ, f)){
		dprint(LIBV4L_LOG_SOURCE_TUNER, LIBV4L_LOG_LEVEL_ERR, "TUN: Failed to get tuner frequency on device %s\n", vdev->file);
		return LIBV4L_ERR_IOCTL;
	}
	return 0;
}

int get_rssi_afc_v4l1(struct video_device *vdev, int idx, int *r, int *a){
	struct video_tuner t;
	CLEAR(t);
	t.tuner = 0;
	if(-1 == ioctl (vdev->fd, VIDIOCGTUNER, &t)){
		dprint(LIBV4L_LOG_SOURCE_TUNER, LIBV4L_LOG_LEVEL_ERR, "TUN: Failed to get tuner info on device %s\n", vdev->file);
		return LIBV4L_ERR_IOCTL;
	}
	dprint(LIBV4L_LOG_SOURCE_TUNER, LIBV4L_LOG_LEVEL_DEBUG, "TUN: Got RSSI %d & AFC 0 on device %s\n", t.signal, vdev->file);
	*r = t.signal;
	*a = 0;
	return 0;
}

