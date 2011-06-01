#include "mainframe.h"
#include "utils.h"

#include "resources/bookmark_add.xpm"
#include "resources/bookmark_mgt.xpm"
#include "resources/buttons/btn_terminal.xpm"
#include "resources/mimetype/folder.xpm"
#include "resources/wxcommandor.xpm"
#include <wx/log.h>

wxLog *logger;


MainFrame::MainFrame(const wxString& title, char ** args): \
    wxFrame( NULL, -1, title, wxDefaultPosition, wxSize(1200,850))
{
    // Init logger
    logger = new wxLogWindow(this, _("WCMD Debug Info"), false);
    if (logger == NULL)
    {
        wxLogFatalError(_("Failed to initiate logger."));
    }
    wxLog::SetActiveTarget(logger);
    // Set Size;
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
    // Menubar
    create_menubar();

    wxSizer *sizer = new wxBoxSizer(wxVERTICAL);
    // Toobar
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

    sizer->Add(hbox, 0, wxEXPAND|wxTOP|wxBOTTOM, 2);

    fs = new FileSelector(this, args);
    sp1 = fs->sp1;
    sp2 = fs->sp2;
    active_id = ID_Sp1;
    sizer->Add(fs, 1, wxEXPAND|wxALL, 5);
    this->SetSizer(sizer);
    SetIcon(wxIcon(wxcommandor, wxBITMAP_TYPE_XPM));
    CreateStatusBar(1);
    SetStatusText(wxT("Welcome to wxCommandor!"));
    update_status();
    Centre();
}

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
    menu_item_view_debug = menu->AppendCheckItem(ID_View_ShowDebug,
                                                  _("&Show Debug Info"));
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
    menuBar->Append(bookmark_menu, _("Bookmarks"));

    // Help
    menu = new wxMenu;
    menu->Append(wxID_ABOUT);
    menuBar->Append(menu, _("&Help") );
    SetMenuBar( menuBar );
}

void MainFrame::BookmarAdd()
{
    Freeze();
    string path = string(get_sp()->get_cwd().char_str());
    bookmarks.push_back(path);
    Append_Bookmark(bookmarks.size() + ID_BookmarkAdd + 1, path);
    Thaw();
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

void MainFrame::Show_Debug(wxCommandEvent &evt)
{

    if (menu_item_view_debug->IsChecked()) {
        wxLogMessage(_("Log enabled!"));
        ((wxLogWindow *)logger)->Show(true);
    }
    else {
        wxLogMessage(_("Log disabled!"));
        ((wxLogWindow *)logger)->Show(false);
    }
    evt.Skip();
    return;
}


DEFINE_EVENT_TYPE(wxEVT_MY_EVENT)


BEGIN_EVENT_TABLE(MainFrame, wxFrame)
EVT_MENU(wxID_EXIT, MainFrame::OnQuit)
EVT_MENU(wxID_ABOUT, MainFrame::OnAbout)
EVT_MENU(wxID_PREFERENCES, MainFrame::OnOption)
EVT_MENU(ID_BookmarkAdd, MainFrame::OnBookmarkAdd)
EVT_MENU(ID_BookmarkEdit, MainFrame::OnBookmarkEdit)
EVT_MENU(ID_View_ShowHidden, MainFrame::Show_Hidden)
EVT_MENU(ID_View_ShowDebug, MainFrame::Show_Debug)
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
