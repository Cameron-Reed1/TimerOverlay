#include <sdbus-c++/sdbus-c++.h>
#include <memory>
#include <string>
#include <vector>
#include <map>


typedef sdbus::Struct<std::string, std::map<std::string, sdbus::Variant>> dbus_shortcut_t;
typedef void(*shortcut_callback_t)(void*);
// typedef std::function<void()> shortcut_callback_t;

struct ShortcutCallback {
    void* userData;
    shortcut_callback_t callback;
};

class GlobalShortcuts {
public:
    GlobalShortcuts() = delete;
    GlobalShortcuts(const char* const tokenPrefix);
    void addShortcut(const std::string& id, const std::string& description, const std::string& trigger, shortcut_callback_t callback, void* userData);
    int createSession();
    int bindKeys();
    bool alreadyBound();
    std::vector<dbus_shortcut_t> listBinds();
    void listen();

private:
    std::string AddNumToToken();

private:
    sdbus::ObjectPath m_SessionPath;
    std::string m_TokenPrefix;
    std::string m_ConnName;
    std::string m_Sender;
    std::unique_ptr<sdbus::IProxy> m_xdgProxy;

    std::vector<dbus_shortcut_t> m_Shortcuts;
    std::map<std::string, ShortcutCallback> m_Callbacks;
};
