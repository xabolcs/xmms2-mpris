#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include <gio/gio.h>

#include "track-info.h"
#include "xmms2.h"
#include "mpris.h"

/** The global data for the app. */
typedef struct App {
    /** The XMMS2 connection. */
    xmmsc_connection_t* con;
    /** The DBUS connection. */
    GDBusConnection* bus;
    /** The Main object for MPRIS */
    MainObject* main_object;
    /** The Player object for MPRIS */
    Player* player;
} App;

/** The global data for the app. */
static App app;

int setup_app() {
    xmmsc_connection_t* con = con = xmmsc_init("xmms2-mpris");

    if (!con) {
        return 1;
    }

    if (!xmmsc_connect(con, getenv("XMMS_PATH"))) {
        fprintf(stderr, "Connection failed: %s\n", xmmsc_get_last_error(con));

        return 1;
    }

    GDBusConnection* bus = get_dbus_connection();

    if (!bus) {
        return 1;
    }

    // Set up the main media player object.
    MainObject* main_object = init_main_dbus_object(bus);

    if (!main_object) {
        return 1;
    }

    Player* player = init_player_dbus_object(bus);

    if (!player) {
        return 1;
    }

    app.con = con;
    app.bus = bus;
    app.main_object = main_object;
    app.player = player;

    return 0;
}

static gboolean timer_callback(G_GNUC_UNUSED gpointer data) {
    XmmsTrackInfo xmms_info;

    get_xmms_track_info(app.con, &xmms_info);
    diplay_track_info(app.player, &xmms_info);
    get_xmms_track_info_unref(&xmms_info);

    return true;
}

int main(int argc, char** argv) {
    if (setup_app()) {
        return 1;
    }

    GMainLoop *loop = g_main_loop_new(NULL, false);

    g_timeout_add(1000, timer_callback, NULL);

    g_main_loop_run(loop);

    // Clean up.
    g_object_unref(app.main_object);
    g_object_unref(app.player);

    return 0;
}
