#include "glace-private.h"

G_DEFINE_TYPE_WITH_PRIVATE(GlaceClientEffect, glace_client_effect, G_TYPE_OBJECT);

static void glace_client_effect_class_init(GlaceClientEffectClass* klass) {

}

static void glace_client_effect_init(GlaceClientEffect* self) {
    self->priv = glace_client_effect_get_instance_private(self);

    self->priv->id = 1;
    self->priv->surface = NULL;
    self->priv->effect_surface = NULL;
    self->priv->destroyed = false;
}

// private constructor
GlaceClientEffect* glace_client_effect_new(struct wl_compositor* compositor, struct wl_surface* surface, struct ext_background_effect_surface_v1* effect_surface) {
    GlaceClientEffect* self = g_object_new(GLACE_TYPE_CLIENT_EFFECT, NULL);

    self->priv->destroyed = false;
    self->priv->id = wl_proxy_get_id((struct wl_proxy*)effect_surface);
    self->priv->compositor = compositor;
    self->priv->effect_surface = effect_surface;
    return self;
}

bool glace_client_effect_set_blur_region(GlaceClientEffect* self, cairo_region_t* region) {
    RETURN_IF_INVALID_CLIENT_EFFECT(self, false);

    struct wl_region* blur_region = wl_region_from_cairo_region(self->priv->compositor, region);
    ext_background_effect_surface_v1_set_blur_region(self->priv->effect_surface, blur_region);
    wl_region_destroy(blur_region);
    return true;
}
