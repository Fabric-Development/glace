#pragma once

#ifndef __LIBGLACE_CLIENT_H__
#define __LIBGLACE_CLIENT_H__

#include <assert.h>
#include <gdk/gdkwayland.h>
#include <glib-object.h>
#include <gtk-3.0/gtk/gtk.h>
#include <stdbool.h>
#include <string.h>

G_BEGIN_DECLS

#define GLACE_TYPE_CLIENT (glace_client_get_type())
// G_DECLARE_DERIVABLE_TYPE(GlaceClient, glace_client, GLACE, CLIENT, GObject)
#define GLACE_CLIENT(obj) (G_TYPE_CHECK_INSTANCE_CAST((obj), GLACE_TYPE_CLIENT, GlaceClient))
#define GLACE_CLIENT_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST((klass), GLACE_TYPE_CLIENT, GlaceClientClass))
#define GLACE_IS_CLIENT(obj) (G_TYPE_CHECK_INSTANCE_TYPE((obj), GLACE_TYPE_CLIENT))
#define GLACE_IS_CLIENT_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE((klass), GLACE_TYPE_CLIENT))
#define GLACE_CLIENT_GET_CLASS(obj) (G_TYPE_INSTANCE_GET_CLASS((obj), GLACE_TYPE_CLIENT, GlaceClientClass))

typedef struct _GlaceClient GlaceClient;
typedef struct _GlaceClientClass GlaceClientClass;
typedef struct _GlaceClientPrivate GlaceClientPrivate;

/**
 * GlaceClientProperties: (skip)
 */
typedef struct _GlaceClientProperties GlaceClientProperties;

struct _GlaceClient {
    GObject parent_instance;
    GlaceClientPrivate* priv;
};

struct _GlaceClientClass {
    GObjectClass parent_class;

    // _class_ methods
    void (*activate)(GlaceClient* self);
    void (*maximize)(GlaceClient* self);
    void (*minimize)(GlaceClient* self);
    void (*fullscreen)(GlaceClient* self);
    void (*unmaximize)(GlaceClient* self);
    void (*unminimize)(GlaceClient* self);
    void (*unfullscreen)(GlaceClient* self);
    void (*move)(GlaceClient* self, GdkWindow* window, const GdkRectangle* rectangle);
    void (*close)(GlaceClient* self);
};

/**
 * _GlaceClientProperties: (skip)
 */
struct _GlaceClientProperties {
    gchar* app_id;
    gchar* title;
    gboolean maximized;
    gboolean minimized;
    gboolean activated;
    gboolean fullscreen;
};

struct _GlaceClientPrivate {
    uint32_t id;
    bool closed;
    struct wl_output* output;
    struct zwlr_foreign_toplevel_handle_v1* wlr_handle;
    GdkWaylandDisplay* gdk_display;
    GlaceClientProperties current_properties;
    GlaceClientProperties pending_properties;
};

enum {
    GLACE_CLIENT_SIGNAL_CHANGED,
    GLACE_CLIENT_SIGNAL_CLOSE,
    GLACE_CLIENT_N_SIGNALS
};

enum {
    GLACE_CLIENT_PROPERTY_0,
    GLACE_CLIENT_PROPERTY_ID,
    GLACE_CLIENT_PROPERTY_APP_ID,
    GLACE_CLIENT_PROPERTY_TITLE,
    GLACE_CLIENT_PROPERTY_MAXIMIZED,
    GLACE_CLIENT_PROPERTY_MINIMIZED,
    GLACE_CLIENT_PROPERTY_ACTIVATED,
    GLACE_CLIENT_PROPERTY_FULLSCREEN,
    GLACE_CLIENT_PROPERTY_CLOSED,
    GLACE_CLIENT_N_PROPERTIES
};

// property getters
guint glace_client_get_id(GlaceClient* self);
const gchar* glace_client_get_app_id(GlaceClient* self);
const gchar* glace_client_get_title(GlaceClient* self);
gboolean glace_client_get_maximized(GlaceClient* self);
gboolean glace_client_get_minimized(GlaceClient* self);
gboolean glace_client_get_activated(GlaceClient* self);
gboolean glace_client_get_fullscreen(GlaceClient* self);
gboolean glace_client_get_closed(GlaceClient* self);

// methods
GType glace_client_get_type();

/**
 * glace_client_maximize:
 * @self: a #GlaceClient
 *
 * to maximize a client (AKA make it to fill it's display area)
 */
void glace_client_maximize(GlaceClient* self);

/**
 * glace_client_unmaximize:
 * @self: a #GlaceClient
 *
 * to unset a client from the maximized state
 */
void glace_client_unmaximize(GlaceClient* self);

/**
 * glace_client_minimize:
 * @self: a #GlaceClient
 *
 * to minimize a client (AKA hide it to taskbar)
 */
void glace_client_minimize(GlaceClient* self);

/**
 * glace_client_unminimize:
 * @self: a #GlaceClient
 *
 * to unset a client from the minimized state
 */
void glace_client_unminimize(GlaceClient* self);

/**
 * glace_client_close:
 * @self: a #GlaceClient
 *
 * to close a client
 * the `closed` property will get changed if the request was done successfully
 * it's guaranteed that you won't receive events for this client after it gets closed
 */
void glace_client_close(GlaceClient* self);

/**
 * glace_client_activate:
 * @self: a #GlaceClient
 *
 * to activate a client (AKA focus it)
 * the `activated` property will get changed if the request was done successfully
 */
void glace_client_activate(GlaceClient* self);

/**
 * glace_client_move:
 * @self: a #GlaceClient
 * @window: (nullable): the #GdkWindow to use it's surface as a hint for the compositor, you can pass NULL for this
 * @rectangle: the #GdkRectangle to use
 *
 * move this client to a X and Y coords / width and height using a rectangle
 * it's not guaranteed if the client will actually get moved or not
 * check if your target compositor(s) supports this feature or not
 */
void glace_client_move(GlaceClient* self, GdkWindow* window, const GdkRectangle* rectangle);

/**
 * glace_client_fullscreen:
 * @self: a #GlaceClient
 *
 * to get a client in a fullscreen state
 * it will be ignore if the client is in a fullscreen state
 * the `fullscreen` property will get changed if the request was done successfully
 */
void glace_client_fullscreen(GlaceClient* self);

/**
 * glace_client_unfullscreen:
 * @self: a #GlaceClient
 *
 * to get a client out of fullscreen state
 * it will be ignore if the client is not in a fullscreen state
 * the `fullscreen` property will get changed if the request was done successfully
 */
void glace_client_unfullscreen(GlaceClient* self);

G_END_DECLS

#endif /* __LIBGLACE_CLIENT_H__ */
