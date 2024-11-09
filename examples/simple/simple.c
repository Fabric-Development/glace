/* cc simple.c `pkg-config --libs --cflags glace` */
#include <glace/glace.h>

static void on_client_chagend(GlaceClient* client) {
    printf(
        "[ID] %u [APP ID] %s [TITLE] %s\n",
        glace_client_get_id(client),
        glace_client_get_app_id(client),
        glace_client_get_title(client)
    );
}

static void on_client_added(GlaceManager* self, GlaceClient* client) {
    g_signal_connect(client, "changed", G_CALLBACK(on_client_chagend), NULL);
}

int main() {
    gtk_init(NULL, NULL);

    GlaceManager* manager = glace_manager_new();

    g_signal_connect(manager, "client-added", G_CALLBACK(on_client_added), NULL);

    gtk_main();
}
