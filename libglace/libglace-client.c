#include "libglace-private.h"

// client getters
guint glace_client_get_id(GlaceClient* self) {
    return (guint)self->priv->id;
}

const gchar* glace_client_get_app_id(GlaceClient* self) {
    return CLIENT_GET_CURRENT_PROP(self, app_id);
}

const gchar* glace_client_get_title(GlaceClient* self) {
    return CLIENT_GET_CURRENT_PROP(self, title);
}

gboolean glace_client_get_maximized(GlaceClient* self) {
    return CLIENT_GET_CURRENT_PROP(self, maximized);
}

gboolean glace_client_get_minimized(GlaceClient* self) {
    return CLIENT_GET_CURRENT_PROP(self, minimized);
}

gboolean glace_client_get_activated(GlaceClient* self) {
    return CLIENT_GET_CURRENT_PROP(self, activated);
}

gboolean glace_client_get_fullscreen(GlaceClient* self) {
    return CLIENT_GET_CURRENT_PROP(self, fullscreen);
}

gboolean glace_client_get_closed(GlaceClient* self) {
    return self->priv->closed;
}


// client methods
void glace_client_maximize(GlaceClient* self) {
    // replacing EMPTY_TOKEN with a trailing comma
    // will do the trick as well
    RETURN_IF_INVALID_CLIENT(self, EMPTY_TOKEN);

    zwlr_foreign_toplevel_handle_v1_set_maximized(self->priv->wlr_handle);
}


void glace_client_unmaximize(GlaceClient* self) {
    RETURN_IF_INVALID_CLIENT(self, EMPTY_TOKEN);

    zwlr_foreign_toplevel_handle_v1_unset_maximized(self->priv->wlr_handle);
}


void glace_client_minimize(GlaceClient* self) {
    RETURN_IF_INVALID_CLIENT(self, EMPTY_TOKEN);

    zwlr_foreign_toplevel_handle_v1_set_minimized(self->priv->wlr_handle);
}


void glace_client_unminimize(GlaceClient* self) {
    RETURN_IF_INVALID_CLIENT(self, EMPTY_TOKEN);

    zwlr_foreign_toplevel_handle_v1_unset_minimized(self->priv->wlr_handle);
}


void glace_client_close(GlaceClient* self) {
    RETURN_IF_INVALID_CLIENT(self, EMPTY_TOKEN);

    zwlr_foreign_toplevel_handle_v1_close(self->priv->wlr_handle);
}


void glace_client_activate(GlaceClient* self) {
    RETURN_IF_INVALID_CLIENT(self, EMPTY_TOKEN);

    GdkSeat* gdk_seat = gdk_display_get_default_seat(self->priv->gdk_display);
    struct wl_seat* seat = gdk_wayland_seat_get_wl_seat(gdk_seat);

    zwlr_foreign_toplevel_handle_v1_activate(
        self->priv->wlr_handle,
        seat
    );
}


void glace_client_move(
    GlaceClient* self,
    GdkWindow* window,
    const GdkRectangle* rectangle
) {
    RETURN_IF_INVALID_CLIENT(self, EMPTY_TOKEN);

    if (window == NULL) {
        zwlr_foreign_toplevel_handle_v1_set_rectangle(
            self->priv->wlr_handle,
            NULL,
            rectangle->x,
            rectangle->y,
            rectangle->width,
            rectangle->height
        );
        return;
    }

    zwlr_foreign_toplevel_handle_v1_set_rectangle(
        self->priv->wlr_handle,
        gdk_wayland_window_get_wl_surface(window),
        rectangle->x,
        rectangle->y,
        rectangle->width,
        rectangle->height
    );
}


void glace_client_fullscreen(GlaceClient* self) {
    RETURN_IF_INVALID_CLIENT(self, EMPTY_TOKEN);

    zwlr_foreign_toplevel_handle_v1_set_fullscreen(self->priv->wlr_handle, self->priv->output);
}


void glace_client_unfullscreen(GlaceClient* self) {
    RETURN_IF_INVALID_CLIENT(self, EMPTY_TOKEN);

    zwlr_foreign_toplevel_handle_v1_unset_fullscreen(self->priv->wlr_handle);
}
