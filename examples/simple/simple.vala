// !/usr/bin/env -S vala --pkg gtk+-3.0 --pkg Glace-0.1
using Gtk;
using Glace;


void main(string[] argv) {
    Gtk.init(ref argv);

    var manager = new Glace.Manager();
    manager.client_added.connect(
        (_, client) => {
            client.changed.connect(
                (_) => {
                    print(
                        @"[ID] $(client.id.to_string()) [APP ID] $(client.app_id) [TITLE] $(client.title)"
                    );
                }
            );
        }
    );

    Gtk.main();
}
