#include "libglace.h"
#include <wayland-client.h>
#include "wlr-foreign-toplevel-management-unstable-v1.h"

#define max(a, b) (a > b ? a : b)
#define min(x, y) ((x) < (y) ? (x) : (y))

#define EMPTY_TOKEN

#define CLIENT_SET_CURRENT_PROP(client, prop, value) \
    (client->priv->current_properties.prop = value)

#define CLIENT_SET_PENDING_PROP(client, prop, value) \
    (client->priv->pending_properties.prop = value)

#define CLIENT_GET_CURRENT_PROP(client, prop) \
    (client->priv->current_properties.prop)

#define CLIENT_GET_PENDING_PROP(client, prop) \
    (client->priv->pending_properties.prop)

#define CLIENT_BRIDGE_PROPS(client, prop, PROP_UPPER) do { \
    if (CLIENT_GET_CURRENT_PROP(client, prop) != CLIENT_GET_PENDING_PROP(client, prop)) { \
        CLIENT_GET_CURRENT_PROP(client, prop) = CLIENT_GET_PENDING_PROP(client, prop); \
        g_object_notify_by_pspec( \
            G_OBJECT(client), \
            glace_client_properties[GLACE_CLIENT_PROPERTY_##PROP_UPPER] \
        ); \
    } \
} while (0)

#define CLIENT_COMMIT_STATE_PROP(client, state, enum_m, prop) do { \
    if (*state == ZWLR_FOREIGN_TOPLEVEL_HANDLE_V1_STATE_##enum_m) { \
        CLIENT_SET_PENDING_PROP(client, prop, true); \
    } else { \
        CLIENT_SET_PENDING_PROP(client, prop, false); \
    } \
} while (0)

#define IF_INVALID_CLIENT(client) if (client == NULL || GLACE_IS_CLIENT(client) == false || client->priv->closed == true)

#define RETURN_IF_INVALID_CLIENT(client, r_value) do { \
    IF_INVALID_CLIENT(client) {\
        g_warning("[WARNING][CLIENT] function %s got an invalid client", __func__);\
        return r_value; \
    } \
} while (0)

#define DISPLAY_CHECK_NULL(expr) do { \
    if G_LIKELY((expr) != NULL); else \
    g_error("[ERROR] '"#expr"' should NOT be NULL, please note that Glace can't work under anything other than Wayland"); \
    g_assert_nonnull(expr); \
} while (0)

#define G_LIST_FOREACH(item, list) for (GList *__glist = list; __glist && (item = __glist->data, true); __glist = __glist->next)

G_DEFINE_TYPE(GlaceManager, glace_manager, G_TYPE_OBJECT);
G_DEFINE_TYPE(GlaceClient, glace_client, G_TYPE_OBJECT);

static guint glace_manager_signals[GLACE_MANAGER_N_SIGNALS] = {0};
static guint glace_client_signals[GLACE_CLIENT_N_SIGNALS] = {0};
static GParamSpec* glace_client_properties[GLACE_CLIENT_N_PROPERTIES] = {NULL,};


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

    zwlr_foreign_toplevel_handle_v1_activate(
        self->priv->wlr_handle,
        self->priv->manager->priv->seat
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


static void glace_client_init(GlaceClient* self) {
    self->priv = G_TYPE_INSTANCE_GET_PRIVATE(
        self,
        GLACE_TYPE_CLIENT,
        GlaceClientPrivate
    );

    self->priv->closed = false;
    CLIENT_SET_PENDING_PROP(self, app_id, false);
    CLIENT_SET_PENDING_PROP(self, title, false);
    CLIENT_SET_PENDING_PROP(self, maximized, false);
    CLIENT_SET_PENDING_PROP(self, minimized, false);
    CLIENT_SET_PENDING_PROP(self, activated, false);
    CLIENT_SET_PENDING_PROP(self, fullscreen, false);
}


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

    glace_client_properties[
        GLACE_CLIENT_PROPERTY_ID
    ] = g_param_spec_uint(
        "id",
        "id",
        "the id of the client",
        0,
        UINT32_MAX,
        0,
        G_PARAM_READABLE
    );

    glace_client_properties[
        GLACE_CLIENT_PROPERTY_APP_ID
    ] = g_param_spec_string(
        "app-id",
        "class",
        "the application id of the client (class name under X11)",
        NULL,
        G_PARAM_READABLE
    );

    glace_client_properties[
        GLACE_CLIENT_PROPERTY_TITLE
    ] = g_param_spec_string(
        "title",
        "title",
        "the current title of the client",
        NULL,
        G_PARAM_READABLE
    );

    // state properties
    glace_client_properties[
        GLACE_CLIENT_PROPERTY_MAXIMIZED
    ] = g_param_spec_boolean(
        "maximized",
        "maximized",
        "whether this client is currently maximized or not",
        false,
        G_PARAM_READABLE
    );

    glace_client_properties[
        GLACE_CLIENT_PROPERTY_MINIMIZED
    ] = g_param_spec_boolean(
        "minimized",
        "minimized",
        "whether this client is currently minimized or not",
        false,
        G_PARAM_READABLE
    );

    glace_client_properties[
        GLACE_CLIENT_PROPERTY_ACTIVATED
    ] = g_param_spec_boolean(
        "activated",
        "focused",
        "whether this client is currently activated (focused) or not",
        false,
        G_PARAM_READABLE
    );

    glace_client_properties[
        GLACE_CLIENT_PROPERTY_FULLSCREEN
    ] = g_param_spec_boolean(
        "fullscreen", "fullscreen", "whether this client is currently in a fullscreen state or not",
        false,
        G_PARAM_READABLE
    );

    glace_client_properties[
        GLACE_CLIENT_PROPERTY_CLOSED
    ] = g_param_spec_boolean(
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


static GlaceClient* get_client_for_id(int32_t id, GlaceManager* manager) {
    GList* clients = manager->priv->clients_list; GlaceClient* client;
    G_LIST_FOREACH(client, clients) {
        if (client->priv->id == id) {
            return client;
        }
    }
    g_debug("[WARNING][MANAGER] %s\n IS OUT WITH NULL!", __func__);
    return NULL;
}


static GlaceClient* get_client_for_wlr_handle(struct zwlr_foreign_toplevel_handle_v1* handle, GlaceManager* manager) {
    g_return_if_fail(handle != NULL || manager != NULL);
    return get_client_for_id(
        wl_proxy_get_id((struct wl_proxy*) handle),
        manager
    );
}


static void remove_client(GlaceClient* client, GlaceManager* manager) {
    if (GLACE_IS_CLIENT(client) != true || GLACE_IS_MANAGER(manager) != true) {
        return;
    }

    g_debug("[INFO][MANAGER] removing client with id %u\n", client->priv->id);
    manager->priv->clients_list = g_list_remove(
        manager->priv->clients_list,
        client
    );

    g_signal_emit(
        manager,
        glace_manager_signals[GLACE_MANAGER_SIGNAL_CLIENT_REMOVED],
        1,
        client
    );
}


static void remove_client_for_id(int32_t id, GlaceManager* manager) {
    GlaceClient* client;
    G_LIST_FOREACH(client, manager->priv->clients_list) {
        if (client->priv->id == id) {
            remove_client(client, manager);
            return;
        }
    }

    g_debug(
        "[INFO][MANAGER] (%s) tried to remove client from manager, but couldn't find it",
        __func__
    );
}


static void remove_client_for_wlr_handle(struct zwlr_foreign_toplevel_handle_v1* handle, GlaceManager* manager) {
    g_return_if_fail(handle != NULL || manager != NULL);
    return remove_client_for_id(
        wl_proxy_get_id((struct wl_proxy*) handle),
        manager
    );
}

// client event handlers
static void on_toplevel_handle_title(
    void* data,
    struct zwlr_foreign_toplevel_handle_v1* wlr_handle,
    const char* title
) {
    GlaceManager* manager = data;
    GlaceClient* self = get_client_for_wlr_handle(wlr_handle, manager);

    CLIENT_SET_PENDING_PROP(self, title, (gchar*)(strdup(title)));
}


static void on_toplevel_handle_app_id(
    void* data,
    struct zwlr_foreign_toplevel_handle_v1* wlr_handle,
    const char* app_id
) {
    GlaceManager* manager = data;
    GlaceClient* self = get_client_for_wlr_handle(wlr_handle, manager);

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
    GlaceManager* manager = data;
    GlaceClient* self = get_client_for_wlr_handle(wlr_handle, manager);

    uint32_t* state;
    wl_array_for_each(state, states) {
        CLIENT_COMMIT_STATE_PROP(self, state, MAXIMIZED, maximized);
        CLIENT_COMMIT_STATE_PROP(self, state, MINIMIZED, minimized);
        CLIENT_COMMIT_STATE_PROP(self, state, ACTIVATED, activated);
        CLIENT_COMMIT_STATE_PROP(self, state, FULLSCREEN, fullscreen);
    }
}


static void on_toplevel_handle_done(
    void* data,
    struct zwlr_foreign_toplevel_handle_v1* wlr_handle
) {
    GlaceManager* manager = data;
    GlaceClient* self = get_client_for_wlr_handle(wlr_handle, manager);

    if (GLACE_IS_CLIENT(self) != true || GLACE_IS_MANAGER(manager) != true) {
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

    g_signal_emit(
        self,
        glace_client_signals[GLACE_CLIENT_SIGNAL_CHANGED],
        0
    );
}


static void on_toplevel_handle_closed(
    void* data,
    struct zwlr_foreign_toplevel_handle_v1* wlr_handle
) {
    GlaceManager* manager = data;
    GlaceClient* self = get_client_for_wlr_handle(wlr_handle, manager);

    remove_client(self, manager);

    self->priv->closed = true;

    g_signal_emit(
        self,
        glace_client_signals[GLACE_CLIENT_SIGNAL_CLOSE],
        0
    );
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


static void on_manager_toplevel(
    void* data,
    struct zwlr_foreign_toplevel_manager_v1* manager,
    struct zwlr_foreign_toplevel_handle_v1* handle
) {
    GlaceManager* self = data;
    GlaceClient* client = g_object_new(GLACE_TYPE_CLIENT, NULL);

    if (GLACE_IS_MANAGER(self) != true) {
        return;
    }


    client->priv->id = wl_proxy_get_id((struct wl_proxy*)handle);
    client->priv->manager = self;
    client->priv->wlr_handle = handle;

    RETURN_IF_INVALID_CLIENT(client, EMPTY_TOKEN);

    self->priv->clients_list = g_list_append(self->priv->clients_list, client);

    g_signal_emit(
        self,
        glace_manager_signals[GLACE_MANAGER_SIGNAL_CLIENT_ADDED],
        1,
        client
    );

    zwlr_foreign_toplevel_handle_v1_add_listener(handle, &toplevel_handle_listener, self);

    g_debug("[INFO][CLIENT] got a handle with id %u\n", client->priv->id);
};


static void on_manager_finished(
    void* data,
    struct zwlr_foreign_toplevel_manager_v1* manager
) {}


static const struct zwlr_foreign_toplevel_manager_v1_listener toplevel_manager_listener = {
    .toplevel = &on_manager_toplevel,
    .finished = &on_manager_finished
};


static void on_registry_global(
    void* data,
    struct wl_registry* registry,
    uint32_t name,
    const char* interface,
    uint32_t version
) {
    g_debug("[INFO][PROTOCOL] got protocol with name %s\n", interface);
    if (strcmp(interface, zwlr_foreign_toplevel_manager_v1_interface.name) == 0) {
        g_debug("[INFO] connecting to zwlr_foreign_toplevel_manager_v1\n");
        GlaceManager* self = data;
        struct zwlr_foreign_toplevel_manager_v1* manager = wl_registry_bind(
            registry,
            name,
            &zwlr_foreign_toplevel_manager_v1_interface,
            max(version, 1)
        );
        self->priv->wlr_manager = manager;
        zwlr_foreign_toplevel_manager_v1_add_listener(manager, &toplevel_manager_listener, self);
    } else if (strcmp(interface, wl_seat_interface.name) == 0) {
        g_debug("[INFO] got seat\n");
        GlaceManager* self = data;
        self->priv->seat = wl_registry_bind(registry, name, &wl_seat_interface, version);
    }
};


static void on_registry_global_remove(void* data, struct wl_registry *registry, uint32_t name) {}


static const struct wl_registry_listener registry_listener = {
    .global = &on_registry_global,
    .global_remove = &on_registry_global_remove,
};

static void glace_manager_init(GlaceManager* self) {
    self->priv = G_TYPE_INSTANCE_GET_PRIVATE(
        self,
        GLACE_TYPE_MANAGER,
        GlaceManagerPrivate
    );

    GdkWaylandDisplay* gdk_display = gdk_display_get_default();
    if (GDK_IS_WAYLAND_DISPLAY(gdk_display) == false) {
        // trap it in next check
        gdk_display = NULL;
    }

    DISPLAY_CHECK_NULL(gdk_display);
    self->priv->gdk_display = gdk_display;
    g_debug(
        "[INFO][MANAGER] got display with name %s",
        gdk_display_get_name(self->priv->gdk_display)
    );

    struct wl_display* display = gdk_wayland_display_get_wl_display(self->priv->gdk_display);
    DISPLAY_CHECK_NULL(display);

    self->priv->display = display;

    // all aboard...
    struct wl_registry* registry = wl_display_get_registry(self->priv->display);
    wl_registry_add_listener(registry, &registry_listener, self);

    wl_display_roundtrip(self->priv->display);
}

static void glace_manager_class_init(GlaceManagerClass* klass) {
    GObjectClass* parent_class = G_OBJECT_CLASS(klass);

    g_type_class_add_private(klass, sizeof(GlaceManagerPrivate));

    glace_manager_signals[GLACE_MANAGER_SIGNAL_CHANGED] = g_signal_new(
        "changed",
        GLACE_TYPE_MANAGER,
        G_SIGNAL_RUN_LAST | G_SIGNAL_DETAILED,
        0,
        NULL,
        NULL,
        NULL,
        G_TYPE_NONE,
        0
    );

    glace_manager_signals[GLACE_MANAGER_SIGNAL_CLIENT_ADDED] = g_signal_new(
        "client-added",
        GLACE_TYPE_MANAGER,
        G_SIGNAL_RUN_LAST | G_SIGNAL_DETAILED,
        0,
        NULL,
        NULL,
        g_cclosure_marshal_VOID__OBJECT,
        G_TYPE_NONE,
        1,
        GLACE_TYPE_CLIENT
    );

    glace_manager_signals[GLACE_MANAGER_SIGNAL_CLIENT_REMOVED] = g_signal_new(
        "client-removed",
        GLACE_TYPE_MANAGER,
        G_SIGNAL_RUN_LAST | G_SIGNAL_DETAILED,
        0,
        NULL,
        NULL,
        g_cclosure_marshal_VOID__OBJECT,
        G_TYPE_NONE,
        1,
        GLACE_TYPE_CLIENT
    );
}


GlaceManager* glace_manager_new() {
    return g_object_new(GLACE_TYPE_MANAGER, NULL);
}
