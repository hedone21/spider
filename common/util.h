/*
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

#ifndef __SPIDER_COMMON_UTIL_H__
#define __SPIDER_COMMON_UTIL_H__

#include <wayland-server.h>

/* Wrapper code of wl_list */
#define spider_list wl_list
#define spider_list_init wl_list_init
#define spider_list_insert wl_list_insert
#define spider_list_remove wl_list_remove
#define spider_list_length wl_list_length
#define spider_list_empty wl_list_empty
#define spider_list_insert_list wl_list_insert_list
#define spider_list_for_each wl_list_for_each
#define spider_list_for_each_safe wl_list_for_each_safe
#define spider_list_for_each_reverse wl_list_for_each_reverse
#define spider_list_for_each_reverse_safe wl_list_for_each_reverse_safe

void spider_list_insert_tail(struct wl_list *list, struct wl_list *elm);

#endif
