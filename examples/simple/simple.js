#!/usr/bin/env gjs

imports.gi.versions.Gtk = "3.0";
imports.gi.versions.Glace = "0.1";
const { Gtk, Glace } = imports.gi;

function main() {
    Gtk.init(ARGV);

    manager = new Glace.Manager();

    manager.connect(
        "client-added",
        (_, client) => {
            client.connect(
                "changed",
                (_) => {
                    print(`[ID] {${client.get_id()}} [APP ID] {${client.get_app_id()}} [TITLE] {${client.get_title()}}`);
                }
            );
        }
    );
}

main()

Gtk.main();
