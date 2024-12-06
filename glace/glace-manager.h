#pragma once

#ifndef __LIBGLACE_MANAGER_H__
#define __LIBGLACE_MANAGER_H__

#include "glace-client.h"

G_BEGIN_DECLS

// GlaceManager
#define GLACE_TYPE_MANAGER (glace_manager_get_type())
// G_DECLARE_DERIVABLE_TYPE(GlaceManager, glace_manager, GLACE, MANAGER, GObject)
#define GLACE_MANAGER(obj) (G_TYPE_CHECK_INSTANCE_CAST((obj), GLACE_TYPE_MANAGER, GlaceManager))
#define GLACE_MANAGER_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST((klass), GLACE_TYPE_MANAGER, GlaceManagerClass))
#define GLACE_IS_MANAGER(obj) (G_TYPE_CHECK_INSTANCE_TYPE((obj), GLACE_TYPE_MANAGER))
#define GLACE_IS_MANAGER_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE((klass), GLACE_TYPE_MANAGER))
#define GLACE_MANAGER_GET_CLASS(obj) (G_TYPE_INSTANCE_GET_CLASS((obj), GLACE_TYPE_MANAGER, GlaceManagerClass))

typedef struct _GlaceManager GlaceManager;
typedef struct _GlaceManagerPrivate GlaceManagerPrivate;
typedef struct _GlaceManagerClass GlaceManagerClass;
typedef void (*GlaceManagerCaptureClientCallback)(GdkPixbuf* pixbuf, gpointer user_data);

struct _GlaceManager {
    GObject parent_instance;
    GlaceManagerPrivate* priv;
};

struct _GlaceManagerClass {
    GObjectClass parent_class;

    // methods
    void (*capture_client)(GlaceManager* self, GlaceClient* client, gboolean overlay_cursor, GlaceManagerCaptureClientCallback callback, gpointer user_data, GDestroyNotify notify);
};

struct _GlaceManagerPrivate {
    GdkWaylandDisplay* gdk_display;
    struct wl_display* display;
    struct wl_shm* wl_shm;
    struct zwlr_foreign_toplevel_manager_v1* wlr_manager;
    struct hyprland_toplevel_export_manager_v1* hl_export_manager;
};

enum {
    GLACE_MANAGER_SIGNAL_CHANGED,
    GLACE_MANAGER_SIGNAL_CLIENT_ADDED,
    GLACE_MANAGER_SIGNAL_CLIENT_REMOVED,
    GLACE_MANAGER_N_SIGNALS
};

// methods
GType glace_manager_get_type();
GlaceManager* glace_manager_new();
void glace_manager_capture_client(GlaceManager* self, GlaceClient* client, gboolean overlay_cursor, GlaceManagerCaptureClientCallback callback, gpointer user_data, GDestroyNotify notify);

G_END_DECLS

#endif /* __LIBGLACE_MANAGER_H__ */
