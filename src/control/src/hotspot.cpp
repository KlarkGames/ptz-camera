#include "hotspot.h"
#include <NetworkManager.h>
#include <stdio.h>

struct cb_data {
    bool success;
    GMainLoop *loop;
    NMActiveConnection *act_conn;
};

static void callback(GObject* client, GAsyncResult* result, gpointer user_data)
{
    struct cb_data *data = (struct cb_data*)user_data;

    data->act_conn = nm_client_add_and_activate_connection_finish(NM_CLIENT(client), result, NULL);
    data->success = (data->act_conn != NULL);

    g_main_loop_quit(data->loop);
}

static void callback_deactivate(GObject* client, GAsyncResult* result, gpointer user_data)
{
    struct cb_data *data = (struct cb_data*)user_data;

    data->success = nm_client_deactivate_connection_finish(NM_CLIENT(client), result, NULL);

    g_main_loop_quit(data->loop);
}

static void callback_delete(GObject* connection, GAsyncResult* result, gpointer user_data)
{
    struct cb_data *data = (struct cb_data*)user_data;

    data->success = nm_remote_connection_delete_finish((NMRemoteConnection*)connection, result, NULL);

    g_main_loop_quit(data->loop);
}

static NMConnection* get_client_nmconnection(const char *connection_id, GString *ssid, const char *password)
{
    NMConnection *connection = NULL;
    NMSettingConnection *s_con;
    NMSettingWireless *s_wireless;
    NMSettingIP4Config *s_ip4;
    NMSettingIP6Config *s_ip6;
    NMSettingWirelessSecurity *s_secure;

    connection = nm_simple_connection_new();

    s_con = (NMSettingConnection*)nm_setting_connection_new();

    g_object_set(G_OBJECT(s_con),
                 NM_SETTING_CONNECTION_ID,
                 connection_id,
                 NM_SETTING_CONNECTION_TYPE,
                 NM_SETTING_WIRELESS_SETTING_NAME,
                 NM_SETTING_CONNECTION_AUTOCONNECT,
                 FALSE,
                 NULL);
    nm_connection_add_setting(connection, NM_SETTING(s_con));

    s_wireless = (NMSettingWireless*)nm_setting_wireless_new();

    g_object_set(G_OBJECT(s_wireless),
                 NM_SETTING_WIRELESS_MODE,
                 NM_SETTING_WIRELESS_MODE_AP,
                 NM_SETTING_WIRELESS_SSID,
                 ssid,
                 NULL);
    nm_connection_add_setting(connection, NM_SETTING(s_wireless));

    s_secure = (NMSettingWirelessSecurity*)nm_setting_wireless_security_new();
    g_object_set(G_OBJECT(s_secure),
                 NM_SETTING_WIRELESS_SECURITY_KEY_MGMT,
                 "wpa-psk",
                 NM_SETTING_WIRELESS_SECURITY_PSK,
                 password,
                 NULL);
    nm_connection_add_setting(connection, NM_SETTING(s_secure));

    s_ip4 = (NMSettingIP4Config*)nm_setting_ip4_config_new();
    g_object_set(G_OBJECT(s_ip4),
                 NM_SETTING_IP_CONFIG_METHOD,
                 NM_SETTING_IP4_CONFIG_METHOD_SHARED,
                 NULL);
    nm_connection_add_setting(connection, NM_SETTING(s_ip4));

    s_ip6 = (NMSettingIP6Config*)nm_setting_ip6_config_new();
    g_object_set(G_OBJECT(s_ip6),
                 NM_SETTING_IP_CONFIG_METHOD,
                 NM_SETTING_IP6_CONFIG_METHOD_IGNORE,
                 NULL);
    nm_connection_add_setting(connection, NM_SETTING(s_ip6));

    return connection;
}

Hotspot::Hotspot()
    : m_client(nullptr)
    , m_hotspot(nullptr)
{}

bool Hotspot::start(const char* ssid, const char* password)
{
    NMClient *client = m_client ? (NMClient*)m_client : nm_client_new(NULL, NULL);
    if (!client)
        return false;
    m_client = (void*)client;

    const char *connection_id = "PtzCameraHotspot";
    GString *ssid_gstr = g_string_new(ssid);

    NMConnection *hotspot = get_client_nmconnection(connection_id, ssid_gstr, password);

    GMainLoop *loop = g_main_loop_new(NULL, FALSE);
    struct cb_data user_data;
    user_data.loop = loop;

    nm_client_add_and_activate_connection_async(client, hotspot, NULL, NULL, NULL, callback, &user_data);
    g_main_loop_run(loop);

    g_main_loop_unref(loop);
    g_string_free(ssid_gstr, FALSE);

    if (!user_data.success)
        return false;

    m_hotspot = (void*)user_data.act_conn;
    return true;
}

bool Hotspot::stop()
{
    if (!m_client || !m_hotspot)
        return false;

    GMainLoop *loop = g_main_loop_new(NULL, FALSE);
    struct cb_data user_data;
    user_data.loop = loop;

    NMActiveConnection *hotspot_act = (NMActiveConnection*)m_hotspot;
    NMRemoteConnection *hotspot_rem = nm_active_connection_get_connection(hotspot_act);

    nm_client_deactivate_connection_async((NMClient*)m_client, hotspot_act, NULL, callback_deactivate, &user_data);
    g_main_loop_run(loop);

    if (!user_data.success)
        return false;

    nm_remote_connection_delete_async(hotspot_rem, NULL, callback_delete, &user_data);
    g_main_loop_run(loop);

    g_main_loop_unref(loop);

    m_hotspot = nullptr;
    return true;
}

Hotspot::~Hotspot()
{
    stop();
}
