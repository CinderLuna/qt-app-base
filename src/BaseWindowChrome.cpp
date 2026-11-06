// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2026 Cinderluna

#include "qtappbase/BaseWindow.h"
#include "qtappbase/WindowHeader.h"
#include <QToolButton>
#include <QtMath>

#if defined(Q_OS_WIN)
#include <qt_windows.h>
#include <windowsx.h>
#endif

namespace qtappbase
{

#if defined(Q_OS_WIN)
namespace
{

constexpr wchar_t CaptionProcedureProperty[] = L"QtAppBase.CaptionOriginalProcedure";

LRESULT CALLBACK captionHelperProcedure(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    const auto original = reinterpret_cast<WNDPROC>(GetPropW(hwnd, CaptionProcedureProperty));
    if (!original)
        return DefWindowProcW(hwnd, message, wParam, lParam);

    if (message == WM_NCDESTROY)
    {
        // Do not leave our callback/property on a recycled handle. Preserve any
        // later native subclass's chain while allowing Qt's own teardown to run.
        if (reinterpret_cast<WNDPROC>(GetWindowLongPtrW(hwnd, GWLP_WNDPROC)) == captionHelperProcedure)
            SetWindowLongPtrW(hwnd, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(original));
        RemovePropW(hwnd, CaptionProcedureProperty);
        return CallWindowProcW(original, hwnd, message, wParam, lParam);
    }

    const auto result = CallWindowProcW(original, hwnd, message, wParam, lParam);
    if ((message == WM_SHOWWINDOW || (message == WM_TIMER && wParam == 1)) &&
        GetPropW(hwnd, CaptionProcedureProperty) == reinterpret_cast<HANDLE>(original))
    {
        // Cancel Qt's blank helper timer so it cannot trigger continuous repaint timers.
        KillTimer(hwnd, 1);
    }
    return result;
}

void stopUnusedCaptionAnimation(HWND owner)
{
    if (!WindowHeader::supportsExpandedTitleBar())
        return;

    EnumWindows(
        [](HWND hwnd, LPARAM parameter) -> BOOL
        {
            const auto ownerHwnd = reinterpret_cast<HWND>(parameter);
            if (GetWindow(hwnd, GW_OWNER) != ownerHwnd)
                return TRUE;

            DWORD process = 0;
            const auto thread = GetWindowThreadProcessId(hwnd, &process);
            wchar_t name[64]{};
            if (process != GetCurrentProcessId() || thread != GetCurrentThreadId() ||
                !GetClassNameW(hwnd, name, 64) || wcscmp(name, L"_q_titlebar") != 0)
            {
                return TRUE;
            }

            if (!GetPropW(hwnd, CaptionProcedureProperty))
            {
                const auto original = GetWindowLongPtrW(hwnd, GWLP_WNDPROC);
                if (!original ||
                    !SetPropW(hwnd, CaptionProcedureProperty, reinterpret_cast<HANDLE>(original)))
                {
                    return TRUE;
                }

                SetLastError(ERROR_SUCCESS);
                if (!SetWindowLongPtrW(hwnd, GWLP_WNDPROC,
                                       reinterpret_cast<LONG_PTR>(captionHelperProcedure)) &&
                    GetLastError() != ERROR_SUCCESS)
                {
                    RemovePropW(hwnd, CaptionProcedureProperty);
                    return TRUE;
                }
            }

            KillTimer(hwnd, 1);
            return TRUE;
        },
        reinterpret_cast<LPARAM>(owner));
}

} // namespace
#endif

void BaseWindow::syncWindowsChrome()
{
#if defined(Q_OS_WIN)
    if (!extendedHeader_ || isFullScreen())
        return;

    const auto hwnd = reinterpret_cast<HWND>(winId());
    // Preserve native system-command support/animation and DWM drop shadow while Qt's
    // expanded-client backend owns WM_NCCALCSIZE.
    const LONG_PTR oldStyle = GetWindowLongPtrW(hwnd, GWL_STYLE);
    const LONG_PTR style =
        (oldStyle & ~WS_POPUP) | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX | WS_MAXIMIZEBOX | WS_THICKFRAME;

    if (style != oldStyle)
    {
        SetWindowLongPtrW(hwnd, GWL_STYLE, style);
        SetWindowPos(hwnd, nullptr, 0, 0, 0, 0,
                     SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE | SWP_FRAMECHANGED);
    }

    stopUnusedCaptionAnimation(hwnd);
#endif
}

bool BaseWindow::nativeEvent(const QByteArray &eventType, void *message, qintptr *result)
{
#if defined(Q_OS_WIN)
    const auto *msg = static_cast<MSG *>(message);
    if (extendedHeader_ && !isFullScreen() && header_ && header_->maximizeButton())
    {
        auto *maximize = header_->maximizeButton();

        if (msg->message == WM_SYSCHAR && msg->wParam == VK_SPACE)
        {
            // TranslateMessage may queue this after Alt+Space's key-down;
            // the system menu has already been handled once.
            *result = 0;
            return true;
        }

        if ((msg->message == WM_NCRBUTTONUP && msg->wParam == HTCAPTION) ||
            (msg->message == WM_SYSKEYDOWN && msg->wParam == VK_SPACE) ||
            (msg->message == WM_SYSCOMMAND && (msg->wParam & 0xFFF0) == SC_KEYMENU &&
             msg->lParam == VK_SPACE))
        {
            header_->showSystemMenu(msg->message == WM_NCRBUTTONUP);
            *result = 0;
            return true;
        }

        if (msg->message == WM_SYSCOMMAND)
        {
            const auto command = msg->wParam & 0xFFF0;
            if (command == SC_MINIMIZE || command == SC_MAXIMIZE || command == SC_RESTORE)
            {
                // DefWindowProc performs the system action; WM_SIZE still flows
                // through Qt and updates its window state and our restore glyph.
                *result = DefWindowProcW(msg->hwnd, msg->message, msg->wParam, msg->lParam);
                return true;
            }
        }

        if (msg->message == WM_NCHITTEST)
        {
            POINT nativePoint{GET_X_LPARAM(msg->lParam), GET_Y_LPARAM(msg->lParam)};
            ScreenToClient(msg->hwnd, &nativePoint);

            const qreal scale = devicePixelRatioF();
            const QPoint point(qFloor(nativePoint.x / scale), qFloor(nativePoint.y / scale));
            const UINT dpi = GetDpiForWindow(msg->hwnd);
            const int border = IsZoomed(msg->hwnd) ? 0
                                                   : GetSystemMetricsForDpi(SM_CXSIZEFRAME, dpi) +
                                                         GetSystemMetricsForDpi(SM_CXPADDEDBORDER, dpi);

            const bool left = nativePoint.x < border;
            const bool right = nativePoint.x >= width() * scale - border;
            const bool top = nativePoint.y < border;
            const bool bottom = nativePoint.y >= height() * scale - border;

            if (border && (left || right || top || bottom))
            {
                *result = left    ? (top      ? HTTOPLEFT
                                     : bottom ? HTBOTTOMLEFT
                                              : HTLEFT)
                          : right ? (top      ? HTTOPRIGHT
                                     : bottom ? HTBOTTOMRIGHT
                                              : HTRIGHT)
                          : top   ? HTTOP
                                  : HTBOTTOM;
                return true;
            }

            // Hit test caption buttons
            for (auto *button : header_->findChildren<QToolButton *>())
            {
                if (button->isVisible() &&
                    QRect(button->mapTo(this, QPoint()), button->size()).contains(point))
                {
                    *result = (button == maximize) ? HTMAXBUTTON : HTCLIENT;
                    return true;
                }
            }

            // Hit test title bar area
            const QPoint headerPoint = header_->mapFrom(this, point);
            if (header_->rect().contains(headerPoint))
            {
                const auto *child = header_->childAt(headerPoint);
                // If child is null or header itself, or child is marked transparent (like title or brand
                // icon), treat as draggable caption area. If child is an interactive widget, treat as
                // HTCLIENT.
                const bool isInteractive =
                    child && (child != header_) && !child->testAttribute(Qt::WA_TransparentForMouseEvents);
                *result = isInteractive ? HTCLIENT : HTCAPTION;
                return true;
            }

            *result = HTCLIENT;
            return true;
        }

        if (msg->message == WM_NCLBUTTONDBLCLK && msg->wParam == HTCAPTION)
        {
            header_->requestMaximizeRestore();
            *result = 0;
            return true;
        }

        if (msg->message == WM_NCLBUTTONDOWN && msg->wParam == HTCAPTION)
        {
            *result = DefWindowProcW(msg->hwnd, msg->message, msg->wParam, msg->lParam);
            return true;
        }

        // --- Windows 11 Snap Layouts hover & click handling ---
        if ((msg->message == WM_NCLBUTTONDOWN || msg->message == WM_NCLBUTTONDBLCLK) &&
            msg->wParam == HTMAXBUTTON)
        {
            maximize->setDown(true);
            *result = 0;
            return true;
        }

        if (msg->message == WM_NCMOUSEMOVE && msg->wParam == HTMAXBUTTON && !maximize->underMouse())
        {
            maximize->setAttribute(Qt::WA_UnderMouse, true);
            maximize->update();
            TRACKMOUSEEVENT tracking{sizeof(TRACKMOUSEEVENT), TME_LEAVE | TME_NONCLIENT, msg->hwnd, 0};
            TrackMouseEvent(&tracking);
        }

        if (msg->message == WM_NCLBUTTONUP && msg->wParam == HTMAXBUTTON)
        {
            const bool pressed = maximize->isDown();
            maximize->setDown(false);
            if (pressed)
                header_->requestMaximizeRestore();
            *result = 0;
            return true;
        }

        if (msg->message == WM_NCMOUSELEAVE || msg->message == WM_CANCELMODE || msg->message == WM_LBUTTONUP)
        {
            maximize->setDown(false);
            maximize->setAttribute(Qt::WA_UnderMouse, false);
            maximize->update();
        }
    }
#endif
    return QMainWindow::nativeEvent(eventType, message, result);
}

} // namespace qtappbase
