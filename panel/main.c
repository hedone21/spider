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

#include <stdlib.h>
#include <unistd.h>
#include <gtk/gtk.h>
#include "panel/panel.h"
#include "common/webkitapi.h"
#include "common/log.h"
#include "common/global_vars.h"

static void set_panelsize(struct spider_panel *panel, int portion)
{
	GdkScreen *screen = gdk_screen_get_default ();

	panel->panel_width = gdk_screen_get_width(screen);
	panel->panel_height = gdk_screen_get_height(screen) * portion / 100;
}

static void map_win_cb(GtkWidget* widget, gpointer data)
{
	struct spider_panel *panel = data;

	spider_compositor_manager_v1_set_bar(panel->compositor_manager, panel->surface, 0, 0);
}

static void destroy_win_cb(GtkWidget* widget, GtkWidget* window)
{
	gtk_main_quit();
}

int main(int argc, char *argv[])
{
	struct spider_panel panel;
	GdkWindow *gdk_window;
	char *url = NULL;

	url = getenv(SPIDER_PANEL_URL);
	spider_dbg("URL=%s\n", url);

	gdk_set_allowed_backends("wayland");
	gtk_init(&argc, &argv);

	panel_init(&panel);

	set_panelsize(&panel, 4);

	panel.window = gtk_window_new(GTK_WINDOW_TOPLEVEL);

	gtk_window_set_title(GTK_WINDOW(panel.window), "panel");
	gtk_window_set_default_size(GTK_WINDOW(panel.window), panel.panel_width, panel.panel_height);
	gtk_window_set_decorated(GTK_WINDOW(panel.window), FALSE);
	//gtk_window_fullscreen(GTK_WINDOW(window));
	gtk_widget_realize(panel.window);

	panel.panel = WEBKIT_WEB_VIEW(webkit_web_view_new());
	gtk_container_add(GTK_CONTAINER(panel.window), GTK_WIDGET(panel.panel));

	g_signal_connect(panel.window, "map", G_CALLBACK(map_win_cb), &panel);
	g_signal_connect(panel.window, "destroy", G_CALLBACK(destroy_win_cb), NULL);

	/*
	gdk_window = gtk_widget_get_window(window);
	panel.surface = gdk_wayland_window_get_wl_surface(gdk_window);
	*/

	webkit_web_view_load_uri(panel.panel, url);

	gtk_widget_set_app_paintable (panel.window, TRUE);

	gdk_window = gtk_widget_get_window(panel.window);
	panel.surface = gdk_wayland_window_get_wl_surface(gdk_window);

	gtk_widget_show_all(panel.window);

	gtk_main();

	return 0;
}
