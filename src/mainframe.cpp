#include "mainframe.h"
#include "utils.h"

#include "resources/bookmark_add.xpm"
#include "resources/bookmark_mgt.xpm"
#include "resources/buttons/btn_terminal.xpm"
#include "resources/mimetype/folder.xpm"
#include "resources/wcmd.xpm"
#include "resources/arch_add.xpm"
#include "resources/arch_extract.xpm"

#include <wx/log.h>


/**
 * Build main frame.
 *
 * @param title -  title to be displayed.
 *
 * @param args - Character args
 * @return MainFrame  just created.
 */
MainFrame::MainFrame(const wxString& title, char ** args): \
    wxFrame( NULL, -1, title, wxDefaultPosition, wxSize(1200,850))
{
    // Set Size;
    read_set_size();
    sizer = new wxBoxSizer(wxVERTICAL);

    // Menubar
    create_menubar();

    // Toobar
    create_toolbar();
    create_displayer(args);

    this->SetSizer(sizer);
    SetIcon(wxIcon(wcmd, wxBITMAP_TYPE_XPM));
    CreateStatusBar(1);
    SetStatusText(wxT("Welcome to wxCommandor!"));
    update_status();
    Centre();
}

/**
 * Reads configuration about size of window, and restore it.
 * @return void
 */
void MainFrame::read_set_size()
{
    int x, y;
    string tmp = config.get_config("auto_size_x");
    if (tmp.empty())
        x = 1200;
    else
        x = atoi(tmp.c_str());

    tmp = config.get_config("auto_size_y");
    if (tmp.empty())
        y = 850;
    else
        y = atoi(tmp.c_str());

    SetSize(wxSize(x, y));
}

/**
 * Creates toolbar.
 * @return void
 */
void MainFrame::create_toolbar()
{
    wxSizer *hbox = new wxBoxSizer(wxHORIZONTAL);
    wxBitmapButton *button = \
        new wxBitmapButton(this, ID_View,
                           wxArtProvider::GetBitmap(_("gtk-find-and-replace"),
                                                    wxART_MENU));
    hbox->Add(button, 0, wxEXPAND|wxLEFT|wxRIGHT, 10);

    button = new wxBitmapButton(this, wxID_EDIT,
                                wxArtProvider::GetBitmap(_("gtk-edit"),
                                                         wxART_MENU));
    hbox->Add(button, 0, wxEXPAND|wxLEFT|wxRIGHT, 10);

    button = new wxBitmapButton(this, wxID_COPY,
                                wxArtProvider::GetBitmap(wxART_COPY));
    hbox->Add(button, 0, wxEXPAND|wxLEFT|wxRIGHT, 10);

    button = new wxBitmapButton(this, ID_Move,
                                wxArtProvider::GetBitmap(wxART_CUT));

    hbox->Add(button, 0, wxEXPAND|wxLEFT|wxRIGHT, 10);

    button = new wxBitmapButton(this, wxID_DELETE,
                                wxArtProvider::GetBitmap(wxART_DELETE));
    hbox->Add(button, 0, wxEXPAND|wxLEFT|wxRIGHT, 10);

    button = new wxBitmapButton(this, ID_Terminal, wxBitmap(btn_terminal));
    hbox->Add(button, 0, wxEXPAND|wxLEFT|wxRIGHT, 10);

    config.get_dentry(DENTRY, true);
    desktop_entry *entry;
    long type;
    while ((entry = (desktop_entry *)config.get_dentry(DENTRY, false)) != NULL) {
        type = string2type(entry->icon);
        if (type != -1) {
            ;
        }
        PDEBUG ("Buttons will be created! Exec: %s\n",
            entry->name.c_str());

    }
    sizer->Add(hbox, 0, wxEXPAND|wxTOP|wxBOTTOM, 2);
}

/**
 * Creates main window: the FileDisplayer.
 *
 * @param args - Character args
 * @return void
 */
void MainFrame::create_displayer(char **args)
{
    wxSplitterWindow *sp = new wxSplitterWindow(this, -1, wxDefaultPosition);
    wxString path0, path1;
    if (args[0] && strlen(args[0])) {
        path0 = char2wxstr(args[0]);
        if (!wxDirExists(path0))
            path0.Clear();
    }

    if (args[1] && strlen(args[1])) {
        path1 = char2wxstr(args[1]);
        if (!wxDirExists(path1))
            path1.Clear();
    }
    sp1 = new FSDisplayPane(sp, ID_Sp1, path0);
    sp2 = new FSDisplayPane(sp, ID_Sp2, path1);
    sp->SplitVertically(sp1, sp2);
    sp->Show(true);
    sp1->SetFocus();
    active_id = ID_Sp1;
    sizer->Add(sp, 1, wxEXPAND|wxALL, 5);
}

/**
 * Deconstructor.
 * @return void
 */
MainFrame::~MainFrame()
{
    wxSize size = GetSize();
    int x = size.GetWidth();
    int y = size.GetHeight();
    char tmp[8] = {'\0'};
    sprintf(tmp, "%d", x);
    config.set_config("auto_size_x", string(tmp));
    memset(tmp, 0, 8);
    sprintf(tmp, "%d", y);
    config.set_config("auto_size_y", string(tmp));
    config.set_config("auto_last_path_l", string(sp1->get_cwd().char_str()));
    config.set_config("auto_last_path_r", string(sp2->get_cwd().char_str()));
    config.dump2file();
}

void MainFrame::update_status()
{
    wxString path(_("Active Directory: "));
    if (active_id == ID_Sp1) {
        path += str2wxstr(sp1->get_cwd());
    }
    else{
        path += str2wxstr(sp2->get_cwd());
    }
    wxLogStatus(path);
}

/**
 * Creates menubar.
 * @return void
 */
void MainFrame::create_menubar()
{
    wxMenu *menu = new wxMenu;
    menuBar = new wxMenuBar;

    // File
    menu->Append(wxID_EXIT);// ID_Quit, _("E&xit") );

    menuBar->Append( menu, _("&File") );

    // View
    menu = new wxMenu;
    menu_item_view_hidden = menu->AppendCheckItem(ID_View_ShowHidden,
                                                  _("&Show Hidden Files"));
    menuBar->Append(menu, _("&View") );

    // Edit
    menu = new wxMenu;
    menu->Append(wxID_PREFERENCES);
    menuBar->Append(menu, _("&Edit") );

    bookmark_menu = new wxMenu;
    menuitem = new wxMenuItem(bookmark_menu, ID_BookmarkAdd,
                                          _("Add Current Directory"));
    menuitem->SetBitmap(wxBitmap(bookmark_add));
    bookmark_menu->Append(menuitem);

    menuitem = new wxMenuItem(bookmark_menu, ID_BookmarkEdit,
                              _("Edit Bookmarks"));
    menuitem->SetBitmap(wxBitmap(bookmark_mgt));
    bookmark_menu->Append(menuitem);
    bookmark_menu->AppendSeparator();
    if (! bookmarks.empty()) {
        unsigned int i;
        for (i = 0; i < bookmarks.size(); i++) {
            Append_Bookmark(ID_BookmarkAdd + i + 1, bookmarks[i]);
        }
    }
    menuBar->Append(bookmark_menu, _("&Bookmarks"));

    menu = new wxMenu;
    menuitem = new wxMenuItem(menu, ID_Compress,
                              _("Compress ..."));
    menuitem->SetBitmap(wxBitmap(arch_add));
    menu->Append(menuitem);

    menuitem = new wxMenuItem(menu, ID_Decompress,
                              _("Decompress ..."));
    menuitem->SetBitmap(wxBitmap(arch_extract));
    menu->Append(menuitem);
    menuBar->Append(menu, _("&Actions"));

    menu = new wxMenu;
    menuitem = new wxMenuItem(menu, ID_Conn_FTP,
                              _("FTP Server"));
    menuitem->SetBitmap(wxBitmap(arch_add));
    menu->Append(menuitem);

    menuitem = new wxMenuItem(menu, ID_Conn_SFTP,
                              _("SFTP Server"));
    menuitem->SetBitmap(wxBitmap(arch_add));
    menu->Append(menuitem);

    submenu_ssh = new wxMenu();
    // menuitem = new wxMenuItem(submenu, -1, _("test"));
    // submenu->Append(menuitem);
    // menuitem = new wxMenuItem(submenu, -1, _("test1"));
    // submenu->Append(menuitem);
    menu->AppendSubMenu(submenu_ssh, _("SSH Server"));

    menuitem = new wxMenuItem(menu, ID_Conn_SMB,
                              _("SMB Server"));
    menuitem->SetBitmap(wxBitmap(arch_add));
    menu->Append(menuitem);

    menuBar->Append(menu, _("&Net"));

    Append_Servers();

    // Help
    menu = new wxMenu;
    menu->Append(wxID_ABOUT);
    menuBar->Append(menu, _("&Help") );
    SetMenuBar( menuBar );
}

/**
 * Add bookmark into record.
 * @return void
 */
void MainFrame::BookmarAdd()
{
    Freeze();
    string path = string(get_sp()->get_cwd().char_str());
    bookmarks.push_back(path);
    Append_Bookmark(bookmarks.size() + ID_BookmarkAdd + 1, path);
    Thaw();
}

void MainFrame::Append_Servers()
{
    PDEBUG ("enter\n");

    int i = ID_Server_Start;
    server_entry *entry = NULL;
    wxMenu *submenu = NULL;
    wxMenuItem *menu_item = NULL;
    config.get_dentry(SENTRY, true);
    while ((entry = (server_entry *)config.get_dentry(SENTRY, false)) != NULL) {
        switch (entry->type) {
            PDEBUG ("ip: %s, type: %d\n", entry->ip.c_str(),
                    (int)entry->type);
        case SSH: {
            submenu = submenu_ssh;
            break;
        }
        case SFTP: {
            submenu = submenu_sftp;
            break;
        }
        case FTP: {
            submenu = submenu_ftp;
            break;
        }
        case SMB: {
            submenu = submenu_smb;
            break;
        }
        default:
            fprintf(stderr, "ERROR: Unkonwn type: %d\n",
                    (int)entry->type);
            continue;
            break;
        }
        menu_item = new wxMenuItem(submenu, i, str2wxstr(entry->name));
        submenu->Append(menu_item);
        Connect(i, wxEVT_COMMAND_MENU_SELECTED,
                wxCommandEventHandler( MainFrame::OnServerActivated));

        i++;
    }
    menuBar->Refresh();
    PDEBUG ("leave.\n");

    return;
}

void MainFrame::OnServerActivated(wxCommandEvent &evt)
{
    PDEBUG ("enter\n");

    int idx =  evt.GetId() - ID_Server_Start;
    PDEBUG ("idx: %d\n", idx);

    server_entry *entry = config.get_sentry(idx);
    PDEBUG ("A: %p\n", entry);
    PDEBUG ("ip: %d\n",  (int)entry->type);
    // PDEBUG ("ip: %s\n",  entry->ip.c_str());

    if (entry && !entry->ip.empty()) {
        switch (entry->type) {
        case SSH: {
            string cmd = "xterm -e ssh " + entry->ip ;
            if (!entry->user.empty()) {
                cmd + " -l " + entry->user;
            }
            PDEBUG ("cmd: %s\n", cmd.c_str());

            get_sp()->do_async_execute(str2wxstr(cmd));
            break;
        }

        default:
            break;
        }
    }
    PDEBUG ("leave\n");

}

void MainFrame::Append_Bookmark(int id, string item)
{
    menuitem = new wxMenuItem(bookmark_menu, id,
                              wxString(item.c_str(), wxConvUTF8));
    menuitem->SetBitmap(wxBitmap(folder));
    bookmark_menu->Append(menuitem);
    Connect(id, wxEVT_COMMAND_MENU_SELECTED,
            wxCommandEventHandler( MainFrame::OnBookmarkClicked));
    menuBar->Refresh();
}

void MainFrame::OnBookmarkClicked(wxCommandEvent &evt)
{
    int idx =  evt.GetId() - ID_BookmarkAdd - 1;
    get_sp()->set_cwd(str2wxstr(bookmarks[idx]));
    get_sp()->update_list(-1);
    Thaw();
}


void MainFrame::OnQuit(wxCommandEvent& WXUNUSED(event))
{
    Close(TRUE);
}

void MainFrame::OnAbout(wxCommandEvent& WXUNUSED(event))
{
    wxAboutDialogInfo info;
    info.SetName(_("wcmd"));
    info.SetVersion(_("0.1.0"));
    info.SetDescription(_("wcmd is a simple file manaber writen with\
 wxwidgets."));
    info.SetCopyright(_T("(C) 2010 Yang, Yingchao <yangyingchao@gmail.com>"));
    wxAboutBox(info);
}

void MainFrame::OnOption (wxCommandEvent& WXUNUSED(event))
{
    PrefDialog *pref = new PrefDialog(this, _("Preferences"));
    if (pref->ShowModal() == wxID_OK)
        update_fs();
    delete(pref);
}


void MainFrame::OnView(wxCommandEvent & event)
{
}

void MainFrame::OnEdit(wxCommandEvent & event)
{
    get_sp()->edit_file();
}

void MainFrame::OnMove(wxCommandEvent & event)
{
    copy_or_move(false);
}
void MainFrame::OnCopy(wxCommandEvent & event)
{
    copy_or_move();
}


void MainFrame::OnDelete(wxCommandEvent & event)
{

    get_sp()->delete_files();
    event.Skip();
}

void MainFrame::ShowHidden()
{
    if (config.get_config("show_hidden") == "false") {
        config.set_config("show_hidden", "true");
        menu_item_view_hidden -> Check(true);
    }
    else {
        config.set_config("show_hidden", "false");
        menu_item_view_hidden -> Check(false);
    }

    update_fs();
}

void MainFrame::Show_Hidden(wxCommandEvent &evt)
{
    ShowHidden();
    evt.Skip();
}

void MainFrame::OnBookmarkAdd(wxCommandEvent &evt)
{
    BookmarAdd();
    evt.Skip();
}


void MainFrame::OnBookmarkEdit(wxCommandEvent &evt)
{
    BookmarkManage *mgt = new BookmarkManage(this);
    int ret;
    ret = mgt->ShowModal();
    delete(mgt);
    if (ret == ID_BookmarkRedraw) {
        OnBookmarkEdit(evt);
    }
    else {
        unsigned int i;
        wxMenuItemList list = bookmark_menu->GetMenuItems();
        if (!list.empty()) // Remove old menu items, but leave the first 2.
            for (i = 3; i < list.size(); i++)
                bookmark_menu->Remove((wxMenuItem *)list[i]);

        if (! bookmarks.empty()) // Add new menu items.
            for (i = 0; i < bookmarks.size(); i++)
                Append_Bookmark(ID_BookmarkAdd + i + 1, bookmarks[i]);
    }
}

void MainFrame::compare_items()
{
    wxString cmd = str2wxstr(config.get_config("diff_tool"));
    if (cmd.IsEmpty()) {
        wxMessageDialog *ddlg = \
            new wxMessageDialog(this, _("Diff tool is not configured!"),
                                _("Error"), wxOK);
        ddlg->ShowModal();
        delete(ddlg);
        return ;
    }
    cmd += _(" \"") + sp1->get_selected_item() + _("\" \"") +   \
        sp2->get_selected_item() + _("\"");

    get_sp()->do_async_execute(str2wxstr(cmd));
    return;
}

void MainFrame::OnThreadCompletion(wxCommandEvent& event)
{

    update_fs();
}

void MainFrame::open_in_other()
{
    wxString path = get_sp()->get_selected_item();
    get_sp_o()->set_cwd(path);
    get_sp_o()->update_list(-1);
    get_sp_o()->focus_list();
    exchange_sp();
}

FSDisplayPane *MainFrame::get_sp()
{
    if (active_id == ID_Sp1)
        return sp1;
    else
        return sp2;
}

wxString MainFrame::get_o_wd()
{
    return get_sp_o()->get_cwd();
}

void MainFrame::exchange_sp()
{
    if (active_id == ID_Sp1) {
        active_id = ID_Sp2;
        sp2->set_focus();
    }
    else{
        active_id = ID_Sp1;
        sp1->set_focus();
    }
    update_status();
}

FSDisplayPane *MainFrame::get_sp_o()
{
    if (active_id == ID_Sp1)
        return sp2;
    else
        return sp1;
}

void MainFrame::update_fs(int idx1, int idx2, wxWindowID id)
{
    if (id == -1) // Get active_id first, it will be changed in "update_list"
        id = active_id;
    sp1->update_list(idx1==-1?sp1->cur_idx:idx1);
    sp2->update_list(idx2==-1?sp2->cur_idx:idx2);
    // Restore active_id.
    if (id == ID_Sp1){
        sp1->SetFocus();
        active_id = id;
    }
    else {
        active_id = id;
        sp2->SetFocus();
    }
}

void MainFrame::set_active_sp(wxWindowID id)
{

    active_id = id;
}

void MainFrame::OpenTerminal(wxCommandEvent &evt)
{
    get_sp()->open_terminal();
}

void MainFrame::copy_or_move(bool copy)
{
    vector<ItemEntry *> src_list;
    wxString dest = get_sp_o()->get_cwd();
    wxString src;
    if (get_sp()->get_selected_files(src_list) || src_list.empty()) {
        wxMessageDialog *dlg = \
            new wxMessageDialog(this, _("Failed to get selected files!"),
                                _("Error"), wxOK);
        dlg->ShowModal();
        delete(dlg);
        return ;
    }
    vector<ItemEntry *>::iterator iter;
    for (iter = src_list.begin(); iter < src_list.end(); iter++) {
        dest += _("/");
        src = (*iter)->get_fullpath();
        copy_or_move_single(src, dest, copy);
    }
    int idx = get_sp()->cur_idx;
    if (!copy) {
        idx--;
        if (idx < 0)
            idx = 0;
    }
}

/**
 * Copy or move file from src to dst.
 * @param src -  src
 * @param dst -  dst
 * @param copy - Flag copy, true to copy, false to move.
 * @return int: 0  - success.
 *              -2 - Source or desty is empty.
 */
int MainFrame::copy_or_move_single(wxString &src, wxString &dest, bool copy)
{

    int ret = 0;
    if (src.IsEmpty() || dest.IsEmpty()) {
        fprintf(stderr, "ERROR: Source or desty is empty!\n");
        return -2;
    }
    wxFileName fn(src);
    wxString fake_dest(fn.GetFullName());
    wxMessageDialog *dlg;
    wxWindowID id;
    fake_dest = dest + _("/") + fake_dest;
    wxString msg(fake_dest);

    if (wxFileExists(fake_dest)) {
        msg += _(" already exited!\n\nOverwrite?");
        dlg = new wxMessageDialog(this, msg, _("Overwrite"));
        id = dlg->ShowModal();
        if (id == wxID_CANCEL){
            delete(dlg);
            return 0;
        }
    }
    wxString cmd;
    cmd =  _("cp -aRf \"") + src + _("\"  \"") +  dest + _("\"");
    if (!copy)
        cmd = _( "mv \"") + src + _("\"  \"") +  dest + _("\"");
    ret = get_sp()->do_async_execute(cmd);
    return ret;
}

void MainFrame::show_file_info()
{
    wxString path = get_sp()->get_selected_item();
    wxString cmd = _("xterm -e \" file ") + path + _(" | less \"");
    get_sp()->do_async_execute(cmd);
}


/**
 * Creates a soft link of selected file(s).
 * @return void
 */
void MainFrame::create_softlink()
{
    vector<ItemEntry *> list;
    list.clear();
    if((get_sp())->get_selected_files(list) || list.empty()) {
        wxLogStatus(_("Failed to get selected files!"));
        wxLogMessage(_("Failed to get selected files!"));
        return ;
    }

    wxString cmd;
    wxString dst_dir = get_sp_o()->get_cwd();
    unsigned int i;

    for (i = 0; i < list.size(); ++i) {
        cmd = _("ln -sf \"") + (list[i])->get_fullpath() + _("\" \"") +  \
            dst_dir + _("/\"");
        get_sp()->do_async_execute(cmd);
    }

    return;
}

/**
 * Compares the selected files.
 * @return void
 */
void MainFrame::compress_files(wxCommandEvent &evt)
{
    get_sp()->compress_files();
    evt.Skip();
}

/**
 * Decompress the selected file.
 * @return void
 */
void MainFrame::decompress_files(wxCommandEvent &evt)
{
    get_sp()->decompress_files();
    evt.Skip();
}



DEFINE_EVENT_TYPE(wxEVT_MY_EVENT)


BEGIN_EVENT_TABLE(MainFrame, wxFrame)
EVT_MENU(wxID_EXIT, MainFrame::OnQuit)
EVT_MENU(wxID_ABOUT, MainFrame::OnAbout)
EVT_MENU(wxID_PREFERENCES, MainFrame::OnOption)
EVT_MENU(ID_BookmarkAdd, MainFrame::OnBookmarkAdd)
EVT_MENU(ID_BookmarkEdit, MainFrame::OnBookmarkEdit)
EVT_MENU(ID_View_ShowHidden, MainFrame::Show_Hidden)
EVT_MENU(ID_Compress, MainFrame::compress_files)
EVT_MENU(ID_Decompress, MainFrame::decompress_files)
EVT_BUTTON(ID_View, MainFrame::OnView)
EVT_BUTTON(wxID_EDIT, MainFrame::OnEdit)
EVT_BUTTON(wxID_COPY, MainFrame::OnCopy)
EVT_BUTTON(wxID_DELETE, MainFrame::OnDelete)
EVT_BUTTON(ID_Terminal, MainFrame::OpenTerminal)
EVT_COMMAND  (-1, wxEVT_MY_EVENT, MainFrame::OnThreadCompletion)
END_EVENT_TABLE()



/*
 * Editor modelines
 *
 * Local Variables:
 * c-basic-offset: 4
 * tab-width: 4
 * indent-tabs-mode: nil
 * End:
 *
 * ex: set shiftwidth=4 tabstop=4 expandtab
 * :indentSize=4:tabSize=4:noTabs=true:
 */
