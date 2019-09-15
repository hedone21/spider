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

#include <gtk/gtk.h>
#include "shell/webkitapi.h"
#include "common/command.c"
#include "common/log.h"

/* webkit decide policy callback */
void wkapi_dp_cb(WebKitWebView *webview, WebKitPolicyDecision *decision, WebKitPolicyDecisionType type)
{
	WebKitNavigationPolicyDecision *navigation_decision = NULL;
	WebKitNavigationAction *navigation_action = NULL;
	WebKitURIRequest *uri_request = NULL;
	WebKitResponsePolicyDecision *response = NULL;
	const char *uri;
	const char *http_method;
	spider_dbg("type=%d\n", type);

	switch (type) {
	case WEBKIT_POLICY_DECISION_TYPE_NAVIGATION_ACTION: 
		navigation_decision = WEBKIT_NAVIGATION_POLICY_DECISION(decision);
		navigation_action = webkit_navigation_policy_decision_get_navigation_action(navigation_decision);
		uri_request = webkit_navigation_action_get_request(navigation_action);
		uri = webkit_uri_request_get_uri(uri_request);
		http_method = webkit_uri_request_get_http_method(uri_request);
		spider_dbg("url=%s http_method=%s\n", uri, http_method);
		/* Make a policy decision here. */
		break;
	case WEBKIT_POLICY_DECISION_TYPE_NEW_WINDOW_ACTION:
		navigation_decision = WEBKIT_NAVIGATION_POLICY_DECISION(decision);
		/* Make a policy decision here. */
		break;
	case WEBKIT_POLICY_DECISION_TYPE_RESPONSE:
		response = WEBKIT_RESPONSE_POLICY_DECISION(decision);
		/* Make a policy decision here. */
		break;
	default:
		/* Making no decision results in webkit_policy_decision_use(). */
		return;
	}
}

void wkapi_sf_cb(WebKitWebView *webviewm, WebKitFormSubmissionRequest *request, gpointer data)
{
	GPtrArray *field_names;
	GPtrArray *field_commands;

	webkit_form_submission_request_list_text_fields(request, &field_names, &field_commands);
	spider_dbg("\n");

	for (gsize i = 0; i < field_names->len; i++) {
		const char *name = g_ptr_array_index(field_names, i);
		const char *command = g_ptr_array_index(field_commands, i);
		spider_dbg("name=%s command=%s\n", name, command);
		command_launch(command);
	}
}

void wkapi_close_cb(WebKitWebView *webView, GtkWidget *window)
{
	gtk_widget_destroy(window);
	return TRUE;
}

