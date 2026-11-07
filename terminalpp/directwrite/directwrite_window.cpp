#if (defined ARCH_WINDOWS && defined RENDERER_NATIVE)
#include "directwrite_window.h"

#include "directwrite_font.h"

namespace tpp {

    namespace {

        constexpr wchar_t DropDownPopupClassName[] = L"terminalpp.custom.dropdown.popup";

    } // namespace

    int DirectWriteWindow::titleBarControlAt(POINT clientPoint) const {
        auto contains = [clientPoint](RECT const & rect) {
            return PtInRect(&rect, clientPoint) != FALSE;
        };
        for (int i = 0; i < 3; ++i) {
            if (contains(captionButtonRects_[i]))
                return CaptionButtonId + i;
        }
        if (contains(menuButtonRect_))
            return MenuButtonId;
        if (contains(addTabButtonRect_))
            return AddTabButtonId;
        if (contains(dropDownButtonRect_))
            return DropDownButtonId;
        for (size_t i = 0; i < tabButtons_.size(); ++i) {
            if (i < tabButtonRects_.size() && contains(tabButtonRects_[i]))
                return TabButtonId + static_cast<int>(i);
        }
        return -1;
    }

    bool DirectWriteWindow::titleBarCloseAt(int id, POINT clientPoint) const {
        if (id < TabButtonId || id >= TabButtonId + static_cast<int>(tabButtons_.size()))
            return false;
        size_t const index = static_cast<size_t>(id - TabButtonId);
        if (index >= tabButtonRects_.size())
            return false;
        RECT const & rect = tabButtonRects_[index];
        return clientPoint.x >= rect.right - static_cast<int>(46 * dpiFactor_);
    }

    void DirectWriteWindow::showDropDownMenu() {
        if (dropDownItems_.empty())
            return;
        if (dropDownMenuActive_)
            return;

        HINSTANCE const instance = DirectWriteApplication::Instance()->hInstance_;
        WNDCLASSEXW windowClass{};
        windowClass.cbSize = sizeof(windowClass);
        if (!GetClassInfoExW(instance, DropDownPopupClassName, &windowClass)) {
            windowClass.style = CS_DBLCLKS;
            windowClass.lpfnWndProc = DropDownPopupProc;
            windowClass.hInstance = instance;
            windowClass.hCursor = LoadCursorW(nullptr, IDC_ARROW);
            windowClass.lpszClassName = DropDownPopupClassName;
            if (!RegisterClassExW(&windowClass))
                return;
        }

        int const scale = std::max(1, static_cast<int>(dpiFactor_ * 100));
        int const rowHeight = MulDiv(30, scale, 100);
        int const width = dropDownPopupWidth(dropDownItems_, 68);
        int const height = rowHeight * static_cast<int>(dropDownItems_.size());
        RECT buttonRect = dropDownButtonRect_;
        MapWindowPoints(hWnd_, nullptr, reinterpret_cast<POINT*>(&buttonRect), 2);
        int x = buttonRect.left;
        int y = buttonRect.bottom;
        RECT workArea{};
        MONITORINFO monitorInfo{sizeof(monitorInfo)};
        if (GetMonitorInfoW(MonitorFromRect(&buttonRect, MONITOR_DEFAULTTONEAREST), &monitorInfo))
            workArea = monitorInfo.rcWork;
        else
            SystemParametersInfoW(SPI_GETWORKAREA, 0, &workArea, 0);
        if (y + height > static_cast<int>(workArea.bottom))
            y = std::max(static_cast<int>(workArea.top), static_cast<int>(buttonRect.top) - height);
        x = std::max(static_cast<int>(workArea.left), std::min(x, static_cast<int>(workArea.right) - width));

        dropDownMenuActive_ = true;
        hoveredDropDownRootItem_ = -1;
        hoveredDropDownChildItem_ = -1;
        dropDownPopup_ = CreateWindowExW(WS_EX_TOOLWINDOW | WS_EX_TOPMOST | WS_EX_NOACTIVATE,
            DropDownPopupClassName, L"", WS_POPUP, x, y, width, height,
            hWnd_, nullptr, instance, this);
        if (dropDownPopup_ == nullptr) {
            dropDownMenuActive_ = false;
            return;
        }
        ShowWindow(dropDownPopup_, SW_SHOWNOACTIVATE);
        SetWindowPos(dropDownPopup_, HWND_TOPMOST, x, y, width, height,
            SWP_NOACTIVATE | SWP_SHOWWINDOW);
        SetCapture(dropDownPopup_);
        SetFocus(hWnd_);
        InvalidateRect(dropDownPopup_, nullptr, FALSE);
    }

    int DirectWriteWindow::dropDownPopupWidth(std::vector<Window::TabBarMenuItem> const & items, int padding) const {
        int const scale = std::max(1, static_cast<int>(dpiFactor_ * 100));
        HDC dc = GetDC(hWnd_);
        if (dc == nullptr)
            return MulDiv(padding + 120, scale, 100);
        HFONT font = CreateFontW(-MulDiv(12, scale, 100), 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
            DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY,
            DEFAULT_PITCH | FF_DONTCARE, L"Segoe UI");
        HGDIOBJ oldFont = SelectObject(dc, font);
        int widest = 0;
        for (Window::TabBarMenuItem const & item : items) {
            utf16_string const title = UTF8toUTF16(item.title);
            SIZE extent{};
            GetTextExtentPoint32W(dc, title.c_str(), static_cast<int>(title.size()), &extent);
            widest = std::max(widest, static_cast<int>(extent.cx));
        }
        SelectObject(dc, oldFont);
        DeleteObject(font);
        ReleaseDC(hWnd_, dc);
        return widest + MulDiv(padding, scale, 100);
    }

    void DirectWriteWindow::showDropDownSubmenu(size_t itemIndex) {
        if (dropDownPopup_ == nullptr || itemIndex >= dropDownItems_.size() ||
            dropDownItems_[itemIndex].children.empty() ||
            (dropDownSubmenuPopup_ != nullptr && hoveredDropDownRootItem_ == static_cast<int>(itemIndex)))
            return;
        if (dropDownSubmenuPopup_ != nullptr && hoveredDropDownRootItem_ >= 0 &&
            hoveredDropDownRootItem_ < static_cast<int>(dropDownItems_.size())) {
            DestroyWindow(dropDownSubmenuPopup_);
            dropDownSubmenuPopup_ = nullptr;
        }
        hoveredDropDownRootItem_ = static_cast<int>(itemIndex);
        hoveredDropDownChildItem_ = -1;

        RECT rootRect{};
        GetWindowRect(dropDownPopup_, &rootRect);
        int const scale = std::max(1, static_cast<int>(dpiFactor_ * 100));
        int const rowHeight = MulDiv(30, scale, 100);
        int const width = dropDownPopupWidth(dropDownItems_[itemIndex].children, 50);
        int const height = rowHeight * static_cast<int>(dropDownItems_[itemIndex].children.size());
        int x = rootRect.right;
        int y = rootRect.top + rowHeight * static_cast<int>(itemIndex);
        RECT workArea{};
        MONITORINFO monitorInfo{sizeof(monitorInfo)};
        if (GetMonitorInfoW(MonitorFromRect(&rootRect, MONITOR_DEFAULTTONEAREST), &monitorInfo))
            workArea = monitorInfo.rcWork;
        else
            SystemParametersInfoW(SPI_GETWORKAREA, 0, &workArea, 0);
        if (x + width > static_cast<int>(workArea.right))
            x = static_cast<int>(rootRect.left) - width;
        if (y + height > static_cast<int>(workArea.bottom))
            y = static_cast<int>(workArea.bottom) - height;
        y = std::max(static_cast<int>(workArea.top), y);

        dropDownSubmenuPopup_ = CreateWindowExW(WS_EX_TOOLWINDOW | WS_EX_TOPMOST | WS_EX_NOACTIVATE,
            DropDownPopupClassName, L"", WS_POPUP, x, y, width, height,
            hWnd_, nullptr, DirectWriteApplication::Instance()->hInstance_, this);
        if (dropDownSubmenuPopup_ != nullptr) {
            ShowWindow(dropDownSubmenuPopup_, SW_SHOWNOACTIVATE);
            SetWindowPos(dropDownSubmenuPopup_, HWND_TOPMOST, x, y, width, height,
                SWP_NOACTIVATE | SWP_SHOWWINDOW);
            InvalidateRect(dropDownSubmenuPopup_, nullptr, FALSE);
        }
        InvalidateRect(dropDownPopup_, nullptr, FALSE);
    }

    void DirectWriteWindow::dismissDropDownMenu() {
        HWND const root = dropDownPopup_;
        HWND const child = dropDownSubmenuPopup_;
        dropDownPopup_ = nullptr;
        dropDownSubmenuPopup_ = nullptr;
        dropDownMenuActive_ = false;
        hoveredDropDownRootItem_ = -1;
        hoveredDropDownChildItem_ = -1;
        if (GetCapture() == root)
            ReleaseCapture();
        if (child != nullptr)
            DestroyWindow(child);
        if (root != nullptr)
            DestroyWindow(root);
        if (IsWindow(hWnd_))
            SetFocus(hWnd_);
    }

    void DirectWriteWindow::updateDropDownMenuHover(POINT screenPoint) {
        auto const rowAt = [this, screenPoint](HWND popup, size_t count) -> int {
            if (popup == nullptr)
                return -1;
            RECT rect{};
            GetWindowRect(popup, &rect);
            if (!PtInRect(&rect, screenPoint))
                return -1;
            int const rowHeight = MulDiv(30, std::max(1, static_cast<int>(dpiFactor_ * 100)), 100);
            int const index = (screenPoint.y - rect.top) / std::max(1, rowHeight);
            return index >= 0 && index < static_cast<int>(count) ? index : -1;
        };

        int const rootIndex = rowAt(dropDownPopup_, dropDownItems_.size());
        if (rootIndex >= 0) {
            if (hoveredDropDownRootItem_ != rootIndex) {
                hoveredDropDownRootItem_ = rootIndex;
                InvalidateRect(dropDownPopup_, nullptr, FALSE);
            }
            if (!dropDownItems_[rootIndex].children.empty())
                showDropDownSubmenu(static_cast<size_t>(rootIndex));
            else if (dropDownSubmenuPopup_ != nullptr) {
                DestroyWindow(dropDownSubmenuPopup_);
                dropDownSubmenuPopup_ = nullptr;
                hoveredDropDownRootItem_ = rootIndex;
                InvalidateRect(dropDownPopup_, nullptr, FALSE);
            }
            return;
        }

        if (dropDownSubmenuPopup_ != nullptr) {
            int const childIndex = rowAt(dropDownSubmenuPopup_,
                dropDownItems_[hoveredDropDownRootItem_].children.size());
            if (childIndex >= 0 && hoveredDropDownChildItem_ != childIndex) {
                hoveredDropDownChildItem_ = childIndex;
                InvalidateRect(dropDownSubmenuPopup_, nullptr, FALSE);
            }
        }
    }

    void DirectWriteWindow::activateDropDownMenuAt(POINT screenPoint) {
        auto const rowAt = [this, screenPoint](HWND popup, size_t count) -> int {
            if (popup == nullptr)
                return -1;
            RECT rect{};
            GetWindowRect(popup, &rect);
            if (!PtInRect(&rect, screenPoint))
                return -1;
            int const rowHeight = MulDiv(30, std::max(1, static_cast<int>(dpiFactor_ * 100)), 100);
            int const index = (screenPoint.y - rect.top) / std::max(1, rowHeight);
            return index >= 0 && index < static_cast<int>(count) ? index : -1;
        };

        int const rootIndex = rowAt(dropDownPopup_, dropDownItems_.size());
        if (rootIndex >= 0 && !dropDownItems_[rootIndex].children.empty()) {
            showDropDownSubmenu(static_cast<size_t>(rootIndex));
            return;
        }
        std::function<void()> action;
        if (rootIndex >= 0) {
            action = dropDownItems_[rootIndex].activate;
        } else if (dropDownSubmenuPopup_ != nullptr && hoveredDropDownRootItem_ >= 0) {
            auto const & children = dropDownItems_[hoveredDropDownRootItem_].children;
            int const childIndex = rowAt(dropDownSubmenuPopup_, children.size());
            if (childIndex >= 0)
                action = children[childIndex].activate;
        }
        dismissDropDownMenu();
        if (action)
            action();
    }

    void DirectWriteWindow::drawDropDownMenu(HWND popup, HDC dc) {
        if (popup == nullptr || dc == nullptr)
            return;
        RECT client{};
        GetClientRect(popup, &client);
        HBRUSH background = CreateSolidBrush(RGB(43, 43, 43));
        FillRect(dc, &client, background);
        DeleteObject(background);

        bool const isChild = popup == dropDownSubmenuPopup_;
        std::vector<Window::TabBarMenuItem> const * items = &dropDownItems_;
        int hover = hoveredDropDownRootItem_;
        if (isChild && hover >= 0 && hover < static_cast<int>(dropDownItems_.size())) {
            items = &dropDownItems_[hover].children;
            hover = hoveredDropDownChildItem_;
        }

        int const scale = std::max(1, static_cast<int>(dpiFactor_ * 100));
        auto const px = [scale](int value) { return MulDiv(value, scale, 100); };
        int const rowHeight = px(30);
        HFONT font = CreateFontW(-px(12), 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
            DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY,
            DEFAULT_PITCH | FF_DONTCARE, L"Segoe UI");
        HGDIOBJ oldFont = SelectObject(dc, font);
        int const oldBkMode = SetBkMode(dc, TRANSPARENT);
        for (size_t i = 0; i < items->size(); ++i) {
            RECT row{0, static_cast<LONG>(i) * rowHeight, client.right, static_cast<LONG>(i + 1) * rowHeight};
            if (static_cast<int>(i) == hover) {
                HBRUSH highlight = CreateSolidBrush(RGB(68, 68, 68));
                FillRect(dc, &row, highlight);
                DeleteObject(highlight);
            }
            Window::TabBarMenuItem const & item = (*items)[i];
            bool const enabled = item.enabled && (item.activate || !item.children.empty());
            int const iconSize = px(12);
            RECT icon{px(8), row.top + (rowHeight - iconSize) / 2, px(8) + iconSize, row.top + (rowHeight + iconSize) / 2};
            HBRUSH iconBrush = CreateSolidBrush(enabled ? RGB(55, 117, 190) : RGB(76, 76, 76));
            HPEN iconPen = CreatePen(PS_SOLID, 1, enabled ? RGB(55, 117, 190) : RGB(76, 76, 76));
            HGDIOBJ oldBrush = SelectObject(dc, iconBrush);
            HGDIOBJ oldPen = SelectObject(dc, iconPen);
            RoundRect(dc, icon.left, icon.top, icon.right, icon.bottom, px(4), px(4));
            SelectObject(dc, oldPen);
            SelectObject(dc, oldBrush);
            DeleteObject(iconPen);
            DeleteObject(iconBrush);
            SetTextColor(dc, enabled ? RGB(245, 245, 245) : RGB(145, 145, 145));
            DrawTextW(dc, L">_", 2, &icon, DT_CENTER | DT_VCENTER | DT_SINGLELINE | DT_NOPREFIX);

            RECT label = row;
            label.left = px(34);
            label.right -= item.children.empty() ? px(8) : px(26);
            utf16_string const title = UTF8toUTF16(item.title);
            DrawTextW(dc, title.c_str(), static_cast<int>(title.size()), &label,
                DT_LEFT | DT_VCENTER | DT_SINGLELINE | DT_NOPREFIX);
            if (!item.children.empty()) {
                RECT chevron = row;
                chevron.left = chevron.right - px(20);
                DrawTextW(dc, L">", 1, &chevron, DT_CENTER | DT_VCENTER | DT_SINGLELINE | DT_NOPREFIX);
            }
        }
        SetBkMode(dc, oldBkMode);
        SelectObject(dc, oldFont);
        DeleteObject(font);
    }

    LRESULT CALLBACK DirectWriteWindow::DropDownPopupProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam) {
        DirectWriteWindow * window = reinterpret_cast<DirectWriteWindow*>(GetWindowLongPtrW(hwnd, GWLP_USERDATA));
        if (message == WM_NCCREATE) {
            auto const * create = reinterpret_cast<CREATESTRUCTW const *>(lParam);
            window = static_cast<DirectWriteWindow*>(create->lpCreateParams);
            SetWindowLongPtrW(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(window));
        }
        if (window == nullptr)
            return DefWindowProcW(hwnd, message, wParam, lParam);
        switch (message) {
            case WM_MOUSEACTIVATE:
                return MA_NOACTIVATE;
            case WM_ERASEBKGND:
                return 1;
            case WM_PAINT: {
                PAINTSTRUCT paint{};
                HDC dc = BeginPaint(hwnd, &paint);
                window->drawDropDownMenu(hwnd, dc);
                EndPaint(hwnd, &paint);
                return 0;
            }
            case WM_MOUSEMOVE: {
                POINT point{};
                GetCursorPos(&point);
                window->updateDropDownMenuHover(point);
                return 0;
            }
            case WM_LBUTTONUP: {
                POINT point{};
                GetCursorPos(&point);
                window->activateDropDownMenuAt(point);
                return 0;
            }
            case WM_KEYDOWN:
                if (wParam == VK_ESCAPE) {
                    window->dismissDropDownMenu();
                    return 0;
                }
                break;
            case WM_CAPTURECHANGED:
                if (hwnd == window->dropDownPopup_ && window->dropDownMenuActive_ &&
                    reinterpret_cast<HWND>(lParam) != window->dropDownPopup_)
                    window->dismissDropDownMenu();
                return 0;
            case WM_NCDESTROY:
                if (hwnd == window->dropDownSubmenuPopup_)
                    window->dropDownSubmenuPopup_ = nullptr;
                if (hwnd == window->dropDownPopup_) {
                    window->dropDownPopup_ = nullptr;
                    window->dropDownMenuActive_ = false;
                }
                SetWindowLongPtrW(hwnd, GWLP_USERDATA, 0);
                break;
        }
        return DefWindowProcW(hwnd, message, wParam, lParam);
    }

    void DirectWriteWindow::setTabBar(std::vector<Window::TabBarItem> tabs, std::function<void()> addTab, std::function<void()> showMenu, std::vector<Window::TabBarMenuItem> dropDownItems) {
        UI_THREAD_ONLY;
        if (dropDownMenuActive_)
            dismissDropDownMenu();
        addTabAction_ = std::move(addTab);
        showMenuAction_ = std::move(showMenu);
        dropDownItems_ = std::move(dropDownItems);

        size_t const sharedCount = std::min(tabButtons_.size(), tabs.size());
        for (size_t i = 0; i < sharedCount; ++i)
            tabButtons_[i].item = std::move(tabs[i]);

        tabButtons_.resize(tabs.size());

        for (size_t i = sharedCount; i < tabs.size(); ++i) {
            tabButtons_[i].item = std::move(tabs[i]);
        }

        layoutCaptionControls(!IsZoomed(hWnd_));
        RECT titleBar{0, 0, sizePx_.width(), captionControlHeight()};
        InvalidateRect(hWnd_, &titleBar, FALSE);
    }

    void DirectWriteWindow::invalidateTitleButton(int id) {
        MARK_AS_UNUSED(id);
        RECT titleBar{0, 0, sizePx_.width(), captionControlHeight()};
        InvalidateRect(hWnd_, &titleBar, FALSE);
    }

    void DirectWriteWindow::activateTitleBarControl(int id) {
        if (id >= CaptionButtonId && id < CaptionButtonId + 3) {
            switch (id - CaptionButtonId) {
                case 0: minimize(); break;
                case 1: toggleMaximized(); break;
                case 2: requestClose(); break;
            }
        } else if (id == MenuButtonId) {
            if (showMenuAction_)
                showMenuAction_();
        } else if (id == AddTabButtonId) {
            if (addTabAction_)
                addTabAction_();
        } else if (id == DropDownButtonId) {
            showDropDownMenu();
        } else if (id >= TabButtonId && id < TabButtonId + static_cast<int>(tabButtons_.size())) {
            auto activate = tabButtons_[id - TabButtonId].item.activate;
            if (activate)
                activate();
        }
    }

    void DirectWriteWindow::drawCaptionBar() {
        if (sizePx_.width() <= 0)
            return;

        auto const color = [](int r, int g, int b) {
            return D2D1::ColorF(r / 255.0f, g / 255.0f, b / 255.0f);
        };
        Microsoft::WRL::ComPtr<ID2D1SolidColorBrush> bar;
        Microsoft::WRL::ComPtr<ID2D1SolidColorBrush> hover;
        Microsoft::WRL::ComPtr<ID2D1SolidColorBrush> active;
        Microsoft::WRL::ComPtr<ID2D1SolidColorBrush> foreground;
        Microsoft::WRL::ComPtr<ID2D1SolidColorBrush> closeHover;
        Microsoft::WRL::ComPtr<ID2D1SolidColorBrush> closeButton;
        if (FAILED(rt_->CreateSolidColorBrush(color(43, 43, 43), &bar)) ||
            FAILED(rt_->CreateSolidColorBrush(color(62, 62, 62), &hover)) ||
            FAILED(rt_->CreateSolidColorBrush(color(12, 12, 12), &active)) ||
            FAILED(rt_->CreateSolidColorBrush(color(240, 240, 240), &foreground)) ||
            FAILED(rt_->CreateSolidColorBrush(color(68, 68, 68), &closeHover)) ||
            FAILED(rt_->CreateSolidColorBrush(color(196, 43, 28), &closeButton)))
            return;

        Microsoft::WRL::ComPtr<IDWriteTextFormat> format;
        if (FAILED(DirectWriteApplication::Instance()->dwFactory_->CreateTextFormat(
            L"Segoe UI", nullptr, DWRITE_FONT_WEIGHT_NORMAL, DWRITE_FONT_STYLE_NORMAL,
            DWRITE_FONT_STRETCH_NORMAL, 14.0f, L"",
            &format)))
            return;
        format->SetWordWrapping(DWRITE_WORD_WRAPPING_NO_WRAP);
        format->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_CENTER);

        auto const toRect = [this](RECT const & rect) {
            FLOAT const scale = static_cast<FLOAT>(dpiFactor_);
            return D2D1::RectF(static_cast<FLOAT>(rect.left) / scale, static_cast<FLOAT>(rect.top) / scale,
                static_cast<FLOAT>(rect.right) / scale, static_cast<FLOAT>(rect.bottom) / scale);
        };
        auto const fill = [this, &toRect](RECT const & rect, ID2D1Brush * brush) {
            rt_->FillRectangle(toRect(rect), brush);
        };
        auto drawText = [this, &format, &foreground, &toRect](std::wstring const & text, RECT rect, DWRITE_TEXT_ALIGNMENT alignment) {
            format->SetTextAlignment(alignment);
            rt_->DrawText(text.c_str(), static_cast<UINT32>(text.size()), format.Get(), toRect(rect), foreground.Get(),
                D2D1_DRAW_TEXT_OPTIONS_CLIP, DWRITE_MEASURING_MODE_NATURAL);
        };

        RECT const clientBar{0, 0, sizePx_.width(), captionControlHeight()};
        fill(clientBar, bar.Get());

        auto drawButton = [&](RECT const & rect, int id) {
            bool const isHot = hoveredButtonId_ == id;
            bool const isPressed = pressedButtonId_ == id;
            ID2D1Brush * const buttonBackground = isHot || isPressed ? hover.Get() : bar.Get();
            if (id >= CaptionButtonId && id < CaptionButtonId + 3) {
                int const index = id - CaptionButtonId;
                if (index == 2 && (isHot || isPressed))
                    fill(rect, closeButton.Get());
                else
                    fill(rect, buttonBackground);
                FLOAT const scale = static_cast<FLOAT>(dpiFactor_);
                FLOAT const cx = static_cast<FLOAT>((rect.left + rect.right) / 2) / scale;
                FLOAT const cy = static_cast<FLOAT>((rect.top + rect.bottom) / 2) / scale;
                if (index == 0) {
                    rt_->DrawLine(D2D1::Point2F(cx - 6, cy), D2D1::Point2F(cx + 6, cy), foreground.Get(), 1.0f);
                } else if (index == 1) {
                    D2D1_RECT_F box = D2D1::RectF(cx - 5, cy - 5, cx + 5, cy + 5);
                    rt_->DrawRectangle(box, foreground.Get(), 1.0f);
                } else {
                    rt_->DrawLine(D2D1::Point2F(cx - 4, cy - 4), D2D1::Point2F(cx + 4, cy + 4), foreground.Get(), 1.2f);
                    rt_->DrawLine(D2D1::Point2F(cx + 4, cy - 4), D2D1::Point2F(cx - 4, cy + 4), foreground.Get(), 1.2f);
                }
                return;
            }

            if (id == MenuButtonId || id == AddTabButtonId || id == DropDownButtonId) {
                fill(rect, buttonBackground);
                FLOAT const scale = static_cast<FLOAT>(dpiFactor_);
                FLOAT const cx = static_cast<FLOAT>((rect.left + rect.right) / 2) / scale;
                FLOAT const cy = static_cast<FLOAT>((rect.top + rect.bottom) / 2) / scale;
                FLOAT const stroke = 1.0f;
                if (id == MenuButtonId) {
                    for (int line = -1; line <= 1; ++line)
                        rt_->DrawLine(D2D1::Point2F(cx - 7, cy + line * 4), D2D1::Point2F(cx + 7, cy + line * 4), foreground.Get(), stroke);
                } else if (id == AddTabButtonId) {
                    drawText(L"+", rect, DWRITE_TEXT_ALIGNMENT_CENTER);
                } else {
                    rt_->DrawLine(D2D1::Point2F(cx - 3, cy - 1), D2D1::Point2F(cx, cy + 2), foreground.Get(), stroke);
                    rt_->DrawLine(D2D1::Point2F(cx, cy + 2), D2D1::Point2F(cx + 3, cy - 1), foreground.Get(), stroke);
                }
                return;
            }

            if (id < TabButtonId || id >= TabButtonId + static_cast<int>(tabButtons_.size()))
                return;
            size_t const index = static_cast<size_t>(id - TabButtonId);
            Window::TabBarItem const & tab = tabButtons_[index].item;
            fill(rect, bar.Get());
            if (tab.active || isHot) {
                RECT surface = rect;
                surface.left += 3;
                surface.right -= 3;
                surface.top += std::max(3L, (surface.bottom - surface.top) / 5);
                fill(surface, tab.active ? active.Get() : hover.Get());
            }
            RECT label = rect;
            label.left += static_cast<LONG>(44 * dpiFactor_);
            label.right -= static_cast<LONG>(48 * dpiFactor_);
            drawText(UTF8toUTF16(tab.title), label, DWRITE_TEXT_ALIGNMENT_LEADING);
            if (tab.active || isHot) {
                RECT closeRect = rect;
                closeRect.left = closeRect.right - static_cast<LONG>(40 * dpiFactor_);
                closeRect.right -= static_cast<LONG>(5 * dpiFactor_);
                if (hoveredCloseButtonId_ == id) {
                    RECT close = rect;
                    close.left = close.right - static_cast<LONG>(40 * dpiFactor_);
                    close.right -= static_cast<LONG>(12 * dpiFactor_);
                    close.top += static_cast<LONG>(8 * dpiFactor_);
                    close.bottom -= static_cast<LONG>(8 * dpiFactor_);
                    fill(close, closeHover.Get());
                }
                drawText(L"×", closeRect, DWRITE_TEXT_ALIGNMENT_CENTER);
            }
        };

        for (int i = 0; i < 3; ++i)
            drawButton(captionButtonRects_[i], CaptionButtonId + i);
        drawButton(menuButtonRect_, MenuButtonId);
        for (size_t i = 0; i < tabButtons_.size() && i < tabButtonRects_.size(); ++i)
            drawButton(tabButtonRects_[i], TabButtonId + static_cast<int>(i));
        drawButton(addTabButtonRect_, AddTabButtonId);
        drawButton(dropDownButtonRect_, DropDownButtonId);
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
            RendererWindow::setFullscreen(value);
            RECT titleBar{0, 0, sizePx_.width(), captionControlHeight()};
            InvalidateRect(hWnd_, &titleBar, FALSE);
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
        Size const terminalSizePx = sizePx_ * dpiFactor_;
        sizePx_ = Size{terminalSizePx.width(), terminalSizePx.height() + captionControlHeight()};
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
        if (window != nullptr && window->dropDownMenuActive_ && msg == WM_KEYDOWN && wParam == VK_ESCAPE) {
            window->dismissDropDownMenu();
            return 0;
        }
        if (window != nullptr && msg == WM_MOUSEACTIVATE &&
            (window->dropDownMenuActive_ || window->suppressNextMenuDismissClick_)) {
            // A click on the owner while TrackPopupMenu is active can first
            // arrive as WM_MOUSEACTIVATE. Eat it here so Windows does not
            // queue a later WM_LBUTTONDOWN after the popup has returned.
            window->suppressNextMenuDismissClick_ = false;
            KillTimer(hWnd, MenuDismissClearTimerId);
            window->suppressMenuDismissMouseUp_ = true;
            return MA_ACTIVATEANDEAT;
        }
        if (window != nullptr && window->dropDownMenuActive_ && msg == WM_LBUTTONDOWN) {
            // TrackPopupMenu runs a nested message loop. A click outside the
            // popup can reach the owner window and otherwise starts terminal
            // text selection underneath the dismissed menu.
            window->suppressMenuDismissMouseUp_ = true;
            return 0;
        }
        if (window != nullptr && msg == WM_TIMER && wParam == MenuDismissClearTimerId) {
            KillTimer(hWnd, MenuDismissClearTimerId);
            window->suppressNextMenuDismissClick_ = false;
            return 0;
        }
        if (window != nullptr && window->suppressNextMenuDismissClick_ && msg == WM_LBUTTONDOWN) {
            window->suppressNextMenuDismissClick_ = false;
            KillTimer(hWnd, MenuDismissClearTimerId);
            window->suppressMenuDismissMouseUp_ = true;
            return 0;
        }
        if (window != nullptr && window->suppressMenuDismissMouseUp_ && msg == WM_LBUTTONUP) {
            window->suppressMenuDismissMouseUp_ = false;
            return 0;
        }
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
				if (cursor.y < window->captionControlHeight() && window->titleBarControlAt(cursor) >= 0)
					return HTCLIENT;
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
				if (cursor.y < window->captionControlHeight())
					return window->titleBarControlAt(cursor) >= 0 ? HTCLIENT : HTCAPTION;
				break;
			}
			case WM_ENTERSIZEMOVE:
				if (window != nullptr) {
					window->interactiveResize_ = true;
				}
				break;
			case WM_EXITSIZEMOVE:
				if (window != nullptr) {
					window->interactiveResize_ = false;
					RECT client{};
					GetClientRect(hWnd, &client);
					window->windowResized(client.right, client.bottom);
					window->layoutCaptionControls(!IsZoomed(hWnd));
				}
				break;
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
					window->layoutCaptionControls(window->interactiveResize_ ? false : !IsZoomed(hWnd));
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
			case WM_LBUTTONDOWN: {
				POINT const point{MOUSE_X, MOUSE_Y};
				int const titleItem = window->fullscreen() ? -1 : window->titleBarControlAt(point);
				if (titleItem >= 0) {
					window->pressedButtonId_ = titleItem;
					SetCapture(hWnd);
					window->invalidateTitleButton(titleItem);
					return 0;
				}
				window->mouseDown(window->pixelsToCoords(Point{MOUSE_X, MOUSE_Y}), MouseButton::Left);
				break;
			}
			case WM_LBUTTONUP: {
				if (window->pressedButtonId_ >= 0) {
					int const pressed = window->pressedButtonId_;
					POINT const point{MOUSE_X, MOUSE_Y};
					int const released = window->titleBarControlAt(point);
					window->pressedButtonId_ = -1;
					if (GetCapture() == hWnd)
						ReleaseCapture();
					window->invalidateTitleButton(pressed);
					if (pressed == released) {
						if (window->titleBarCloseAt(pressed, point))
							PostMessageW(hWnd, WM_APP + 0x33, static_cast<WPARAM>(pressed), 0);
						else
							window->activateTitleBarControl(pressed);
					}
					return 0;
				}
				window->mouseUp(window->pixelsToCoords(Point{MOUSE_X, MOUSE_Y}), MouseButton::Left);
				break;
			}
			case WM_CAPTURECHANGED:
				if (window->pressedButtonId_ >= 0) {
					int const previous = window->pressedButtonId_;
					window->pressedButtonId_ = -1;
					window->invalidateTitleButton(previous);
				}
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
			case WM_MOUSEMOVE: {
				POINT const point{GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam)};
				if (!window->fullscreen() && point.y < window->captionControlHeight()) {
					int const hover = window->titleBarControlAt(point);
					int const closeHover = window->titleBarCloseAt(hover, point) ? hover : -1;
					if (window->hoveredButtonId_ != hover || window->hoveredCloseButtonId_ != closeHover) {
						window->hoveredButtonId_ = hover;
						window->hoveredCloseButtonId_ = closeHover;
						window->invalidateTitleButton(hover);
					}
					TRACKMOUSEEVENT track{};
					track.cbSize = sizeof(track);
					track.dwFlags = TME_LEAVE;
					track.hwndTrack = hWnd;
					TrackMouseEvent(&track);
					SetCursor(LoadCursorW(nullptr, IDC_ARROW));
					return 0;
				}
				if (window->hoveredButtonId_ != -1 || window->hoveredCloseButtonId_ != -1) {
					window->hoveredButtonId_ = -1;
					window->hoveredCloseButtonId_ = -1;
					window->invalidateTitleButton(0);
				}
				if (window->pressedButtonId_ >= 0)
					return 0;
				window->mouseMove(window->pixelsToCoords(Point{point.x, point.y}));
				break;
			}
			/* Triggered when mouse leaves the window. There
 			 */
			case WM_MOUSELEAVE:
				if (window->hoveredButtonId_ != -1 || window->hoveredCloseButtonId_ != -1) {
					window->hoveredButtonId_ = -1;
					window->hoveredCloseButtonId_ = -1;
					window->invalidateTitleButton(0);
				}
                window->mouseOut();
				break;
            /** When mouse is moved, the cursor is always reset to the window's default cursor, so to keep the cursor set by the application, we must make sure to set it each time the setcursor message is received. 
             */
            case WM_SETCURSOR: 
                if (LOWORD(lParam) == HTCLIENT) {
                    POINT point{};
                    GetCursorPos(&point);
                    ScreenToClient(hWnd, &point);
                    SetCursor(!window->fullscreen() && point.y < window->captionControlHeight()
                        ? LoadCursorW(nullptr, IDC_ARROW) : window->mouseCursor_);
                    return TRUE;
                }
                break;            
        }
        return DefWindowProc(hWnd, msg, wParam, lParam);
    }


} // namespace tpp

#endif
