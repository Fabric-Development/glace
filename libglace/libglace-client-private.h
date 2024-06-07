#ifndef __LIBGLACE_CLIENT_PRIVATE_H__
#define __LIBGLACE_CLIENT_PRIVATE_H__

#include "libglace-client.h"
#include "libglace-manager-private.h"

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


// signal emitters
static void glace_client_signal_changed_emit(GlaceClient* self);
static void glace_client_signal_close_emit(GlaceClient* self);

// methods
static void glace_client_init(GlaceClient* self);
static void glace_client_class_init(GlaceClientClass* klass);

GlaceClient* glace_client_new(struct zwlr_foreign_toplevel_handle_v1* wlr_handle, GdkWaylandDisplay* gdk_display);


#endif /* __LIBGLACE_CLIENT_PRIVATE_H__ */
