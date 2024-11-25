#include "glace-private.h"

static guint glace_client_signals[GLACE_CLIENT_N_SIGNALS] = {0};
static GParamSpec* glace_client_properties[GLACE_CLIENT_N_PROPERTIES] = {
    NULL,
};

static void glace_client_signal_changed_emit(GlaceClient* self) {
    g_signal_emit(
        self,
        glace_client_signals[GLACE_CLIENT_SIGNAL_CHANGED],
        0
    );
}

static void glace_client_signal_close_emit(GlaceClient* self) {
    g_signal_emit(
        self,
        glace_client_signals[GLACE_CLIENT_SIGNAL_CLOSE],
        0
    );
}

static void glace_client_get_property(
    GObject* object,
    guint prop_id,
    GValue* value,
    GParamSpec* pspec
) {
    GlaceClient* self = GLACE_CLIENT(object);

    switch (prop_id) {
    case GLACE_CLIENT_PROPERTY_ID:
        g_value_set_uint(value, (guint)self->priv->id);
        break;
    case GLACE_CLIENT_PROPERTY_APP_ID:
        g_value_set_string(value, CLIENT_GET_CURRENT_PROP(self, app_id));
        break;
    case GLACE_CLIENT_PROPERTY_TITLE:
        g_value_set_string(value, CLIENT_GET_CURRENT_PROP(self, title));
        break;
    case GLACE_CLIENT_PROPERTY_MAXIMIZED:
        g_value_set_boolean(value, CLIENT_GET_CURRENT_PROP(self, maximized));
        break;
    case GLACE_CLIENT_PROPERTY_MINIMIZED:
        g_value_set_boolean(value, CLIENT_GET_CURRENT_PROP(self, minimized));
        break;
    case GLACE_CLIENT_PROPERTY_ACTIVATED:
        g_value_set_boolean(value, CLIENT_GET_CURRENT_PROP(self, activated));
        break;
    case GLACE_CLIENT_PROPERTY_FULLSCREEN:
        g_value_set_boolean(value, CLIENT_GET_CURRENT_PROP(self, fullscreen));
        break;
    case GLACE_CLIENT_PROPERTY_CLOSED:
        g_value_set_boolean(value, self->priv->closed);
        break;
    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
        break;
    }
}

// client event handlers
static void on_toplevel_handle_title(
    void* data,
    struct zwlr_foreign_toplevel_handle_v1* wlr_handle,
    const char* title
) {
    GlaceClient* self = data;

    CLIENT_SET_PENDING_PROP(self, title, (gchar*)(strdup(title)));
}

static void on_toplevel_handle_app_id(
    void* data,
    struct zwlr_foreign_toplevel_handle_v1* wlr_handle,
    const char* app_id
) {
    GlaceClient* self = data;

    CLIENT_SET_PENDING_PROP(self, app_id, (gchar*)(strdup(app_id)));
}

// TODO: implement those
static void on_toplevel_handle_output_enter(
    void* data,
    struct zwlr_foreign_toplevel_handle_v1* wlr_handle,
    struct wl_output* output
) {}

static void on_toplevel_handle_output_leave(
    void* data,
    struct zwlr_foreign_toplevel_handle_v1* wlr_handle,
    struct wl_output* output
) {}

static void on_toplevel_handle_state(
    void* data,
    struct zwlr_foreign_toplevel_handle_v1* wlr_handle,
    struct wl_array* states
) {
    GlaceClient* self = data;

    CLIENT_SET_DEFAULT_STATES(self);

    enum zwlr_foreign_toplevel_handle_v1_state* state;
    wl_array_for_each(state, states) {
        switch (*state) {
            CLIENT_SET_STATE_FOR_CASE(self, state, MAXIMIZED, maximized);
            CLIENT_SET_STATE_FOR_CASE(self, state, MINIMIZED, minimized);
            CLIENT_SET_STATE_FOR_CASE(self, state, ACTIVATED, activated);
            CLIENT_SET_STATE_FOR_CASE(self, state, FULLSCREEN, fullscreen);
        }
    }
}

static void on_toplevel_handle_done(
    void* data,
    struct zwlr_foreign_toplevel_handle_v1* wlr_handle
) {
    GlaceClient* self = data;
    IF_INVALID_CLIENT(self) {
        // already finalized client
        return;
    }

    if (CLIENT_GET_CURRENT_PROP(self, title) && CLIENT_GET_PENDING_PROP(self, title)) {
        free(CLIENT_GET_CURRENT_PROP(self, title));
    }

    if (CLIENT_GET_CURRENT_PROP(self, app_id) && CLIENT_GET_PENDING_PROP(self, app_id)) {
        free(CLIENT_GET_CURRENT_PROP(self, app_id));
    }

    if (CLIENT_GET_PENDING_PROP(self, title)) {
        CLIENT_BRIDGE_PROPS(self, title, TITLE);
        CLIENT_SET_PENDING_PROP(self, title, NULL);
    }

    if (CLIENT_GET_PENDING_PROP(self, app_id)) {
        CLIENT_BRIDGE_PROPS(self, app_id, APP_ID);
        CLIENT_SET_PENDING_PROP(self, app_id, NULL);
    }

    // state properties
    CLIENT_BRIDGE_PROPS(self, maximized, MAXIMIZED);
    CLIENT_BRIDGE_PROPS(self, minimized, MINIMIZED);
    CLIENT_BRIDGE_PROPS(self, activated, ACTIVATED);
    CLIENT_BRIDGE_PROPS(self, fullscreen, FULLSCREEN);

    glace_client_signal_changed_emit(self);
}

static void on_toplevel_handle_closed(
    void* data,
    struct zwlr_foreign_toplevel_handle_v1* wlr_handle
) {
    GlaceClient* self = data;

    self->priv->closed = true;

    glace_client_signal_close_emit(self);
}

static void on_toplevel_handle_parent(
    void* data,
    struct zwlr_foreign_toplevel_handle_v1* wlr_handle,
    struct zwlr_foreign_toplevel_handle_v1* parent
) {}

static struct zwlr_foreign_toplevel_handle_v1_listener toplevel_handle_listener = {
    .title = &on_toplevel_handle_title,
    .app_id = &on_toplevel_handle_app_id,
    .output_enter = &on_toplevel_handle_output_enter,
    .output_leave = &on_toplevel_handle_output_leave,
    .state = &on_toplevel_handle_state,
    .done = &on_toplevel_handle_done,
    .closed = &on_toplevel_handle_closed,
    .parent = &on_toplevel_handle_parent
};

static void glace_client_class_init(GlaceClientClass* klass) {
    GObjectClass* parent_class = G_OBJECT_CLASS(klass);

    g_type_class_add_private(klass, sizeof(GlaceClientPrivate));

    // overrides
    parent_class->get_property = glace_client_get_property;

    // add methods
    klass->maximize = glace_client_maximize;
    klass->unmaximize = glace_client_unmaximize;
    klass->minimize = glace_client_minimize;
    klass->unminimize = glace_client_unminimize;
    klass->close = glace_client_close;
    klass->activate = glace_client_activate;
    klass->move = glace_client_move;
    klass->fullscreen = glace_client_fullscreen;
    klass->unfullscreen = glace_client_unfullscreen;

    glace_client_signals[GLACE_CLIENT_SIGNAL_CHANGED] = g_signal_new(
        "changed",
        GLACE_TYPE_CLIENT,
        G_SIGNAL_RUN_LAST | G_SIGNAL_DETAILED,
        0,
        NULL,
        NULL,
        NULL,
        G_TYPE_NONE,
        0
    );

    glace_client_signals[GLACE_CLIENT_SIGNAL_CLOSE] = g_signal_new(
        "close",
        GLACE_TYPE_CLIENT,
        G_SIGNAL_RUN_LAST | G_SIGNAL_DETAILED,
        0,
        NULL,
        NULL,
        NULL,
        G_TYPE_NONE,
        0
    );

    glace_client_properties[GLACE_CLIENT_PROPERTY_ID] = g_param_spec_uint(
        "id",
        "id",
        "the id of the client",
        0,
        UINT32_MAX,
        0,
        G_PARAM_READABLE
    );

    glace_client_properties[GLACE_CLIENT_PROPERTY_APP_ID] = g_param_spec_string(
        "app-id",
        "class",
        "the application id of the client (class name under X11)",
        NULL,
        G_PARAM_READABLE
    );

    glace_client_properties[GLACE_CLIENT_PROPERTY_TITLE] = g_param_spec_string(
        "title",
        "title",
        "the current title of the client",
        NULL,
        G_PARAM_READABLE
    );

    // state properties
    glace_client_properties[GLACE_CLIENT_PROPERTY_MAXIMIZED] = g_param_spec_boolean(
        "maximized",
        "maximized",
        "whether this client is currently maximized or not",
        false,
        G_PARAM_READABLE
    );

    glace_client_properties[GLACE_CLIENT_PROPERTY_MINIMIZED] = g_param_spec_boolean(
        "minimized",
        "minimized",
        "whether this client is currently minimized or not",
        false,
        G_PARAM_READABLE
    );

    glace_client_properties[GLACE_CLIENT_PROPERTY_ACTIVATED] = g_param_spec_boolean(
        "activated",
        "focused",
        "whether this client is currently activated (focused) or not",
        false,
        G_PARAM_READABLE
    );

    glace_client_properties[GLACE_CLIENT_PROPERTY_FULLSCREEN] = g_param_spec_boolean(
        "fullscreen", "fullscreen", "whether this client is currently in a fullscreen state or not", false, G_PARAM_READABLE
    );

    glace_client_properties[GLACE_CLIENT_PROPERTY_CLOSED] = g_param_spec_boolean(
        "closed",
        "closed",
        "whether this client is closed (killed) or not, it's guaranteed that you won't receive events for this client after it gets closed",
        false,
        G_PARAM_READABLE
    );

    g_object_class_install_properties(
        parent_class,
        GLACE_CLIENT_N_PROPERTIES,
        glace_client_properties
    );
}

static void glace_client_init(GlaceClient* self) {
    self->priv = G_TYPE_INSTANCE_GET_PRIVATE(
        self,
        GLACE_TYPE_CLIENT,
        GlaceClientPrivate
    );

    self->priv->closed = false;
    CLIENT_SET_CURRENT_PROP(self, app_id, false);
    CLIENT_SET_CURRENT_PROP(self, title, false);
    CLIENT_SET_CURRENT_PROP(self, maximized, false);
    CLIENT_SET_CURRENT_PROP(self, minimized, false);
    CLIENT_SET_CURRENT_PROP(self, activated, false);
    CLIENT_SET_CURRENT_PROP(self, fullscreen, false);
}

GlaceClient* glace_client_new(
    struct zwlr_foreign_toplevel_handle_v1* wlr_handle,
    GdkWaylandDisplay* gdk_display
) {
    GlaceClient* self = g_object_new(GLACE_TYPE_CLIENT, NULL);
    self->priv->id = wl_proxy_get_id((struct wl_proxy*)wlr_handle);
    self->priv->wlr_handle = wlr_handle;
    self->priv->gdk_display = gdk_display;
    zwlr_foreign_toplevel_handle_v1_add_listener(
        self->priv->wlr_handle,
        &toplevel_handle_listener,
        self
    );
    return self;
}

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
G_DEFINE_TYPE(GlaceClient, glace_client, G_TYPE_OBJECT);
