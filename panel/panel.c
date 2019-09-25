/*
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include "panel/panel.h"
#include "common/log.h"
#include "protocol/wlr-layer-shell-unstable-v1-client-protocol.h"

static void xdg_wm_base_ping(void *data, struct xdg_wm_base *shell, uint32_t serial)
{
	xdg_wm_base_pong(shell, serial);
}

static const struct xdg_wm_base_listener wm_base_listener = {
	.ping = xdg_wm_base_ping,
};

static void registry_handle_global(void *data, struct wl_registry *registry, 
		uint32_t id, const char *interface, uint32_t version)
{
	struct spider_panel *panel = data;
	spider_dbg("Got a registry event for %s id %d\n", interface, id);
	if (strcmp(interface, "wl_compositor") == 0){
		panel->compositor = wl_registry_bind(registry, id,
				&wl_compositor_interface, 3);
	}else if (strcmp(interface, "wl_output") == 0) {
		panel->output = wl_registry_bind(registry, id, 
				&wl_output_interface, 2);
	}else if (strcmp(interface, "wl_seat") == 0) {
		panel->seat = wl_registry_bind(registry, id, &wl_seat_interface, 5);
	} else if (strcmp(interface, "wl_shm") == 0) {
		panel->shm = wl_registry_bind(registry, id, &wl_shm_interface, 1);
	} else if (strcmp(interface, zwlr_layer_shell_v1_interface.name) == 0) {
		panel->layer_shell = wl_registry_bind(
				registry, id, &zwlr_layer_shell_v1_interface, 1);
	} else if (strcmp(interface, "xdg_wm_base") == 0) {
		panel->wm_base = wl_registry_bind(registry, id, &xdg_wm_base_interface, 1);
		xdg_wm_base_add_listener(panel->wm_base, &wm_base_listener, panel);
	} else if (strcmp(interface, "spider_desktop_manager_v1") == 0) {
		panel->desktop_manager = wl_registry_bind(registry, id, &spider_desktop_manager_v1_interface, 1);
	}
}

static void registry_remove_global(void *data, struct wl_registry *registry, 
		uint32_t id)
{
	spider_dbg("Got a registry losing event for %d\n", id);
}

static const struct wl_registry_listener registry_listener = {
	registry_handle_global,
	registry_remove_global
};

int panel_init(struct spider_panel *panel)
{
	int ret = 1;
	int status = -1;

	panel->gdk_display = gdk_display_get_default();
	panel->display = gdk_wayland_display_get_wl_display(panel->gdk_display);
	if (panel->display == NULL) {
		goto OUT;
	}

	panel->registry = wl_display_get_registry(panel->display);
	if (panel->registry == NULL) {
		goto OUT;
	}
	wl_registry_add_listener(panel->registry, &registry_listener, panel);

	wl_display_roundtrip(panel->display);

	status = wl_display_dispatch(panel->display);
	if (status == -1) {
		goto OUT;
	}

	ret = 0;
OUT:
	return ret;
}
