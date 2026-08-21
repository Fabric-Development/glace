#pragma once

#ifndef __LIBGLACE_CLIENT_EFFECT_PRIVATE_H__
#define __LIBGLACE_CLIENT_EFFECT_PRIVATE_H__

#include "glace-client-effect.h"

#define IF_INVALID_CLIENT_EFFECT(client) if (client == NULL || GLACE_IS_CLIENT_EFFECT(client) == false || client->priv->destroyed == true)

#define RETURN_IF_INVALID_CLIENT_EFFECT(client, r_value)                                                                                                                    \
    do {                                                                                                                                                                    \
        IF_INVALID_CLIENT_EFFECT(client) {                                                                                                                                  \
            g_warning("[WARNING][CLIENT EFFECT] function %s got an invalid client effect handle, you should drop this one and replace it with a fresh new one.", __func__); \
            return r_value;                                                                                                                                                 \
        }                                                                                                                                                                   \
    } while (0)

// got it from the internet, blame them.
static inline struct wl_region* wl_region_from_cairo_region(struct wl_compositor* compositor, cairo_region_t* cairo_reg) {
    struct wl_region* wl_reg = wl_compositor_create_region(compositor);
    if (!wl_reg)
        return NULL;

    int count = cairo_region_num_rectangles(cairo_reg);
    for (int i = 0; i < count; i++) {
        cairo_rectangle_int_t rect;
        cairo_region_get_rectangle(cairo_reg, i, &rect);
        wl_region_add(wl_reg, rect.x, rect.y, rect.width, rect.height);
    }

    return wl_reg;
}

GlaceClientEffect* glace_client_effect_new(struct wl_compositor* compositor, struct wl_surface* surface, struct ext_background_effect_surface_v1* effect_surface);

#endif /* __LIBGLACE_CLIENT_EFFECT_PRIVATE_H__ */
