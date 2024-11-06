#ifndef __LIBGLACE_MANAGER_PRIVATE_H__
#define __LIBGLACE_MANAGER_PRIVATE_H__

#include "libglace-client-private.h"
#include "libglace-manager.h"
#include "wlr-foreign-toplevel-management-unstable-v1.h"

#define max(a, b) (a > b ? a : b)
#define min(x, y) ((x) < (y) ? (x) : (y))

#define EMPTY_TOKEN

#define DISPLAY_CHECK_NULL(expr)                                                                                                    \
    do {                                                                                                                            \
        if G_LIKELY ((expr) != NULL)                                                                                                \
            ;                                                                                                                       \
        else                                                                                                                        \
            g_error("[ERROR] '" #expr "' should NOT be NULL, please note that Glace can't work under anything other than Wayland"); \
        g_assert_nonnull(expr);                                                                                                     \
    } while (0)

#define G_LIST_FOREACH(item, list) for (GList* __glist = list; __glist && (item = __glist->data, true); __glist = __glist->next)

// static void glace_manager_init(GlaceManager* self);
// static void glace_manager_class_init(GlaceManagerClass* klass);

static void glace_manager_signal_changed_emit(GlaceManager* self);
static void glace_manager_signal_client_added_emit(GlaceManager* self, GlaceClient* client);
static void glace_manager_signal_client_removed_emit(GlaceManager* self, GlaceClient* client);

#endif /* __LIBGLACE_MANAGER_PRIVATE_H__ */
