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
#include "common/log.h"
#include "common/global_vars.h"

static void set_panelsize(struct spider_panel *panel, int portion)
{
	GdkScreen *screen = gdk_screen_get_default ();

	panel->panel_width = gdk_screen_get_width(screen);
	panel->panel_height = gdk_screen_get_height(screen) * portion / 100;
}

static void draw_win_cb(GtkWidget* widget, cairo_t *cr, gpointer data)
{
	double x, y, w, h;
	cairo_clip_extents(cr, &x, &y, &w, &h);
	cairo_set_source_rgba (cr, 0., 0., 0., 0.25); //translucent red
	cairo_set_operator(cr, CAIRO_OPERATOR_SOURCE);
	cairo_rectangle(cr, x, y, w, h);
	cairo_fill(cr);
}

static void destroy_win_cb(GtkWidget* widget, GtkWidget* window)
{
	gtk_main_quit();
}

int main(int argc, char *argv[])
{
	struct spider_panel panel;
	GdkWindow *gdk_window;

	gdk_set_allowed_backends("wayland");
	gtk_init(&argc, &argv);

	panel_init(&panel);

	set_panelsize(&panel, 4);

	panel.window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	panel.panel = gtk_layout_new(NULL, NULL);

	gtk_window_set_title(GTK_WINDOW(panel.window), "panel");
	gtk_window_set_default_size(GTK_WINDOW(panel.window), panel.panel_width, panel.panel_height);
	gtk_window_set_decorated(GTK_WINDOW(panel.window), FALSE);
	//gtk_window_fullscreen(GTK_WINDOW(window));
	gtk_widget_realize(panel.window);

	g_signal_connect(panel.window, "destroy", G_CALLBACK(destroy_win_cb), NULL);
	g_signal_connect(panel.panel, "draw", G_CALLBACK(draw_win_cb), &panel);

	/*
	gdk_window = gtk_widget_get_window(window);
	panel.surface = gdk_wayland_window_get_wl_surface(gdk_window);
	*/

	gtk_container_add(GTK_CONTAINER(panel.window), panel.panel);

	gtk_widget_set_app_paintable (panel.window, TRUE);

	gtk_widget_show_all(panel.window);

	/* TODO: do busy waiting for loading spider-shell */
	sleep(2);

	gtk_main();

	return 0;
}
