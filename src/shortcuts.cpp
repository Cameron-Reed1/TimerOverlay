#include <cstdlib>
#include <sdbus-c++/IProxy.h>
#include <sdbus-c++/sdbus-c++.h>
#include <iostream>
#include <cstdint>
#include <memory>
#include <string>
#include <unistd.h>
#include <utility>
#include <random>
#include <vector>
#include <map>

#include "shortcuts.h"


const char* xdgName = "org.freedesktop.portal.Desktop";
const char* xdgPath = "/org/freedesktop/portal/desktop";
const char* shortcutsInterface = "org.freedesktop.portal.GlobalShortcuts";
const char* requstInterface = "org.freedesktop.portal.Request";




GlobalShortcuts::GlobalShortcuts(const char* const tokenPrefix)
    : m_TokenPrefix(tokenPrefix), m_Shortcuts() { };


std::string GlobalShortcuts::AddNumToToken()
{
    std::random_device dev;
    std::mt19937 rng(dev());
    std::uniform_int_distribution<std::mt19937::result_type> dist(1000,9999);

    return m_TokenPrefix + std::to_string(dist(rng));
}

void GlobalShortcuts::addShortcut(const std::string& id, const std::string& description, const std::string& trigger, shortcut_callback_t callback, void* userData)
{
    std::map<std::string, sdbus::Variant> smap;
    smap["description"] = sdbus::Variant(description);
    smap["preferred_trigger"] = sdbus::Variant(trigger);
    
    m_Shortcuts.emplace_back(id, smap);

    m_Callbacks[id] = ShortcutCallback{ userData, callback };
}

int GlobalShortcuts::createSession()
{
    bool requestInProgress = false;
    int result = 1;

    auto conn = sdbus::createSessionBusConnection();
    m_ConnName = conn->getUniqueName();
    m_Sender = m_ConnName.substr(1);


    m_xdgProxy = sdbus::createProxy(std::move(conn), xdgName, xdgPath);

    std::map<std::string, sdbus::Variant> create_session_args;
    std::string token = AddNumToToken();
    create_session_args["handle_token"] = sdbus::Variant(token);
    create_session_args["session_handle_token"] = sdbus::Variant(m_TokenPrefix);

    for (size_t i = 1; i < m_Sender.length(); i++) {
        if (m_Sender[i] == '.') {
            m_Sender[i] = '_';
        }
    }
    std::string expectedRequestPath = "/org/freedesktop/portal/desktop/request/" + m_Sender + "/" + token;

    auto requestProxy = sdbus::createProxy(m_xdgProxy->getConnection(), xdgName, expectedRequestPath);
    requestProxy->uponSignal("Response").onInterface(requstInterface)
        .call([this, &requestInProgress, &result](uint32_t result_code, std::map<std::string, sdbus::Variant> res_map)
            {
                if (result_code == 0) {
                    this->m_SessionPath = res_map.at("session_handle").get<std::string>();
                } else {
                    std::cerr << "Failed to create GlobalShortcuts session" << std::endl;
                }
                result = result_code;
                requestInProgress = false;
            });
    requestProxy->finishRegistration();


    sdbus::ObjectPath requestPath;
    requestInProgress = true;
    m_xdgProxy->callMethod("CreateSession").onInterface(shortcutsInterface)
        .withArguments(create_session_args).storeResultsTo(requestPath);

    std::unique_ptr<sdbus::IProxy> requestProxy2;
    if (requestPath != expectedRequestPath) {
        requestProxy->unregister();
        requestProxy2 = sdbus::createProxy(m_xdgProxy->getConnection(), xdgName, requestPath);
        requestProxy2->uponSignal("Response").onInterface(requstInterface)
            .call([this, &requestInProgress, &result](uint32_t result_code, std::map<std::string, sdbus::Variant> res_map)
                {
                    if (result_code == 0) {
                        this->m_SessionPath = res_map.at("session_handle").get<sdbus::ObjectPath>();
                    } else {
                        std::cerr << "Failed to create GlobalShortcuts session" << std::endl;
                    }
                    result = result_code;
                    requestInProgress = false;
                });
        requestProxy2->finishRegistration();
    }

    while (requestInProgress) {
        usleep(500);
    }

    return result;
}

bool GlobalShortcuts::alreadyBound()
{
    std::vector<dbus_shortcut_t> binds = listBinds();
    if (binds.size() != m_Shortcuts.size()) {
        return false;
    }

    for (dbus_shortcut_t shortcut: m_Shortcuts) {
        const std::string shortcut_id = shortcut.get<0>();
        bool match = false;
        for (dbus_shortcut_t bind: binds) {
            const std::string bind_id = bind.get<0>();
            if (shortcut_id == bind_id) {
                match = true;
                break;
            }
        }

        if (!match) {
            return false;
        }
    }

    return true;
}

std::vector<dbus_shortcut_t> GlobalShortcuts::listBinds()
{
    std::vector<dbus_shortcut_t> binds;
    bool requestInProgress = false;

    std::map<std::string, sdbus::Variant> list_shortcuts_args;
    const std::string token = AddNumToToken();
    list_shortcuts_args["handle_token"] = sdbus::Variant(token);

    sdbus::ObjectPath expectedRequestPath = "/org/freedesktop/portal/desktop/request/" + m_Sender + "/" + token;

    auto requestProxy = sdbus::createProxy(m_xdgProxy->getConnection(), xdgName, expectedRequestPath);
    requestProxy->uponSignal("Response").onInterface(requstInterface)
        .call([&requestInProgress, &binds](uint32_t result_code, std::map<std::string, sdbus::Variant> res_map)
            {
                if (result_code == 0) {
                    binds = res_map.at("shortcuts").get<std::vector<dbus_shortcut_t>>();
                    // for (dbus_shortcut_t bind: binds) {
                    //     std::cout << "id: " << bind.get<0>();
                    //     std::cout << ", desc: " << bind.get<1>().at("description").get<std::string>();
                    //     std::cout << ", trigger: " << bind.get<1>().at("trigger_description").get<std::string>();
                    //     std::cout << std::endl << std::endl;
                    // }
                } else {
                    std::cerr << "Failed to list shortcuts" << std::endl;
                }
                requestInProgress = false;
            });
    requestProxy->finishRegistration();


    sdbus::ObjectPath requestPath;
    requestInProgress = true;
    m_xdgProxy->callMethod("ListShortcuts").onInterface(shortcutsInterface)
        .withArguments(m_SessionPath, list_shortcuts_args).storeResultsTo(requestPath);

    std::unique_ptr<sdbus::IProxy> requestProxy2;
    if (requestPath != expectedRequestPath) {
        requestProxy->unregister();
        requestProxy2 = sdbus::createProxy(m_xdgProxy->getConnection(), xdgName, requestPath);
        requestProxy2->uponSignal("Response").onInterface(requstInterface)
            .call([&requestInProgress, &binds](uint32_t result_code, std::map<std::string, sdbus::Variant> res_map)
                {
                    if (result_code == 0) {
                        binds = res_map.at("shortcuts").get<std::vector<dbus_shortcut_t>>();
                    } else {
                        std::cerr << "Failed to list shortcuts" << std::endl;
                    }
                    requestInProgress = false;
                });
        requestProxy2->finishRegistration();
    }


    while (requestInProgress) {
        usleep(500);
    }

    return binds;
}

int GlobalShortcuts::bindKeys()
{
    bool requestInProgress = false;
    int result = 1;

    // std::cout << "session: " << this->sessionPath.c_str() << std::endl;

    std::map<std::string, sdbus::Variant> bind_shortcuts_args;
    const std::string token = AddNumToToken();
    bind_shortcuts_args["handle_token"] = sdbus::Variant(token);

    sdbus::ObjectPath expectedRequestPath = "/org/freedesktop/portal/desktop/request/" + m_Sender + "/" + token;

    auto requestProxy = sdbus::createProxy(m_xdgProxy->getConnection(), xdgName, expectedRequestPath);
    requestProxy->uponSignal("Response").onInterface(requstInterface)
        .call([&requestInProgress, &result](uint32_t result_code, std::map<std::string, sdbus::Variant> res_map){
                (void) res_map;
                if (result_code == 0) {
                    // auto binds = res_map.at("shortcuts").get<std::vector<dbus_shortcut_t>>();
                    // for (auto bind: binds) {
                    //     std::cout << "id: " << bind.get<0>();
                    //     std::cout << ", desc: " << bind.get<1>().at("description").get<std::string>();
                    //     std::cout << ", trigger: " << bind.get<1>().at("trigger_description").get<std::string>();
                    //     std::cout << std::endl;
                    // }
                } else {
                    std::cerr << "Failed to bind shortcuts" << std::endl;
                }
                result = result_code;
                requestInProgress = false;
            });
    requestProxy->finishRegistration();

    sdbus::ObjectPath requestPath;
    requestInProgress = true;
    m_xdgProxy->callMethod("BindShortcuts").onInterface(shortcutsInterface)
        .withArguments(m_SessionPath, m_Shortcuts, "", bind_shortcuts_args).storeResultsTo(requestPath);

    std::unique_ptr<sdbus::IProxy> requestProxy2;
    if (requestPath != expectedRequestPath) {
        requestProxy->unregister();
        requestProxy2 = sdbus::createProxy(m_xdgProxy->getConnection(), xdgName, requestPath);
        requestProxy2->uponSignal("Response").onInterface(requstInterface)
            .call([&requestInProgress, &result](uint32_t result_code, std::map<std::string, sdbus::Variant> res_map){
                    (void) res_map;
                    if (result_code == 0) {
                        std::cerr << "Failed to bind shortcuts" << std::endl;
                    }
                    result = result_code;
                    requestInProgress = false;
                });
        requestProxy2->finishRegistration();
    }

    while (requestInProgress) {
        usleep(500);
    }

    return result;
}

void GlobalShortcuts::listen()
{
    m_xdgProxy->uponSignal("Activated").onInterface(shortcutsInterface)
        .call([this](sdbus::ObjectPath session_handle, const std::string& shortcut_id, uint64_t timestamp, std::map<std::string, sdbus::Variant> options)
            {
                (void) options;
                (void) session_handle;
                (void) timestamp;

                // std::cout << "Shortcut activated!" << std::endl;
                // std::cout << "session: " << session_handle << ", id: " << shortcut_id << ", time: " << timestamp << std::endl << std::endl;
                ShortcutCallback cb = this->m_Callbacks[shortcut_id];
                cb.callback(cb.userData);
            });
    m_xdgProxy->finishRegistration();
}


// Very basic example

//int main()
//{
//    GlobalShortcuts shortcuts;
//
//    if (shortcuts.createSession() != 0) {
//        std::cout << "Failed to create shortcuts session" << std::endl;
//        return -1;
//    }
//
//    shortcuts.addShortcut("test1", "Prints things", "CTRL+SHIFT+a");
//    shortcuts.addShortcut("test2", "Prints things, but like, different", "CTRL+SHIFT+b");
//
//    if (shortcuts.listBinds().size() == 0) {
//        std::cout << "Requsting to bind keys" << std::endl;
//        shortcuts.bindKeys();
//    }
//
//    shortcuts.listen();
//
//    while (true) {};
//}
//

// Code for working with libdbus directly. There isn't a lot of information around on how to use it
// And it is pretty difficult to use

//void connect()
//{
//    DBusConnection* dbus_conn = nullptr;
//    DBusError dbus_error;
//
//    dbus_error_init(&dbus_error);
//
//    dbus_conn = dbus_bus_get(DBUS_BUS_SESSION, &dbus_error);
//    std::cout << "Connected to DBUS as \"" << dbus_bus_get_unique_name(dbus_conn) << "\"." << std::endl;
//
//    // Request connection to GlobalShortcuts portal
//    DBusMessage* conn_request_msg = dbus_message_new_method_call("org.freedesktop.portal.Desktop", "/org/freedesktop/portal/desktop", "org.freedesktop.portal.GlobalShortcuts", "CreateSession");
//    DBusMessage* reply = dbus_connection_send_with_reply_and_block(dbus_conn, conn_request_msg, DBUS_TIMEOUT_USE_DEFAULT, &dbus_error);
//
//    const char* conn_request_path = nullptr;
//    dbus_message_get_args(reply, &dbus_error, DBUS_TYPE_OBJECT_PATH, conn_request_path, DBUS_TYPE_INVALID); 
//
//    dbus_message_unref(reply);
//    dbus_message_unref(conn_request_msg);
//
//    std::string rule = "type='signal',path='";
//    rule = rule + conn_request_path + '\'';
//    dbus_bus_add_match(dbus_conn, rule.c_str(), &dbus_error);
//    while (true) {
//        dbus_connection_read_write(dbus_conn, DBUS_TIMEOUT_USE_DEFAULT);
//        DBusMessage* msg_in = dbus_connection_pop_message(dbus_conn);
//        if (msg_in == nullptr) {
//            continue;
//        }
//
//        if (dbus_message_is_signal(msg_in, "org.freedesktop.portal.Request", "Response")) {
//            uint32_t result;
//            
//            dbus_message_get_args(msg_in, &dbus_error, DBUS_TYPE_UINT32, &result, DBUS_TYPE_ARRAY, , DBUS_TYPE_INVALID); 
//
//            std::cout << "Global shortcut session creation result: " << result << std::endl;
//        }
//
//        dbus_message_unref(msg_in);
//    }
//
//    dbus_connection_unref(dbus_conn);
//}
