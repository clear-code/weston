/*
 * Copyright 2022 ClearCode Inc.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the next
 * paragraph) shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */

#include "config.h"

#include <string.h>
#include <stdio.h>
#include <wayland-client.h>
#include <weston-global-touch-client-protocol.h>


static struct app {
	struct wl_display *display;
	struct weston_global_touch *global_touch;
	const char *command;
} app;

static void
touch_down_handler(void *data, struct weston_global_touch *interface,
		   uint32_t time, int32_t touch_id, uint32_t x, uint32_t y)
{
	printf("touch_down_handler\n"
	       "    time: %d\n"
	       "    touch_id: %d\n"
	       "    x: %lf\n"
	       "    y: %lf\n",
	       time, touch_id,
	       wl_fixed_to_double(x),
	       wl_fixed_to_double(y));
}

static void
touch_up_handler(void *data, struct weston_global_touch *interface,
		 uint32_t time, int32_t touch_id)
{
	printf("touch_up_handler\n"
	       "    time: %d\n"
	       "    touch_id: %d\n",
	       time, touch_id);
}

static void
touch_motion_handler(void *data, struct weston_global_touch *interface,
	       uint32_t time, int32_t touch_id, uint32_t x, uint32_t y)
{
	printf("touch_motion_handler\n"
	       "    time: %d\n"
	       "    touch_id: %d\n"
	       "    x: %lf\n"
	       "    y: %lf\n",
	       time, touch_id,
	       wl_fixed_to_double(x),
	       wl_fixed_to_double(y));
}

static void
touch_frame_handler(void *data, struct weston_global_touch *interface)
{
	printf("touch_frame_handler\n");
}

static void
touch_cancel_handler(void *data, struct weston_global_touch *interface)
{
	printf("touch_cancel_handler\n");
}

struct weston_global_touch_listener touch_listener = {
	touch_down_handler,
	touch_up_handler,
	touch_motion_handler,
	touch_frame_handler,
	touch_cancel_handler
};

static void
run_monitor(struct weston_global_touch *global_touch)
{
	weston_global_touch_add_listener(global_touch,
					 &touch_listener, &app);
	while (wl_display_dispatch(app.display) != -1);
}

static void
global_handler(void *data, struct wl_registry *registry, uint32_t id,
	       const char *interface, uint32_t version)
{
	if (!strcmp(interface, "weston_global_touch")) {
		app.global_touch
			= wl_registry_bind(registry, id,
					   &weston_global_touch_interface, 1);
		if (app.command && !strcmp(app.command, "enable")) {
			weston_global_touch_enable(app.global_touch);
			wl_display_roundtrip(app.display);
		} else if (app.command && !strcmp(app.command, "disable")) {
			weston_global_touch_disable(app.global_touch);
			wl_display_roundtrip(app.display);
		} else if (app.command && !strcmp(app.command, "monitor")) {
			run_monitor(app.global_touch);
		} else if (app.command && *app.command) {
			fprintf(stderr, "Unknown command: %s\n", app.command);
		} else {
			fprintf(stderr, "Command isn't specified\n");
		}
	}
}

static void
global_remove_handler(void *data, struct wl_registry *registry, uint32_t name)
{
}

static const struct wl_registry_listener registry_listener = {
	global_handler,
	global_remove_handler
};

int
main(int argc, char **argv)
{
	struct wl_display *display;
	struct wl_registry *registry;

	display = wl_display_connect(NULL);
	if (!display) {
		fprintf(stderr, "failed to create display\n");
		return -1;
	}

	app.display = display;
	app.global_touch = NULL;
	app.command = argc > 1 ? argv[1] : NULL;

	registry = wl_display_get_registry(display);
	wl_registry_add_listener(registry, &registry_listener, &app);

	wl_display_roundtrip(display);

	if (app.global_touch) {
		weston_global_touch_destroy(app.global_touch);
	} else {
		fprintf(stderr,
			"weston-global-touch protocol isn't supported!\n");
	}
	wl_registry_destroy(registry);
	wl_display_disconnect(display);

	return 0;
}
