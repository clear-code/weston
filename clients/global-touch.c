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

static const char *command = NULL;

static void
global_handler(void *data, struct wl_registry *registry, uint32_t id,
	       const char *interface, uint32_t version)
{
	if (!strcmp(interface, "weston_global_touch")) {
		struct weston_global_touch *switcher;
		switcher = wl_registry_bind(registry, id,
					    &weston_global_touch_interface,
					    1);
		if (command && !strcmp(command, "enable")) {
			weston_global_touch_enable(switcher);
		} else if (command && !strcmp(command, "disable")) {
			weston_global_touch_disable(switcher);
		} else if (command && *command) {
			fprintf(stderr, "Unknown command: %s\n", command);
		} else {
			fprintf(stderr, "Command isn't specified\n");
		}
		weston_global_touch_destroy(switcher);
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

	if (argc > 1)
		command = argv[1];

	registry = wl_display_get_registry(display);
	wl_registry_add_listener(registry, &registry_listener, NULL);

	wl_display_roundtrip(display);
	wl_display_roundtrip(display);

	wl_registry_destroy(registry);
	wl_display_disconnect(display);

	return 0;
}
