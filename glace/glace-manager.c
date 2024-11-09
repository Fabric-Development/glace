#include "glace-private.h"

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
    if (!GLACE_IS_MANAGER(self)) {
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

    GlaceManager* self = data;
    if (!GLACE_IS_MANAGER(self)) {
        return;
    }

    if (strcmp(interface, zwlr_foreign_toplevel_manager_v1_interface.name) == 0) {
        g_debug("[INFO][PROTOCOL] connecting to zwlr_foreign_toplevel_manager_v1\n");
        struct zwlr_foreign_toplevel_manager_v1* manager = wl_registry_bind(
            registry,
            name,
            &zwlr_foreign_toplevel_manager_v1_interface,
            max(version, 1)
        );
        self->priv->wlr_manager = manager;

        zwlr_foreign_toplevel_manager_v1_add_listener(manager, &toplevel_manager_listener, self);
    } else if (strcmp(interface, hyprland_toplevel_export_manager_v1_interface.name) == 0) {
        g_debug("[INFO][PROTOCOL] connecting to hyprland_toplevel_export_manager_v1\n");

        struct hyprland_toplevel_export_manager_v1* export_manager = wl_registry_bind(
            registry,
            name,
            &hyprland_toplevel_export_manager_v1_interface,
            max(version, 1)
        );
        self->priv->hl_export_manager = export_manager;
    } else if (strcmp(interface, wl_shm_interface.name) == 0) {
        g_debug("[INFO][PROTOCOL] getting a shared memory buffer\n");

        self->priv->wl_shm = wl_registry_bind(registry, name, &wl_shm_interface, version);
    }

    return;
};

static void on_registry_global_remove(void* data, struct wl_registry* registry, uint32_t name) {}

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

    if (self->priv->wlr_manager == NULL)
        g_warning(
            "[WARNING][MANAGER] your compositor does not support the wlr-foreign-toplevel-management protocol, Glace will not work if the protocol support is missing!"
        );
}

static int anonymous_shm_open() {
    char* name;
    int retries = 100;
    do {
        --retries;
        name = g_strdup_printf("/glace-hyprland-frame-%i", g_random_int());

        // shm_open guarantees that O_CLOEXEC is set
        int fd = shm_open(name, O_RDWR | O_CREAT | O_EXCL, 0600);
        if (fd >= 0) {
            shm_unlink(name);
            return fd;
        }
    } while (retries > 0 && errno == EEXIST);

    return -1;
}

static int create_shm_file(off_t size) {
    int fd = anonymous_shm_open();
    if (fd < 0) {
        return fd;
    }

    if (ftruncate(fd, size) < 0) {
        close(fd);
        return -1;
    }

    return fd;
}

static GlaceFrameBuffer* glace_frame_buffer_new(struct wl_shm* shm, enum wl_shm_format format, int32_t width, int32_t height, int32_t stride) {
    size_t size = stride * height;

    int fd = create_shm_file(size);
    if (fd == -1) {
        return NULL;
    }

    if (shm == NULL) {
        return NULL;
    }

    void* raw_buffer = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (raw_buffer == MAP_FAILED) {
        close(fd);
        return NULL;
    }

    struct wl_shm_pool* pool = wl_shm_create_pool(shm, fd, size);

    if (!pool) {
        munmap(raw_buffer, size);
        close(fd);
        return NULL;
    }

    struct wl_buffer* wl_buffer = wl_shm_pool_create_buffer(pool, 0, width, height, stride, format);

    wl_shm_pool_destroy(pool);
    close(fd);

    GlaceFrameBuffer* buffer = calloc(1, sizeof(GlaceFrameBuffer));
    buffer->wl_buffer = wl_buffer;
    buffer->raw_buffer = raw_buffer;
    buffer->size = size;
    buffer->width = width;
    buffer->height = height;
    buffer->stride = stride;
    buffer->format = format;

    return buffer;
}

static inline void frame_buffer_correct_format(GlaceFrameBuffer* buffer) {
    // BGRA -> RGBA
    uint32_t* agbr = (uint32_t*)buffer->raw_buffer;
    for (size_t i = 0; i < buffer->width * buffer->height; i++) {
        uint32_t pixel = agbr[i];

        agbr[i] = (pixel & 0xFF00FF00) | ((pixel << 16) & 0x00FF0000) | ((pixel >> 16) & 0xFF);
    }
}

static void glace_frame_buffer_destroy(GlaceFrameBuffer* buffer) {
    if (buffer == NULL) {
        return;
    }
    munmap(buffer->raw_buffer, buffer->size);
    wl_buffer_destroy(buffer->wl_buffer);
    free(buffer);
}

static void glace_frame_data_destroy(GlaceFrameData* data) {
    // die and be a hero or live long enough to see yourself become a villain
    if (!data) {
        return;
    }

    if (data->buffer != NULL) {
        glace_frame_buffer_destroy(data->buffer);
    }

    data->buffer = NULL;
    data->client = NULL;
    data->manager = NULL;
    data->callback = NULL;
    data->callback_data = NULL;

    free(data);
    return;
}

static void on_export_manager_frame_buffer(void* user_data, struct hyprland_toplevel_export_frame_v1* export_frame, uint32_t format, uint32_t width, uint32_t height, uint32_t stride) {
    GlaceFrameData* data = user_data;
    data->buffer = glace_frame_buffer_new(data->manager->priv->wl_shm, format, width, height, stride);
    return;
}

static void on_export_manager_frame_damage(void* user_data, struct hyprland_toplevel_export_frame_v1* export_frame, uint32_t x, uint32_t y, uint32_t width, uint32_t height) {}

static void on_export_manager_frame_flags(void* user_data, struct hyprland_toplevel_export_frame_v1* export_frame, uint32_t flags) {}

static void on_export_manager_frame_ready(void* user_data, struct hyprland_toplevel_export_frame_v1* export_frame, uint32_t tv_sec_hi, uint32_t tv_sec_lo, uint32_t tv_nsec) {
    GlaceFrameData* data = user_data;

    if (!data || !data->buffer) {
        data->callback(NULL, data->callback_data);
        return;
    }

    frame_buffer_correct_format(data->buffer);

    GdkPixbuf* pixbuf = gdk_pixbuf_new_from_data(
        data->buffer->raw_buffer,
        GDK_COLORSPACE_RGB,
        TRUE,
        8,
        data->buffer->width,
        data->buffer->height,
        data->buffer->stride,
        NULL,
        NULL
    );

    data->callback(pixbuf, data->callback_data);
    glace_frame_data_destroy(data);

    hyprland_toplevel_export_frame_v1_destroy(export_frame);

    return;
}

static void on_export_manager_frame_failed(void* user_data, struct hyprland_toplevel_export_frame_v1* export_frame) {
    GlaceFrameData* data = user_data;

    data->callback(NULL, data->callback_data);
    glace_frame_data_destroy(data);

    hyprland_toplevel_export_frame_v1_destroy(export_frame);

    return;
}

static void on_export_manager_frame_linux_dmabuf(void* user_data, struct hyprland_toplevel_export_frame_v1* export_frame, uint32_t format, uint32_t width, uint32_t height) {}

static void on_export_manager_frame_buffer_done(void* user_data, struct hyprland_toplevel_export_frame_v1* export_frame) {
    GlaceFrameData* data = user_data;

    // all aboard...
    hyprland_toplevel_export_frame_v1_copy(export_frame, data->buffer->wl_buffer, 1);

    return;
}

static const struct hyprland_toplevel_export_frame_v1_listener export_manager_frame_listener = {
    .buffer = &on_export_manager_frame_buffer,
    .damage = &on_export_manager_frame_damage,
    .flags = &on_export_manager_frame_flags,
    .ready = &on_export_manager_frame_ready,
    .failed = &on_export_manager_frame_failed,
    .linux_dmabuf = &on_export_manager_frame_linux_dmabuf,
    .buffer_done = &on_export_manager_frame_buffer_done
};

// public methods
GlaceManager* glace_manager_new() {
    return g_object_new(GLACE_TYPE_MANAGER, NULL);
}

void glace_manager_capture_client(GlaceManager* self, GlaceClient* client, gboolean overlay_cursor, GlaceManagerCaptureClientCallback callback, gpointer user_data, GDestroyNotify notify) {
    if (!self->priv->hl_export_manager) {
        g_warning_once("at the moment, capturing a client is only available for Hyprland users.");
        callback(NULL, user_data);
        return;
    }

    GlaceFrameData* data = calloc(1, sizeof(GlaceFrameData));
    data->manager = self;
    data->client = client;
    data->callback = callback;
    data->callback_data = user_data;
    data->buffer = NULL;

    struct hyprland_toplevel_export_frame_v1* frame = hyprland_toplevel_export_manager_v1_capture_toplevel_with_wlr_toplevel_handle(
        self->priv->hl_export_manager, (gint)overlay_cursor, client->priv->wlr_handle
    );

    hyprland_toplevel_export_frame_v1_add_listener(frame, &export_manager_frame_listener, data);

    return;
}

G_DEFINE_TYPE(GlaceManager, glace_manager, G_TYPE_OBJECT);
