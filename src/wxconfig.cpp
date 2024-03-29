#include "wxconfig.h"

const string  gtk_prefix("file://");
const int PRE_LEN = gtk_prefix.length();
vector<string> bookmarks;

const string sections[] = {
    "plain",
    "server",
    "buttons",
};

Config::Config()
{
    string home = getenv("HOME");
    config_path = home + "/.config/wcmd/config";
    bookmark_path = home + "/.gtk-bookmarks";

    build_config_list(home);
    vector<plain_config>::iterator iter;
    int ret;
    if ((ret = checkpath(config_path)) == -1) {
        fprintf(stderr, "ERROR: failed to check path!\n");
    }
    else {
        read_configs();
        read_bookmarks();
    }
    dentry_pos = 0;
    sentry_pos = 0;
}


Config::~Config()
{
    dump2file();
}

void Config::build_config_list(string &home)
{
    plain_config auto_last_path_l = {
        "auto_last_path_l", home, TYPE_STR
    };
    entry_list.push_back(auto_last_path_l);

    plain_config auto_last_path_r = {
        "auto_last_path_r", home, TYPE_STR
    };
    entry_list.push_back(auto_last_path_r);

    plain_config auto_size_x = {
        "auto_size_x", "1280", TYPE_STR
    };
    entry_list.push_back(auto_size_x);

    plain_config auto_size_y = {
        "auto_size_y", "850", TYPE_STR
    };
    entry_list.push_back(auto_size_y);

    plain_config show_hidden = {
        "show_hidden", "false", TYPE_BOOL
    };
    entry_list.push_back(show_hidden);

    plain_config editor = {
        "editor", "/usr/bin/emacsclient",TYPE_STR
    };
    entry_list.push_back(editor);

    plain_config app_terminal = {
        "app_terminal", "sakura", TYPE_STR
    };
    entry_list.push_back(app_terminal);

    plain_config img_editor = {
        "img_editor", "gimp", TYPE_STR
    };
    entry_list.push_back(img_editor);

    plain_config img_viewer = {
        "img_viewer", "display", TYPE_STR
    };
    entry_list.push_back(img_viewer);

    plain_config pdf_reader = {
        "pdf_reader", "evince", TYPE_STR
    };
    entry_list.push_back(pdf_reader);

    plain_config video_player = {
        "video_player", "mplayer", TYPE_STR
    };
    entry_list.push_back(video_player);

    plain_config diff_tool = {
        "diff_tool", "meld", TYPE_STR
    };
    entry_list.push_back(diff_tool);
}

string Config::get_config(const string key)
{
    vector<plain_config>::iterator iter;
    string val = "";
    for (iter = entry_list.begin(); iter < entry_list.end(); iter++) {
        if ((*iter).desc == key) {
            val = (*iter).value;
            return val;
        }
    }
    return "";
}

void Config::set_config(const string key, const string val)
{
    vector<plain_config>::iterator iter;
    for (iter = entry_list.begin(); iter < entry_list.end(); iter++) {
        if ((*iter).desc == key) {
                (*iter).value = val;
                break;
        }
    }
}

void Config::dump2file()
{
    dump_config();
    dump_bookmarks();
}

void Config::dump_config()
{
    ofstream fout;
    string path = string(getenv("HOME"));
    string tmp;
    path += "/.config/wcmd/config";
    fout.open(path.c_str());
    if(fout == NULL){
        fprintf(stderr, "ERROR: Failed to open config file!\n");
        exit(0);
    }

    vector<plain_config>::iterator iter;

    // write plain section.
    fout << "[plain]" << endl;
    for (iter = entry_list.begin(); iter < entry_list.end(); iter++) {

        tmp = (*iter).desc + " = " + (*iter).value;
        fout << tmp << endl;
    }

    // write other sections
    PDEBUG ("Other sections: %d\n", (int)dentry_list.size());

    tmp = "[server]\n" ;
    fout << tmp << endl;
    for(server_iter=server_list.begin();
        server_iter < server_list.end(); server_iter ++) {
        if ((*server_iter).name.size()) {
            PDEBUG ("Dumping: %s\n", (*server_iter).name.c_str());

            tmp = (*server_iter).name + " = " + \
                num2string((int)((*server_iter).type)) + ":"   +     \
                (*server_iter).ip + ":" +     \
                (*server_iter).user + ":" +   \
                (*server_iter).passwd;
            fout << tmp << endl;
        }
    }

    for(dentry_iter=dentry_list.begin();
        dentry_iter < dentry_list.end(); dentry_iter ++) {
        if ((*dentry_iter).name.size()) {
            tmp = "[" + (*dentry_iter).name + "]\n" + \
                "icon = " + (*dentry_iter).icon + "\n"\
                "exec = " + (*dentry_iter).exec;
            fout << tmp << endl;
        }
    }
    fout.close();
}

void Config::dump_bookmarks()
{
    ofstream fout;
    string path = string(getenv("HOME"));
    path += "/.gtk-bookmarks";
    fout.open(path.c_str());
    if(fout == NULL){
        fprintf(stderr, "ERROR: Failed to open config file!\n");
        exit(0);
    }

    vector<string>::iterator iter;
    for (iter = bookmarks.begin(); iter < bookmarks.end(); iter++) {

        fout << gtk_prefix + *iter << endl;
    }

    fout.close();
}

/**
 * Reads configurations, fill them into enter_list.
 * @return void
 */
void Config::read_configs()
{
    ifstream fin;
    fin.open(config_path.c_str());
    if (!fin) {
        ;
    }
    else {
        int ret;
        string str, key, val, section;
        vector<plain_config>::iterator iter;
        bool found;
        desktop_entry dentry;
        server_entry s_entry;

        while (getline(fin, str) != NULL) {
            if (str == "[" + sections[0] + "]") {
                section = sections[0];
                continue;
            }
            else if ((str[0] == '[') && (str[str.size() - 1] == ']')) {
                section = str.substr(1, str.size() - 2);
                continue;
            }

            if ((ret = splitstr(str, key, val)) != 0) {
                fprintf(stderr, "ERROR: Failed to parse string\n");
                PDEBUG ("String: %s\n", str.c_str());

                continue;
            }
            if (section == sections[0]) {
                for (iter=entry_list.begin();iter<entry_list.end();iter++){
                    if (key == (*iter).desc) {
                        (*iter).value = val;
                        break;
                    }
                }
            }
            else if (section == sections[1]){ // SSH_SERVER
                PDEBUG ("Reading Config: Name = %s\n", key.c_str());

                s_entry.name = key;
                vector<string> str_list;
                // type:ip:user:passwd
                if (strsplit(val, ":", str_list)) {
                    s_entry.type = (SERVER_TYPE)string2num(str_list[0]);
                    s_entry.ip = str_list[1];
                    s_entry.user = str_list[2];
                    s_entry.passwd = str_list[3];
                    server_list.push_back(s_entry);
                }
            }

            else { // buttons.
                found = false;
                for(dentry_iter=dentry_list.begin();
                    dentry_iter < dentry_list.end(); dentry_iter ++) {
                    if ((*dentry_iter).name == section) {
                        found = true;
                        break;
                    }
                }
                if (found) {
                    PDEBUG ("Section %s found.\n", section.c_str());

                    if (key == "icon")
                        (*dentry_iter).icon = val;
                    else if (key == "exec")
                        (*dentry_iter).exec = val;
                    else {
                        PDEBUG ("Should not happen: %s = %s.\n",
                                key.c_str(), val.c_str());

                    }
                }
                else {
                    PDEBUG ("Section %s not found.\n", section.c_str());

                    dentry.name = section;
                    if (key == "icon")
                        dentry.icon = val;
                    else if (key == "exec")
                        dentry.exec = val;
                    else  {
                        PDEBUG ("Should not happen: %s = %s.\n",
                                key.c_str(), val.c_str());

                    }
                    dentry_list.push_back(dentry);
                }
            }
        }
#ifdef DEBUG
        for(dentry_iter=dentry_list.begin();
            dentry_iter < dentry_list.end(); dentry_iter ++) {
            cout << "Section: " << (*dentry_iter).name << endl;
            cout << "Icon: " << (*dentry_iter).icon << endl;
            cout << "Exec: " << (*dentry_iter).exec << endl;
        }

        for (server_iter = server_list.begin(); server_iter < server_list.end(); server_iter++) {
            cout << "Name = " << (*server_iter).name << endl;
            cout << "IP = " << (*server_iter).ip << endl;
            cout << endl;

        }

#endif
    }
}

/**
 * Read bookmarks, fill them into bookmarks.
 * @return void
 */
void Config::read_bookmarks()
{
    ifstream fin;
    fin.open(bookmark_path.c_str());
    if (!fin) {

    }
    else {
        string str, key, val;
        while (getline(fin, str) != NULL) {

            str = str.substr(PRE_LEN, str.length() - PRE_LEN);
            if (is_dir_exist(str))
                bookmarks.push_back(str);
            else
                fprintf(stderr, "ERROR: invalid bookmark! %s\n", str.c_str());
        }
    }
}

int Config::checkpath(const string &path)
{
    if (!is_dir_exist(dirname(strdup(path.c_str())))) {
        char cmd[256] = {'\0'};
        sprintf(cmd, "mkdir -p %s", path.c_str());
        return system(cmd);
    }
    return 0;
}

int Config::splitstr(string &str, string &key, string &val)
{
    size_t pos = str.find("=");
    if (pos == string::npos ){

        return -1;
    }
    key.assign(str,0, pos-1);
    val.assign(str, pos+1, str.length() - pos);
    strip(key);
    strip(val);
    if (key.empty() || val.empty()) {

        return -1;
    }
    return 0;
}

void Config::strip(string &str)
{
    unsigned int i = 0;
    while (str[i] == ' ' && i < str.length()) {
        i++;
    }
    str.erase(0, i);
}


void Config::add_dentry(const string &name, const string &exec,
                        const string &icon)
{
    bool found = false;

    for(dentry_iter=dentry_list.begin();
        dentry_iter < dentry_list.end(); dentry_iter ++) {
        if ((*dentry_iter).name == name) {
            found = true;
            break;
        }
    }
    if (found) {
        (*dentry_iter).exec = exec;
        (*dentry_iter).icon = icon;
    }
    else {
        desktop_entry entry;
        entry.name = name;
        entry.icon = icon;
        entry.exec = exec;
        dentry_list.push_back(entry);
    }
}

void Config::del_dentry(const string &name)
{
    bool found;
    for(dentry_iter=dentry_list.begin();
        dentry_iter < dentry_list.end(); dentry_iter ++) {
        if ((*dentry_iter).name == name) {
            found = true;
            break;
        }
    }
    if (found) {
        dentry_list.erase(dentry_iter);
    }
}

server_entry *Config::get_sentry(int id)
{
    if ((id > (int)server_list.size() - 1) || (id < 0)) {
        fprintf(stderr, "ERROR: index exceeded range: %d, %d\n",
                id, (int)server_list.size() - 1);
        return NULL;
    }
    return &server_list[id];
}

/**
 * Get next desktop entry stored in this module.
 *
 * @param type - server_entry or desktop_entry;
 *
 * @param reset - whether to reset
 *
 * @return void*
 */

void  *Config::get_dentry(int type, bool reset)
{
    PDEBUG ("enter, type: %d\n", (int)type);

    unsigned int *idx = NULL;
    void *ret = NULL;
    switch (type) {
    case 0: {
        idx = &sentry_pos;
        break;
    }
    case 1: {
        idx = &dentry_pos;
        break;
    }
    default:
        goto end;
    }

    if (reset ) {
        *idx = 0;
        goto end;
    }
    else {
        if (type == 0) {
            if (*idx == server_list.size()) {
                goto end;
            }
            ret =  &server_list[*idx];
        }
        else {
            if (*idx == dentry_list.size()) {
                goto end;
            }
            ret = &dentry_list[*idx];
        }
        (*idx) ++;
    }
end:
    PDEBUG ("leave: %p\n", ret);
    return ret;
}


Config config;
