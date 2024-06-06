import gi
gi.require_version("Gtk", "3.0")
gi.require_version('Glace', '0.1')
from gi.repository import Gtk, Glace

def on_client_added(_, client):
    client.connect(
        "changed",
        lambda *args: print(
            f"[ID] {client.get_id()} [APP ID] {client.get_app_id()} [TITLE] {client.get_title()}"
        )
    )

manager = Glace.Manager()
manager.connect("client-added", on_client_added)

Gtk.main()
