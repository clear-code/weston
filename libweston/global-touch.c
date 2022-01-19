/*
 * Copyright 2022 ClearCode Inc.
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice (including the
 * next paragraph) shall be included in all copies or substantial
 * portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT.  IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
 * BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
 * ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include "config.h"
#include "compositor.h"
#include "weston-global-touch-server-protocol.h"

struct weston_global_touch {
	struct wl_global *global_touch;
	struct wl_list resource_list;
};

static void
global_touch_disable(struct wl_client *client,
		     struct wl_resource *resource)
{
	struct weston_compositor *compositor = wl_resource_get_user_data(resource);
	printf("Disable touchscreen!\n");
	weston_input_touchscreen_set_enabled(compositor, false);
}

static void
global_touch_enable(struct wl_client *client,
		    struct wl_resource *resource)
{
	struct weston_compositor *compositor = wl_resource_get_user_data(resource);
	printf("Enable touchscreen!\n");
	weston_input_touchscreen_set_enabled(compositor, true);
}

static const struct weston_global_touch_interface
global_touch_implementation = {
	global_touch_disable,
	global_touch_enable,
};

static void
destroy_resource(struct wl_resource *resource)
{
	wl_list_remove(wl_resource_get_link(resource));
}

static void
bind_global_touch(struct wl_client *client,
		  void *data, uint32_t version, uint32_t id)
{
	struct weston_compositor *compositor = data;
	struct wl_resource *resource;

	resource = wl_resource_create(client,
				      &weston_global_touch_interface,
				      version, id);
	if (resource == NULL) {
		wl_client_post_no_memory(client);
		return;
	}

	wl_resource_set_implementation(resource,
				       &global_touch_implementation,
				       compositor, destroy_resource);
	wl_list_insert(&compositor->global_touch->resource_list,
		       wl_resource_get_link(resource));
}

WL_EXPORT int
weston_compositor_create_global_touch(struct weston_compositor *compositor)
{
	if (compositor->global_touch)
		return -1;

	compositor->global_touch = zalloc(sizeof(struct weston_global_touch));
	if (!compositor->global_touch)
		return -1;

	compositor->global_touch->global_touch
		= wl_global_create(compositor->wl_display,
				   &weston_global_touch_interface, 1,
				   compositor, bind_global_touch);
	if (!compositor->global_touch) {
		free(compositor->global_touch->global_touch);
		compositor->global_touch = NULL;
		return -1;
	}

	wl_list_init(&compositor->global_touch->resource_list);

	return 0;
}

WL_EXPORT int
weston_compositor_destroy_global_touch(struct weston_compositor *compositor)
{
	if (!compositor->global_touch)
		return -1;

	wl_global_destroy(compositor->global_touch->global_touch);
	free(compositor->global_touch);
	compositor->global_touch = NULL;

	return 0;
}

static inline int64_t
timespec_to_msec(const struct timespec *a)
{
        return (int64_t)a->tv_sec * 1000 + a->tv_nsec / 1000000;
}

WL_EXPORT void
notify_global_touch(struct weston_touch_device *device,
		    const struct timespec *time, int touch_id,
		    double double_x, double double_y,
		    int touch_type)
{
	struct weston_compositor *compositor = device->aggregate->seat->compositor;
	struct wl_list *resource_list = &compositor->global_touch->resource_list;
	struct wl_resource *resource;
	uint32_t msecs = timespec_to_msec(time);
	wl_fixed_t x = wl_fixed_from_double(double_x);
	wl_fixed_t y = wl_fixed_from_double(double_y);

	switch (touch_type) {
	case WL_TOUCH_UP:
		wl_resource_for_each(resource, resource_list)
			weston_global_touch_send_up(resource, msecs, touch_id);
		break;
	case WL_TOUCH_DOWN:
		wl_resource_for_each(resource, resource_list)
			weston_global_touch_send_down(resource, msecs,
						      touch_id, x, y);
		break;
	case WL_TOUCH_MOTION:
		wl_resource_for_each(resource, resource_list)
			weston_global_touch_send_motion(resource, msecs,
							touch_id, x, y);
		break;
	default:
		return;
	}
}

WL_EXPORT void
notify_global_touch_frame(struct weston_touch_device *device)
{
	struct weston_compositor *compositor = device->aggregate->seat->compositor;
	struct wl_resource *resource;

	wl_resource_for_each(resource, &compositor->global_touch->resource_list)
		weston_global_touch_send_frame(resource);
}

WL_EXPORT void
notify_global_touch_cancel(struct weston_touch_device *device)
{
	struct weston_compositor *compositor = device->aggregate->seat->compositor;
	struct wl_resource *resource;

	wl_resource_for_each(resource, &compositor->global_touch->resource_list)
		weston_global_touch_send_cancel(resource);
}
