#include "libglace-private.h"

static guint glace_manager_signals[GLACE_MANAGER_N_SIGNALS] = {0};


static void glace_manager_signal_changed_emit(GlaceManager* self) {
    g_signal_emit(
        self,
        glace_manager_signals[GLACE_MANAGER_SIGNAL_CHANGED],
        0
    );
}


static void glace_manager_signal_client_added_emit(GlaceManager* self, GlaceClient* client) {
    g_signal_emit(
        self,
        glace_manager_signals[GLACE_MANAGER_SIGNAL_CLIENT_ADDED],
        1,
        client
    );
}


static void glace_manager_signal_client_removed_emit(GlaceManager* self, GlaceClient* client) {
    g_signal_emit(
        self,
        glace_manager_signals[GLACE_MANAGER_SIGNAL_CLIENT_REMOVED],
        1,
        client
    );
}


static void on_client_closed_cleanup(GlaceClient* self, gpointer data) {
    GlaceManager* manager = (GlaceManager*)data;

    glace_manager_signal_client_removed_emit(manager, self);
    glace_manager_signal_changed_emit(manager);
}


static void on_manager_toplevel(
    void* data,
    struct zwlr_foreign_toplevel_manager_v1* manager,
    struct zwlr_foreign_toplevel_handle_v1* handle
) {
    GlaceManager* self = data;
    if (GLACE_IS_MANAGER(self) != true) {
        return;
    }

    GlaceClient* client = glace_client_new(handle, self->priv->gdk_display);
    RETURN_IF_INVALID_CLIENT(client, EMPTY_TOKEN);

    g_signal_connect(client, "close", G_CALLBACK(on_client_closed_cleanup), self);

    glace_manager_signal_client_added_emit(self, client);
    glace_manager_signal_changed_emit(self);

    g_debug("[INFO][MANAGER] got a client with id %u\n", glace_client_get_id(client));
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
        g_debug("[INFO][PROTOCOL] connecting to zwlr_foreign_toplevel_manager_v1\n");
        GlaceManager* self = data;

        if (GLACE_IS_MANAGER(self) != true) {
            return;
        }

        struct zwlr_foreign_toplevel_manager_v1* manager = wl_registry_bind(
            registry,
            name,
            &zwlr_foreign_toplevel_manager_v1_interface,
            max(version, 1)
        );
        self->priv->wlr_manager = manager;

        zwlr_foreign_toplevel_manager_v1_add_listener(manager, &toplevel_manager_listener, self);
    }
};


static void on_registry_global_remove(void* data, struct wl_registry *registry, uint32_t name) {}


static const struct wl_registry_listener registry_listener = {
    .global = &on_registry_global,
    .global_remove = &on_registry_global_remove,
};


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

    g_debug(
        "[INFO][MANAGER] got display with name %s",
        gdk_display_get_name(gdk_display)
    );

    struct wl_display* display = gdk_wayland_display_get_wl_display(gdk_display);
    DISPLAY_CHECK_NULL(display);

    self->priv->display = display;
    self->priv->gdk_display = gdk_display;

    // all aboard...
    struct wl_registry* registry = wl_display_get_registry(self->priv->display);
    wl_registry_add_listener(registry, &registry_listener, self);

    wl_display_roundtrip(self->priv->display);

    if (self->priv->wlr_manager == NULL) g_warning(
        "[WARNING][MANAGER] your compositor does not support the wlr-foreign-toplevel-management protocol, Glace will not work if the protocol support is missing!"
    );
}


G_DEFINE_TYPE(GlaceManager, glace_manager, G_TYPE_OBJECT);
