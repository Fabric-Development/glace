#pragma once

#ifndef __LIBGLACE_CLIENT_EFFECT_H__
#define __LIBGLACE_CLIENT_EFFECT_H__

#include <assert.h>
#include <gdk/gdkwayland.h>
#include <glib-object.h>
#include <gtk-3.0/gtk/gtk.h>
#include <cairo/cairo.h>
#include <stdbool.h>
#include <string.h>

G_BEGIN_DECLS

#define GLACE_TYPE_CLIENT_EFFECT (glace_client_effect_get_type())
// G_DECLARE_DERIVABLE_TYPE(GlaceClient, glace_client, GLACE, CLIENT, GObject)

#define GLACE_CLIENT_EFFECT(obj) (G_TYPE_CHECK_INSTANCE_CAST((obj), GLACE_TYPE_CLIENT_EFFECT, GlaceClientEffect))
#define GLACE_CLIENT_EFFECT_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST((klass), GLACE_TYPE_CLIENT_EFFECT, GlaceClientEffectClass))
#define GLACE_IS_CLIENT_EFFECT(obj) (G_TYPE_CHECK_INSTANCE_TYPE((obj), GLACE_TYPE_CLIENT_EFFECT))
#define GLACE_IS_CLIENT_EFFECT_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE((klass), GLACE_TYPE_CLIENT_EFFECT))
#define GLACE_CLIENT_EFFECT_GET_CLASS(obj) (G_TYPE_INSTANCE_GET_CLASS((obj), GLACE_TYPE_CLIENT_EFFECT, GlaceClientEffectClass))

typedef struct _GlaceClientEffect GlaceClientEffect;
typedef struct _GlaceClientEffectClass GlaceClientEffectClass;
typedef struct _GlaceClientEffectPrivate GlaceClientEffectPrivate;

struct _GlaceClientEffect {
    GObject parent_instance;
    GlaceClientEffectPrivate* priv;
};

struct _GlaceClientEffectClass {
    GObjectClass parent_class;

};

struct _GlaceClientEffectPrivate {
    bool destroyed;
    uint32_t id;

    struct wl_surface* surface;
    struct wl_compositor* compositor;
    struct ext_background_effect_surface_v1* effect_surface;
};



// methods
GType glace_client_effect_get_type(void) G_GNUC_CONST;
bool glace_client_effect_set_blur_region(GlaceClientEffect* self, cairo_region_t* region); // TODO: finish implementation

G_END_DECLS

#endif /* __LIBGLACE_CLIENT_EFFECT_H__ */
