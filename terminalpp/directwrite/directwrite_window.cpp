#if (defined ARCH_WINDOWS && defined RENDERER_NATIVE)
#include "directwrite_window.h"

#include "directwrite_font.h"

namespace tpp {

    LRESULT CALLBACK DirectWriteWindow::CaptionButtonProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam, UINT_PTR subclassId, DWORD_PTR refData) {
        DirectWriteWindow * window = reinterpret_cast<DirectWriteWindow*>(refData);
        int const id = GetDlgCtrlID(hWnd);
        if (window != nullptr) {
            if (msg == WM_MOUSEMOVE) {
                if (window->hoveredButtonId_ != id) {
                    int const previous = window->hoveredButtonId_;
                    window->hoveredButtonId_ = id;
                    window->invalidateTitleButton(previous);
                    InvalidateRect(hWnd, nullptr, FALSE);
                }
                int closeHoverId = -1;
                if (id >= TabButtonId && id < TabButtonId + static_cast<int>(window->tabButtons_.size())) {
                    RECT rect{};
                    GetClientRect(hWnd, &rect);
                    if (GET_X_LPARAM(lParam) >= rect.right - static_cast<int>(46 * window->dpiFactor_))
                        closeHoverId = id;
                }
                if (window->hoveredCloseButtonId_ != closeHoverId) {
                    int const previousClose = window->hoveredCloseButtonId_;
                    window->hoveredCloseButtonId_ = closeHoverId;
                    window->invalidateTitleButton(previousClose);
                    window->invalidateTitleButton(closeHoverId);
                }
                TRACKMOUSEEVENT track{};
                track.cbSize = sizeof(track);
                track.dwFlags = TME_LEAVE;
                track.hwndTrack = hWnd;
                TrackMouseEvent(&track);
            } else if (msg == WM_MOUSELEAVE) {
                if (window->hoveredButtonId_ == id) {
                    window->hoveredButtonId_ = -1;
                    window->invalidateTitleButton(id);
                }
                if (window->hoveredCloseButtonId_ == id) {
                    window->hoveredCloseButtonId_ = -1;
                    window->invalidateTitleButton(id);
                }
            } else if (msg == WM_LBUTTONUP && id >= TabButtonId && id < TabButtonId + static_cast<int>(window->tabButtons_.size())) {
                RECT rect{};
                GetClientRect(hWnd, &rect);
                if (GET_X_LPARAM(lParam) >= rect.right - static_cast<int>(46 * window->dpiFactor_)) {
                    PostMessageW(window->hWnd_, WM_APP + 0x33, static_cast<WPARAM>(id), 0);
                    return 0;
                }
            } else if (msg == WM_NCDESTROY) {
                RemoveWindowSubclass(hWnd, CaptionButtonProc, subclassId);
            }
        }
        MARK_AS_UNUSED(wParam);
        return DefSubclassProc(hWnd, msg, wParam, lParam);
    }

    HWND DirectWriteWindow::createTitleButton(int id) {
        HWND button = CreateWindowExW(0, L"BUTTON", L"", WS_CHILD | BS_OWNERDRAW,
            0, 0, 1, captionControlHeight(), hWnd_,
            reinterpret_cast<HMENU>(static_cast<INT_PTR>(id)),
            DirectWriteApplication::Instance()->hInstance_, nullptr);
        if (button != nullptr)
            SetWindowSubclass(button, CaptionButtonProc, 1, reinterpret_cast<DWORD_PTR>(this));
        return button;
    }

    void DirectWriteWindow::showDropDownMenu() {
        if (dropDownItems_.empty() || dropDownButton_ == nullptr)
            return;

        HMENU menu = CreatePopupMenu();
        if (menu == nullptr)
            return;

        std::vector<std::function<void()>> actions;
        std::function<void(HMENU, std::vector<Window::TabBarMenuItem> const &)> appendItems;
        appendItems = [&](HMENU parent, std::vector<Window::TabBarMenuItem> const & items) {
            for (Window::TabBarMenuItem const & item : items) {
                utf16_string title = UTF8toUTF16(item.title);
                if (!item.children.empty()) {
                    HMENU submenu = CreatePopupMenu();
                    if (submenu == nullptr)
                        continue;
                    appendItems(submenu, item.children);
                    AppendMenuW(parent, MF_POPUP | MF_STRING, reinterpret_cast<UINT_PTR>(submenu), title.c_str());
                } else {
                    UINT flags = MF_STRING;
                    if (!item.enabled || !item.activate)
                        flags |= MF_GRAYED;
                    UINT const id = DropDownMenuItemId + static_cast<UINT>(actions.size());
                    AppendMenuW(parent, flags, id, title.c_str());
                    actions.push_back(item.activate);
                }
            }
        };
        appendItems(menu, dropDownItems_);

        RECT buttonRect{};
        GetWindowRect(dropDownButton_, &buttonRect);
        SetForegroundWindow(hWnd_);
        UINT const selected = TrackPopupMenu(
            menu, TPM_LEFTALIGN | TPM_TOPALIGN | TPM_RIGHTBUTTON | TPM_RETURNCMD,
            buttonRect.left, buttonRect.bottom, 0, hWnd_, nullptr);
        DestroyMenu(menu);
        PostMessageW(hWnd_, WM_NULL, 0, 0);

        if (selected >= DropDownMenuItemId) {
            size_t const index = selected - DropDownMenuItemId;
            if (index < actions.size() && actions[index])
                actions[index]();
        }
        SetFocus(hWnd_);
    }

    void DirectWriteWindow::setTabBar(std::vector<Window::TabBarItem> tabs, std::function<void()> addTab, std::function<void()> showMenu, std::vector<Window::TabBarMenuItem> dropDownItems) {
        UI_THREAD_ONLY;
        addTabAction_ = std::move(addTab);
        showMenuAction_ = std::move(showMenu);
        dropDownItems_ = std::move(dropDownItems);

        size_t const sharedCount = std::min(tabButtons_.size(), tabs.size());
        for (size_t i = 0; i < sharedCount; ++i)
            tabButtons_[i].item = std::move(tabs[i]);

        while (tabButtons_.size() > tabs.size()) {
            TabButton & tab = tabButtons_.back();
            if (tab.hWnd != nullptr) {
                RemoveWindowSubclass(tab.hWnd, CaptionButtonProc, 1);
                DestroyWindow(tab.hWnd);
            }
            tabButtons_.pop_back();
        }

        for (size_t i = sharedCount; i < tabs.size(); ++i) {
            TabButton tab;
            tab.item = std::move(tabs[i]);
            tab.hWnd = createTitleButton(TabButtonId + static_cast<int>(i));
            tabButtons_.push_back(std::move(tab));
        }

        layoutCaptionControls(!IsZoomed(hWnd_));
        for (TabButton const & tab : tabButtons_) {
            if (tab.hWnd != nullptr)
                InvalidateRect(tab.hWnd, nullptr, FALSE);
        }
    }

    void DirectWriteWindow::invalidateTitleButton(int id) {
        if (id < 0)
            return;
        if (id >= CaptionButtonId && id < CaptionButtonId + 3) {
            InvalidateRect(captionButtons_[id - CaptionButtonId], nullptr, FALSE);
        } else if (id == MenuButtonId) {
            InvalidateRect(menuButton_, nullptr, FALSE);
        } else if (id == AddTabButtonId) {
            InvalidateRect(addTabButton_, nullptr, FALSE);
        } else if (id == DropDownButtonId) {
            InvalidateRect(dropDownButton_, nullptr, FALSE);
        } else if (id >= TabButtonId && id < TabButtonId + static_cast<int>(tabButtons_.size())) {
            InvalidateRect(tabButtons_[id - TabButtonId].hWnd, nullptr, FALSE);
        }
    }

    void DirectWriteWindow::drawTitleBarItem(DRAWITEMSTRUCT * draw) {
        int const id = static_cast<int>(draw->CtlID);
        bool const hot = (draw->itemState & ODS_HOTLIGHT) != 0 || hoveredButtonId_ == id;
        bool const pressed = (draw->itemState & ODS_SELECTED) != 0;
        COLORREF const barColor = RGB(43, 43, 43);
        COLORREF const hoverColor = RGB(62, 62, 62);
        COLORREF background = barColor;
        HBRUSH brush = nullptr;
        HPEN pen = nullptr;
        HFONT font = CreateFontW(-static_cast<int>(14 * dpiFactor_), 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
            DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_DONTCARE, L"Segoe UI");
        HGDIOBJ oldFont = SelectObject(draw->hDC, font);
        SetBkMode(draw->hDC, TRANSPARENT);
        SetTextColor(draw->hDC, RGB(240, 240, 240));

        if (id >= CaptionButtonId && id < CaptionButtonId + 3) {
            int const index = id - CaptionButtonId;
            background = hot || pressed ? (index == 2 && pressed ? RGB(196, 43, 28) : hoverColor) : barColor;
            brush = CreateSolidBrush(background);
            FillRect(draw->hDC, &draw->rcItem, brush);
            DeleteObject(brush);
            wchar_t const* glyphs[] = {L"—", L"□", L"×"};
            DrawTextW(draw->hDC, glyphs[index], 1, &draw->rcItem, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
        } else if (id == MenuButtonId || id == AddTabButtonId || id == DropDownButtonId) {
            if (hot || pressed) {
                brush = CreateSolidBrush(hoverColor);
                FillRect(draw->hDC, &draw->rcItem, brush);
                DeleteObject(brush);
            } else {
                brush = CreateSolidBrush(barColor);
                FillRect(draw->hDC, &draw->rcItem, brush);
                DeleteObject(brush);
            }
            if (id == MenuButtonId) {
                int const cx = (draw->rcItem.left + draw->rcItem.right) / 2;
                int const cy = (draw->rcItem.top + draw->rcItem.bottom) / 2;
                pen = CreatePen(PS_SOLID, 1, RGB(230, 230, 230));
                HGDIOBJ oldPen = SelectObject(draw->hDC, pen);
                for (int line = -1; line <= 1; ++line) {
                    MoveToEx(draw->hDC, cx - 7, cy + line * 4, nullptr);
                    LineTo(draw->hDC, cx + 7, cy + line * 4);
                }
                SelectObject(draw->hDC, oldPen);
                DeleteObject(pen);
            } else if (id == AddTabButtonId) {
                DrawTextW(draw->hDC, L"+", 1, &draw->rcItem, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
            } else {
                int const cx = draw->rcItem.left + static_cast<int>(16 * dpiFactor_);
                int const cy = (draw->rcItem.top + draw->rcItem.bottom) / 2;
                pen = CreatePen(PS_SOLID, std::max(1, static_cast<int>(dpiFactor_)), RGB(220, 220, 220));
                HGDIOBJ oldPen = SelectObject(draw->hDC, pen);
                MoveToEx(draw->hDC, cx - 3, cy - 1, nullptr);
                LineTo(draw->hDC, cx, cy + 2);
                LineTo(draw->hDC, cx + 3, cy - 1);
                SelectObject(draw->hDC, oldPen);
                DeleteObject(pen);
            }
        } else if (id >= TabButtonId && id < TabButtonId + static_cast<int>(tabButtons_.size())) {
            TabButton const & tab = tabButtons_[id - TabButtonId];
            brush = CreateSolidBrush(barColor);
            FillRect(draw->hDC, &draw->rcItem, brush);
            DeleteObject(brush);
            if (tab.item.active) {
                RECT active = draw->rcItem;
                active.left += 3;
                active.right -= 3;
                active.top += std::max(3, static_cast<int>(active.bottom - active.top) / 5);
                brush = CreateSolidBrush(RGB(12, 12, 12));
                pen = CreatePen(PS_SOLID, 1, RGB(12, 12, 12));
                HGDIOBJ oldBrush = SelectObject(draw->hDC, brush);
                HGDIOBJ oldPen = SelectObject(draw->hDC, pen);
                Rectangle(draw->hDC, active.left, active.top, active.right, active.bottom);
                SelectObject(draw->hDC, oldPen);
                SelectObject(draw->hDC, oldBrush);
                DeleteObject(pen);
                DeleteObject(brush);
            } else if (hot) {
                RECT hover = draw->rcItem;
                hover.left += 3;
                hover.right -= 3;
                hover.top += std::max(3, static_cast<int>(hover.bottom - hover.top) / 5);
                brush = CreateSolidBrush(RGB(55, 55, 55));
                pen = CreatePen(PS_SOLID, 1, RGB(55, 55, 55));
                HGDIOBJ oldBrush = SelectObject(draw->hDC, brush);
                HGDIOBJ oldPen = SelectObject(draw->hDC, pen);
                Rectangle(draw->hDC, hover.left, hover.top, hover.right, hover.bottom);
                SelectObject(draw->hDC, oldPen);
                SelectObject(draw->hDC, oldBrush);
                DeleteObject(pen);
                DeleteObject(brush);
            }
            RECT label = draw->rcItem;
            label.left += static_cast<int>(44 * dpiFactor_);
            label.right -= static_cast<int>(48 * dpiFactor_);
            utf16_string title = UTF8toUTF16(tab.item.title);
            DrawTextW(draw->hDC, title.c_str(), static_cast<int>(title.size()), &label, DT_LEFT | DT_VCENTER | DT_SINGLELINE | DT_END_ELLIPSIS);
            if (tab.item.active || hot) {
                RECT closeRect = draw->rcItem;
                closeRect.right -= static_cast<int>(5 * dpiFactor_);
                closeRect.left = closeRect.right - static_cast<int>(40 * dpiFactor_);
                if (hoveredCloseButtonId_ == id) {
                    RECT closeHover = draw->rcItem;
                    closeHover.left = closeHover.right - static_cast<int>(40 * dpiFactor_);
                    closeHover.right -= static_cast<int>(12 * dpiFactor_);
                    closeHover.top += static_cast<int>(8 * dpiFactor_);
                    closeHover.bottom -= static_cast<int>(8 * dpiFactor_);
                    brush = CreateSolidBrush(RGB(68, 68, 68));
                    FillRect(draw->hDC, &closeHover, brush);
                    DeleteObject(brush);
                }
                SetTextColor(draw->hDC, RGB(230, 230, 230));
                DrawTextW(draw->hDC, L"×", 1, &closeRect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
            }
        }

        SelectObject(draw->hDC, oldFont);
        DeleteObject(font);
    }

    void DirectWriteWindow::setTitle(std::string const & value) {
        RendererWindow::setTitle(value);
        // actually change the title since we are in the UI thread now
        utf16_string t = UTF8toUTF16(value);
        // ok, on windows wchar_t and char16_t are the same (see helpers/char.h)
        SetWindowTextW(hWnd_, t.c_str());
    }

    void DirectWriteWindow::setIcon(Window::Icon icon) {
        RendererWindow::setIcon(icon);
        DirectWriteApplication *app = DirectWriteApplication::Instance();
        WPARAM iconHandle;
        switch (icon) {
            case Icon::Notification:
                iconHandle = reinterpret_cast<WPARAM>(app->iconNotification_);
                break;
            default:
                iconHandle = reinterpret_cast<WPARAM>(app->iconDefault_);
                break;
        }
        PostMessage(hWnd_, WM_SETICON, ICON_BIG, iconHandle);
        PostMessage(hWnd_, WM_SETICON, ICON_SMALL, iconHandle);
    }

    void DirectWriteWindow::startWindowDrag() {
        ReleaseCapture();
        POINT point{};
        GetCursorPos(&point);
        SendMessageW(hWnd_, WM_NCLBUTTONDOWN, HTCAPTION, MAKELPARAM(point.x, point.y));
        ReleaseCapture();
        if (rendererMouseCaptured()) {
            mouseLeaveTracked_ = false;
            ui::Renderer::mouseOut();
        }
        mouseButtonsDown_ = 0;
    }

    void DirectWriteWindow::setFullscreen(bool value) {
        if (fullscreen() != value) {
            if (value == true) {
                MONITORINFO mInfo = { sizeof(mInfo) };
                if (GetWindowPlacement(hWnd_, &wndPlacement_) &&
                    GetMonitorInfo(MonitorFromWindow(hWnd_, MONITOR_DEFAULTTOPRIMARY), &mInfo)) {
                    SetWindowLong(hWnd_, GWL_STYLE, WS_POPUP | WS_CLIPCHILDREN);
                    int width = mInfo.rcMonitor.right - mInfo.rcMonitor.left;
                    int height = mInfo.rcMonitor.bottom - mInfo.rcMonitor.top;
                    SetWindowPos(hWnd_, HWND_TOP, mInfo.rcMonitor.left, mInfo.rcMonitor.top, width, height, SWP_NOOWNERZORDER | SWP_FRAMECHANGED);
                } else {
                    // we are not actually fullscreen
                    return;
                }
            } else {
                SetWindowLong(hWnd_, GWL_STYLE, windowedStyle_);
                SetWindowPlacement(hWnd_, &wndPlacement_);
                SetWindowPos(hWnd_, nullptr, 0, 0, 0, 0,
                    SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER |
                    SWP_NOOWNERZORDER | SWP_FRAMECHANGED);
            }
            for (HWND button : captionButtons_)
                ShowWindow(button, value ? SW_HIDE : SW_SHOWNOACTIVATE);
            ShowWindow(menuButton_, value ? SW_HIDE : SW_SHOWNOACTIVATE);
            ShowWindow(addTabButton_, value ? SW_HIDE : SW_SHOWNOACTIVATE);
            ShowWindow(dropDownButton_, value ? SW_HIDE : SW_SHOWNOACTIVATE);
            for (TabButton const & tab : tabButtons_)
                ShowWindow(tab.hWnd, value ? SW_HIDE : SW_SHOWNOACTIVATE);
            RendererWindow::setFullscreen(value);
        }
    }

    void DirectWriteWindow::requestClipboard(Widget * sender) {
        RendererWindow::requestClipboard(sender);
		std::string result;
		if (OpenClipboard(nullptr)) {
			HANDLE clip = GetClipboardData(CF_UNICODETEXT);
			if (clip) {
				// ok, on windows wchar_t and char16_t are the same (see helpers/char.h)
				utf16_char* data = reinterpret_cast<utf16_char*>(GlobalLock(clip));
				if (data) {
					result = UTF16toUTF8(data);
					GlobalUnlock(clip);
				}
			}
			CloseClipboard();
		}
        pasteClipboard(result);
    }

    void DirectWriteWindow::requestSelection(Widget * sender) {
        RendererWindow::requestSelection(sender);
        DirectWriteApplication * app = DirectWriteApplication::Instance();
        if (app->selectionOwner_ != nullptr)
            pasteSelection(app->selection_);
    }

    void DirectWriteWindow::setClipboard(std::string const & contents) {
        DirectWriteApplication::Instance()->setClipboard(contents);
    }

    void DirectWriteWindow::setSelection(std::string const & contents, Widget * owner) {
        MARK_AS_UNUSED(owner);
        DirectWriteApplication * app = DirectWriteApplication::Instance();
        // first we clear the selection if it belongs to different window
        if (app->selectionOwner_ != nullptr && app->selectionOwner_ != this)
            app->selectionOwner_->clearSelection(nullptr);
        // set the contents and the selection owner to ourselves
        app->selectionOwner_ = this;
        app->selection_ = contents;
    }

    void DirectWriteWindow::clearSelection(Widget * sender) {
        // on windows, this is rather simple, first deal with the application global selection, then call the parent implementation
        DirectWriteApplication * app = DirectWriteApplication::Instance();
        if (app->selectionOwner_ == this) {
            app->selectionOwner_ = nullptr;
            app->selection_.clear();
        }
        RendererWindow::clearSelection(sender);
    }

    DirectWriteWindow::DirectWriteWindow(std::string const & title, int cols, int rows, EventQueue & eventQueue):
        RendererWindow<DirectWriteWindow, HWND>{cols, rows, eventQueue},
        wndPlacement_{ sizeof(wndPlacement_) },
        frameWidth_{0},
        frameHeight_{0},
        font_(nullptr),
        glyphIndices_{nullptr},
        glyphAdvances_{nullptr},
        glyphOffsets_{nullptr},
        mouseLeaveTracked_{false} {
        // determine the system DPI and adjust the desired window size in pixels accordingly (larger text settings will increase the real window size in pixels)
        dpiFactor_ = GetDpiForSystem() / 96.0;
        sizePx_ = sizePx_ * dpiFactor_;
        // create the window with given title
        utf16_string t = UTF8toUTF16(title);
        windowedStyle_ = WS_POPUP | WS_MINIMIZEBOX | WS_MAXIMIZEBOX | WS_CLIPCHILDREN;
        hWnd_ = CreateWindowExW(
            WS_EX_LEFT, // the default
            DirectWriteApplication::Instance()->WindowClassName_, // window class
            // ok, on windows wchar_t and char16_t are the same (see helpers/char.h)
            t.c_str(), // window name (all start as terminal++)
            windowedStyle_,
            CW_USEDEFAULT, // x position
            CW_USEDEFAULT, // y position
            sizePx().width(),
            sizePx().height(),
            nullptr, // handle to parent
            nullptr, // handle to menu 
            DirectWriteApplication::Instance()->hInstance_, // module handle
            this // lParam for WM_CREATE message
        );
        createCaptionControls();
        // initialize the rendering structures
        D2D1_SIZE_U dxsize = D2D1::SizeU(sizePx().width(), sizePx().height());
        OSCHECK(SUCCEEDED(DirectWriteApplication::Instance()->d2dFactory_->CreateHwndRenderTarget(
            D2D1::RenderTargetProperties(),
            D2D1::HwndRenderTargetProperties(
                hWnd_,
                dxsize
            ),
            &rt_
        )));
        rt_->SetTransform(D2D1::IdentityMatrix());
        OSCHECK(SUCCEEDED(rt_->CreateSolidColorBrush(
            D2D1::ColorF(D2D1::ColorF::White),
            &fg_
        )));
        OSCHECK(SUCCEEDED(rt_->CreateSolidColorBrush(
            D2D1::ColorF(D2D1::ColorF::Black),
            &bg_
        )));
        OSCHECK(SUCCEEDED(rt_->CreateSolidColorBrush(
            D2D1::ColorF(D2D1::ColorF::White),
            &decor_
        )));
        OSCHECK(SUCCEEDED(rt_->CreateSolidColorBrush(
            D2D1::ColorF(0xffffff, 0.5),
            &border_
        )));
        ZeroMemory(&glyphRun_, sizeof(DWRITE_GLYPH_RUN));

        // create the glyph run buffer and size it appropriately
        updateDirectWriteStructures(width());

        // register the window
        RegisterWindowHandle(this, hWnd_);  
        
        setTitle(title_);
        setIcon(icon_);      
    }

    void DirectWriteWindow::updateDirectWriteStructures(int cols) {
		delete[] glyphIndices_;
		delete[] glyphAdvances_;
		delete[] glyphOffsets_;
		glyphIndices_ = new UINT16[cols];
		glyphAdvances_ = new FLOAT[cols];
		glyphOffsets_ = new DWRITE_GLYPH_OFFSET[cols];
		glyphRun_.glyphIndices = glyphIndices_;
		glyphRun_.glyphAdvances = glyphAdvances_;
		glyphRun_.glyphOffsets = glyphOffsets_;
		ZeroMemory(glyphOffsets_, sizeof(DWRITE_GLYPH_OFFSET) * cols);
		// nothing to be done with advances and indices as these are filled by the drawing method 
		glyphRun_.glyphCount = 0;
    }


	// https://docs.microsoft.com/en-us/windows/desktop/inputdev/virtual-key-codes
	Key DirectWriteWindow::GetKey(unsigned vk) {
		// we don't distinguish between left and right win keys
		if (vk == VK_RWIN)
			vk = VK_LWIN;
        Key result = Key::FromCode(vk);
        if (result == Key::Invalid)
            return result;
		// MSB == pressed, LSB state since last time
        if (GetAsyncKeyState(VK_SHIFT) & 0x8000)
            result = result + Key::Shift;
        if (GetAsyncKeyState(VK_CONTROL) & 0x8000)
            result = result + Key::Ctrl;
        if (GetAsyncKeyState(VK_MENU) & 0x8000)
            result = result + Key::Alt;
        if ((GetAsyncKeyState(VK_LWIN) & 0x8000) || (GetAsyncKeyState(VK_RWIN) & 0x8000))
            result = result + Key::Win;
        return result;
	}

	LRESULT CALLBACK DirectWriteWindow::EventHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
        DirectWriteWindow * window = GetWindowForHandle(hWnd);
        switch (msg) {
			/* When the window is created, the border width and height of a terminal window is determined and the window's size is updated to adjust for it. */
			case WM_CREATE: {
				// now calculate the border and actually update the window size to account for it
				CREATESTRUCT& cs = *reinterpret_cast<CREATESTRUCT*>(lParam);
				// get the tw member from the create struct argument
				ASSERT(window == nullptr);
				window = reinterpret_cast<DirectWriteWindow*>(cs.lpCreateParams);
				ASSERT(window != nullptr);
				RECT r;
				r.left = cs.x;
				r.right = cs.x + cs.cx;
				r.top = cs.y;
				r.bottom = cs.y + cs.cy;
				AdjustWindowRectEx(&r, cs.style, false, cs.dwExStyle);
				unsigned fw = r.right - r.left - cs.cx;
				unsigned fh = r.bottom - r.top - cs.cy;
				if (fw != 0 || fh != 0) {
					window->frameWidth_ = fw;
					window->frameHeight_ = fh;
					SetWindowPos(hWnd, HWND_TOP, cs.x, cs.y, cs.cx + fw, cs.cy + fh, SWP_NOZORDER | SWP_NOACTIVATE);
				}
				break;
			}
			/** Closes the current window. 
             
                Calls the renderer's requestClose() method which shoudl in call renderer's closing function that calls DestroyWindow. 
             */
			case WM_CLOSE: {
				ASSERT(window != nullptr) << "Unknown window";
                window->requestClose();
                return 0;
			}
			/** Destroys the window, if it is the last window, quits the app. */
			case WM_DESTROY: {
				ASSERT(window != nullptr) << "Attempt to destroy unknown window";
				// delete the window object
                delete window;
                if (Windows().empty())
                    PostQuitMessage(EXIT_SUCCESS);
				break;
			}
            /** Window gains keyboard focus. 
			 */
			case WM_SETFOCUS:
				if (window != nullptr)
    				window->focusIn();
				break;
			/** Window loses keyboard focus. 
			 */
			case WM_KILLFOCUS:
				if (window != nullptr)
    				window->focusOut();
				break;
			case WM_NCHITTEST: {
				if (window == nullptr || window->fullscreen())
					break;
				POINT cursor{GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam)};
				ScreenToClient(hWnd, &cursor);
				RECT client{};
				GetClientRect(hWnd, &client);
				int const edge = std::max(6, GetSystemMetrics(SM_CXSIZEFRAME));
				bool const left = cursor.x < edge;
				bool const right = cursor.x >= client.right - edge;
				bool const top = cursor.y < edge;
				bool const bottom = cursor.y >= client.bottom - edge;
				if (top && left) return HTTOPLEFT;
				if (top && right) return HTTOPRIGHT;
				if (bottom && left) return HTBOTTOMLEFT;
				if (bottom && right) return HTBOTTOMRIGHT;
				if (left) return HTLEFT;
				if (right) return HTRIGHT;
				if (top) return HTTOP;
				if (bottom) return HTBOTTOM;
				break;
			}
			/* Called when the window is resized interactively by the user. Makes sure that the window size snaps to discrete terminal sizes. 
            
            We must take into account the DPI current DPI system settings that may increase the real cell size. 
            */
			case WM_SIZING: {
				RECT* winRect = reinterpret_cast<RECT*>(lParam);
				switch (wParam) {
                    case WMSZ_BOTTOM:
                    case WMSZ_BOTTOMRIGHT:
                    case WMSZ_BOTTOMLEFT:
                        winRect->bottom -= (winRect->bottom - winRect->top - window->frameHeight_) % static_cast<int>(window->cellSize_.height() * window->dpiFactor_);
                        break;
                    default:
                        winRect->top += (winRect->bottom - winRect->top - window->frameHeight_) % static_cast<int>(window->cellSize_.height() * window->dpiFactor_);
                        break;
				}
				switch (wParam) {
                    case WMSZ_RIGHT:
                    case WMSZ_TOPRIGHT:
                    case WMSZ_BOTTOMRIGHT:
                        winRect->right -= (winRect->right - winRect->left - window->frameWidth_) % static_cast<int>(window->cellSize_.width() * window->dpiFactor_);
                        break;
                    default:
                        winRect->left += (winRect->right - winRect->left - window->frameWidth_) % static_cast<int>(window->cellSize_.width() * window->dpiFactor_);
                        break;
				}
				break;
			}
			/* Called when the window is resized to given values.

			   No resize is performed if the window is minimized (we would have terminal size of length 0).

			   It is ok if no terminal window is associated with the handle as the message can be sent from the WM_CREATE when window is resized to account for the window border which has to be calculated.
			 */
			case WM_SIZE: {
				if (wParam == SIZE_MINIMIZED)
					break;
				if (window != nullptr) {
					window->layoutCaptionControls(!IsZoomed(hWnd));
					RECT rect;
					GetClientRect(hWnd, &rect);
					window->windowResized(rect.right, rect.bottom);
				}
				break;
			}
			case WM_ERASEBKGND:
				// Direct2D paints the full client area. Skipping the default erase avoids
				// a blank frame while the target is resized during maximize/restore.
				return 1;
			/* Repaint of the window is requested. 
             */
			case WM_PAINT: {
				ASSERT(window != nullptr) << "Attempt to paint unknown window";
				window->repaint();
				break;
			}
			case WM_DRAWITEM: {
				DRAWITEMSTRUCT* draw = reinterpret_cast<DRAWITEMSTRUCT*>(lParam);
				if (draw != nullptr && window != nullptr) {
					window->drawTitleBarItem(draw);
					return TRUE;
				}
				break;
			}
			case WM_APP + 0x33:
				if (window != nullptr) {
					int const index = static_cast<int>(wParam) - TabButtonId;
					if (index >= 0 && index < static_cast<int>(window->tabButtons_.size())) {
						auto close = window->tabButtons_[index].item.close;
						if (close)
							close();
					}
				}
				return 0;
			case WM_COMMAND:
				if (HIWORD(wParam) == BN_CLICKED && window != nullptr) {
					int const id = LOWORD(wParam);
					if (id == AddTabButtonId) {
						if (window->addTabAction_) window->addTabAction_();
						return 0;
					}
					if (id == DropDownButtonId) {
						window->showDropDownMenu();
						return 0;
					}
					if (id == MenuButtonId) {
						if (window->showMenuAction_) window->showMenuAction_();
						return 0;
					}
					if (id >= TabButtonId && id < TabButtonId + static_cast<int>(window->tabButtons_.size())) {
						auto activate = window->tabButtons_[id - TabButtonId].item.activate;
						if (activate) activate();
						return 0;
					}
				}
				if (HIWORD(wParam) == BN_CLICKED && LOWORD(wParam) >= CaptionButtonId && LOWORD(wParam) < CaptionButtonId + 3) {
					if (window != nullptr) {
						switch (LOWORD(wParam) - CaptionButtonId) {
							case 0: window->minimize(); break;
							case 1: window->toggleMaximized(); break;
							case 2: window->requestClose(); break;
						}
					}
					return 0;
				}
				break;
			/* No need to use WM_UNICHAR since WM_CHAR is already unicode aware */
			case WM_UNICHAR:
				UNREACHABLE;
				break;
			/* If a system character is intercepted, do nothing & bypass the default implementation.
			 
			    This silences sounds played when alt + enter was pressed multiple times and some other weird behavior. 

				TODO Maybe a better way how to distinguish when to pass the character to the default handler and when to ignore it based on whether the shortcut is actually used?
			 */
			case WM_SYSCHAR:
			    switch (wParam) {
					case Char::LF:
					case Char::CR:
					    return 0;
					default:
					    break;
				}
			    break;
			case WM_CHAR:
				if (wParam >= 0x20)
					window->keyChar(Char{static_cast<char32_t>(wParam)});
				break;
			/* Processes special key events.
			
			   TODO perhaps all the syskeydown & syskeyup events should be stopped? 
			 */
			case WM_SYSKEYDOWN:
			case WM_KEYDOWN: {
				Key k = GetKey(static_cast<unsigned>(wParam));
				if (k != Key::Invalid)
					window->keyDown(k);
				// returning w/o calling the default window proc means that the OS will not interfere by interpreting own shortcuts
				// NOTE add other interfering shortcuts as necessary
				if (k == Key::F10 || k.key() == Key::AltKey)
				    return 0;
				break;
			}
			/* The modifier part of the key corresponds to the state of the modifiers *after* the key has been released. 
			 */
			case WM_SYSKEYUP:
			case WM_KEYUP: {
				Key k = GetKey(static_cast<unsigned>(wParam));
				window->keyUp(k);
				break;
			}
			/* Mouse events which simply obtain the mouse coordinates, convert the buttons and wheel values to vterm standards and then calls the DirectWriteTerminalWindow's events, which perform the pixels to cols & rows translation and then call the terminal itself.
			 */
#define MOUSE_X static_cast<int>(lParam & 0xffff)
#define MOUSE_Y static_cast<int>((lParam >> 16) & 0xffff)
			case WM_LBUTTONDOWN:
				window->mouseDown(window->pixelsToCoords(Point{MOUSE_X, MOUSE_Y}), MouseButton::Left);
				break;
			case WM_LBUTTONUP:
				window->mouseUp(window->pixelsToCoords(Point{MOUSE_X, MOUSE_Y}), MouseButton::Left);
				break;
			case WM_RBUTTONDOWN:
				window->mouseDown(window->pixelsToCoords(Point{MOUSE_X, MOUSE_Y}), MouseButton::Right);
				break;
			case WM_RBUTTONUP:
				window->mouseUp(window->pixelsToCoords(Point{MOUSE_X, MOUSE_Y}), MouseButton::Right);
				break;
			case WM_MBUTTONDOWN:
				window->mouseDown(window->pixelsToCoords(Point{MOUSE_X, MOUSE_Y}), MouseButton::Wheel);
				break;
			case WM_MBUTTONUP:
				window->mouseUp(window->pixelsToCoords(Point{MOUSE_X, MOUSE_Y}), MouseButton::Wheel);
				break;
			/* Mouse wheel contains the position relative to screen top/left, so we must first translate it to window coordinates. 
			 */
			case WM_MOUSEWHEEL: {
				POINT pos{ MOUSE_X, MOUSE_Y };
				ScreenToClient(hWnd, &pos);
				window->mouseWheel(window->pixelsToCoords(Point{pos.x, pos.y}), GET_WHEEL_DELTA_WPARAM(wParam) / WHEEL_DELTA);
				break;
			}
			case WM_MOUSEMOVE:
				window->mouseMove(window->pixelsToCoords(Point{GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam)}));
				break;
			/* Triggered when mouse leaves the window. There
 			 */
			case WM_MOUSELEAVE:
                window->mouseOut();
				break;
            /** When mouse is moved, the cursor is always reset to the window's default cursor, so to keep the cursor set by the application, we must make sure to set it each time the setcursor message is received. 
             */
            case WM_SETCURSOR: 
                if (LOWORD(lParam) == HTCLIENT) {
                    SetCursor(window->mouseCursor_);
                    return TRUE;
                }
                break;            
        }
        return DefWindowProc(hWnd, msg, wParam, lParam);
    }


} // namespace tpp

#endif
