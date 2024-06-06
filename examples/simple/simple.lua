local gi = require "lgi"
local Gtk = gi.require("Gtk", "3.0")
local Glace = gi.require("Glace", "0.1")

local function main()
    Glace.Manager {
        on_client_added = function(watcher, client)
            function client:on_changed()
                print(
                    string.format(
                        "[ID] %u [APP ID] %s [TITLE] %s",
                        client:get_id(),
                        client:get_app_id(),
                        client:get_title()
                    )
                )
             end
        end
    }
    Gtk.main()
end

main()
