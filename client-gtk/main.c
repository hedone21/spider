/*
 * Copyright (c) 2019 Minyoung.Go <hedone21@gmail.com>
 * 
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
#include <webkit2/webkit2.h>
#include "common/log.h"
#include "common/global_vars.h"

static void destroy_win_cb(GtkWidget* widget, GtkWidget* window)
{
	gtk_main_quit();
}

static gboolean close_web_cb(WebKitWebView* webView, GtkWidget* window)
{
	gtk_widget_destroy(window);
	return TRUE;
}

int main(int argc, char* argv[])
{
	GtkWidget *win = NULL;
	WebKitWebView *web = NULL;
	char *url = NULL;

	url = getenv(SPIDER_WEB_URL);
	spider_dbg("URL=%s\n", url);

	gtk_init(&argc, &argv);

	win = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_set_default_size(GTK_WINDOW(win), 1280, 720);
	gtk_window_set_decorated(GTK_WINDOW(win), FALSE);
	gtk_window_fullscreen(GTK_WINDOW(win));

	web = WEBKIT_WEB_VIEW(webkit_web_view_new());

	gtk_container_add(GTK_CONTAINER(win), GTK_WIDGET(web));

	g_signal_connect(win, "destroy", G_CALLBACK(destroy_win_cb), NULL);
	g_signal_connect(web, "close", G_CALLBACK(close_web_cb), win);

	webkit_web_view_load_uri(web, url);

	gtk_widget_grab_focus(GTK_WIDGET(web));

	gtk_widget_show_all(win);

	gtk_main();

	return 0;
}
