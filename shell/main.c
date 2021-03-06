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

/* 
 * This code is based on https://github.com/aragua/gtkbrowser.
 */

#include <stdlib.h>
#include <unistd.h>
#include <gtk/gtk.h>
#include "shell/shell.h"
#include "common/webkitapi.h"
#include "common/log.h"
#include "common/global_vars.h"

int SPIDER_LOGLEVEL = 2;

static void scale_fullscreen(int *screen_width, int *screen_height)
{
	GdkScreen *screen = gdk_screen_get_default ();

	*screen_width = gdk_screen_get_width(screen);
	*screen_height = gdk_screen_get_height(screen);
}

static void map_win_cb(GtkWidget* widget, gpointer data)
{
	struct spider_shell *shell = data;

	spider_compositor_manager_v1_set_background(shell->compositor_manager, shell->surface);
}

static void destroy_win_cb(GtkWidget* widget, GtkWidget* window)
{
	gtk_main_quit();
}

int main(int argc, char* argv[])
{
	struct spider_shell shell;
	GtkWidget *window = NULL;
	GdkWindow *gdk_window;
	WebKitWebView *web = NULL;
	int screen_width;
	int screen_height;
	char *url = NULL;
	char *loglevel;

	loglevel = getenv("LOGLEVEL");
	if (loglevel) {
		SPIDER_LOGLEVEL = atoi(loglevel);
		spider_dbg("LOGLEVEL is set to %s\n", loglevel);
	}

	loglevel = getenv("LOGLEVEL_SHELL");
	if (loglevel) {
		SPIDER_LOGLEVEL = atoi(loglevel);
		spider_dbg("LOGLEVEL(shell) is set to %s\n", loglevel);
	}

	url = getenv(SPIDER_WEB_URL);
	spider_dbg("URL=%s\n", url);

	gdk_set_allowed_backends("wayland");
	gtk_init(&argc, &argv);

	shell_init(&shell);

	scale_fullscreen(&screen_width, &screen_height);

	window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_set_default_size(GTK_WINDOW(window), screen_width, screen_height);
	gtk_window_set_decorated(GTK_WINDOW(window), FALSE);
	gtk_window_fullscreen(GTK_WINDOW(window));
	gtk_widget_realize(window);

	web = WEBKIT_WEB_VIEW(webkit_web_view_new());
	gtk_container_add(GTK_CONTAINER(window), GTK_WIDGET(web));

	g_signal_connect(window, "map", G_CALLBACK(map_win_cb), &shell);
	g_signal_connect(window, "destroy", G_CALLBACK(destroy_win_cb), NULL);
	g_signal_connect(web, "decide-policy", G_CALLBACK(wkapi_dp_cb), window);
	g_signal_connect(web, "submit-form", G_CALLBACK(wkapi_sf_cb), window);
	g_signal_connect(web, "close", G_CALLBACK(wkapi_close_cb), window);

	webkit_web_view_load_uri(web, url);

	gtk_widget_grab_focus(GTK_WIDGET(web));

	gdk_window = gtk_widget_get_window(window);
	//shell.surface = wl_compositor_create_surface(shell.compositor);
	shell.surface = gdk_wayland_window_get_wl_surface(gdk_window);
	//shell.compositor = gdk_wayland_display_get_wl_compositor(shell.gdk_display);
	//gdk_wayland_window_set_use_custom_surface(gdk_window);
	//shell.xdg_surface = xdg_wm_base_get_xdg_surface(shell.wm_base, shell.surface);
	spider_dbg("%p\n", shell.surface);

	//spider_compositor_manager_v1_set_background(shell.compositor_manager, shell.surface);
	gtk_widget_show_all(window);

	gtk_main();

	return 0;
}
