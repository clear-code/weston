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
#include "weston-touch-switch-server-protocol.h"

static void
touch_switch_destroy(struct wl_client *client,
		     struct wl_resource *resource)
{
	wl_resource_destroy(resource);
}

static void
touch_switch_disable(struct wl_client *client,
		     struct wl_resource *resource)
{
	printf("Disable touchscreen!");
}

static void
touch_switch_enable(struct wl_client *client,
		    struct wl_resource *resource)
{
	printf("Enable touchscreen!");
}

static const struct weston_touch_switch_interface
touch_switch_implementation = {
	touch_switch_destroy,
	touch_switch_disable,
	touch_switch_enable,
};

static void
bind_touch_switch(struct wl_client *client,
		  void *data, uint32_t version, uint32_t id)
{
	struct weston_compositor *compositor = data;
	struct wl_resource *resource;

	resource = wl_resource_create(client,
				      &weston_touch_switch_interface,
				      version, id);
	if (resource == NULL) {
		wl_client_post_no_memory(client);
		return;
	}

	wl_resource_set_implementation(resource,
				       &touch_switch_implementation,
				       compositor, NULL);
}

WL_EXPORT int
weston_compositor_enable_touch_switch(struct weston_compositor *compositor)
{
	if (compositor->touch_switch)
		return -1;

	compositor->touch_switch = wl_global_create(compositor->wl_display,
						    &weston_touch_switch_interface, 1,
						    compositor, bind_touch_switch);
	if (!compositor->touch_switch)
		return -1;

	return 0;
}
