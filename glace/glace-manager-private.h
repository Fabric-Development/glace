#pragma once

#ifndef __LIBGLACE_MANAGER_PRIVATE_H__
#define __LIBGLACE_MANAGER_PRIVATE_H__

#include "glace-client-private.h"
#include "glace-client.h"
#include "glace-manager.h"

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

typedef struct _GlaceFrameData GlaceFrameData;
typedef struct _GlaceFrameBuffer GlaceFrameBuffer;

struct _GlaceFrameBuffer {
    struct wl_buffer* wl_buffer;
    void* raw_buffer;

    size_t size;
    uint32_t width;
    uint32_t height;
    uint32_t stride;
    uint32_t format;
};

struct _GlaceFrameData {
    GlaceManager* manager;
    GlaceClient* client;

    GlaceFrameBuffer* buffer;

    GlaceManagerCaptureClientCallback callback;
    gpointer callback_data;
};

const static struct hyprland_toplevel_export_frame_v1_listener export_manager_frame_listener;

// static void glace_manager_init(GlaceManager* self);
// static void glace_manager_class_init(GlaceManagerClass* klass);

static void glace_manager_signal_changed_emit(GlaceManager* self);
static void glace_manager_signal_client_added_emit(GlaceManager* self, GlaceClient* client);
static void glace_manager_signal_client_removed_emit(GlaceManager* self, GlaceClient* client);

#endif /* __LIBGLACE_MANAGER_PRIVATE_H__ */
