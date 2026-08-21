#pragma once

#ifndef __LIBGLACE_MANAGER_H__
#define __LIBGLACE_MANAGER_H__

#include "glace-client-effect.h"
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

/**
 * GlaceManagerCaptureClientCallback:
 * @pixbuf: (transfer full): the captured pixbuf, ownership is transferred to the caller
 *
 * called when a client capture operation completes.
 * the caller must unref the @pixbuf once uneeded to avoid leaks.
 */
typedef void (*GlaceManagerCaptureClientCallback)(GdkPixbuf* pixbuf, gpointer user_data);

struct _GlaceManager {
    GObject parent_instance;
    GlaceManagerPrivate* priv;
};

struct _GlaceManagerClass {
    GObjectClass parent_class;

    // public methods
    void (*capture_client)(GlaceManager* self, GlaceClient* client, gboolean overlay_cursor, GlaceManagerCaptureClientCallback callback, gpointer user_data, GDestroyNotify notify);
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

/**
 * glace_manager_get_client_effect_for_surface:
 * @self: a #GlaceManager
 * @surface: the wayland surface to get an effect handle from
 *
 * Returns: (transfer full) (nullable): A newly allocated #GlaceClientEffect,
 *          or %NULL if creation failed. The caller is responsible
 *          for unreferencing it with g_object_unref() if it is not %NULL.
 */
GlaceClientEffect* glace_manager_get_client_effect_for_surface(GlaceManager* self, struct wl_surface* surface);

/**
 * glace_manager_get_client_effect_for_window:
 * @self: a #GlaceManager
 * @window: the #GdkWindow to get an effect handle from
 *
 * Returns: (transfer full) (nullable): A newly allocated #GlaceClientEffect,
 *          or %NULL if creation failed. The caller is responsible
 *          for unreferencing it with g_object_unref() if it is not %NULL.
 */
GlaceClientEffect* glace_manager_get_client_effect_for_window(GlaceManager* self, GdkWindow* window);

/**
 * glace_manager_capture_client:
 * @self: a #GlaceManager
 * @client: the #GlaceClient instance to capture
 * @overlay_cursor: whether or not to render the cursor on the client, optional and defaults to false
 * @callback: a callback for receiving the rendered snapshot, this callback should be able of receiving a GdkPixbuf where the data resigns
 *
 * try and get a snapshot capture of a client, this only works on hyprland currently.
 */
void glace_manager_capture_client(GlaceManager* self, GlaceClient* client, gboolean overlay_cursor, GlaceManagerCaptureClientCallback callback, gpointer user_data, GDestroyNotify notify);

G_END_DECLS

#endif /* __LIBGLACE_MANAGER_H__ */
