/****************************************************************************
**
** Copyright (C) 2018 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the QtGui module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 3 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL3 included in the
** packaging of this file. Please review the following information to
** ensure the GNU Lesser General Public License version 3 requirements
** will be met: https://www.gnu.org/licenses/lgpl-3.0.html.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 2.0 or (at your option) the GNU General
** Public license version 3 or any later version approved by the KDE Free
** Qt Foundation. The licenses are as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL2 and LICENSE.GPL3
** included in the packaging of this file. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-2.0.html and
** https://www.gnu.org/licenses/gpl-3.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/

#ifndef QXKBCOMMON_P_H
#define QXKBCOMMON_P_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#include <QtCore/QString>
#include <QtCore/QVector>
#include <QtCore/QLoggingCategory>
#include <QtCore/QList>

#include <xkbcommon/xkbcommon.h>

#include <memory>

QT_BEGIN_NAMESPACE

Q_DECLARE_LOGGING_CATEGORY(lcXkbcommon)

class QEvent;
class QKeyEvent;
class QPlatformInputContext;

class QXkbCommon
{
public:
    static QString lookupString(struct xkb_state *state, xkb_keycode_t code);
    static QString lookupStringNoKeysymTransformations(xkb_keysym_t keysym);

    static QVector<xkb_keysym_t> toKeysym(QKeyEvent *event);

    static int keysymToQtKey(xkb_keysym_t keysym, Qt::KeyboardModifiers modifiers);
    static int keysymToQtKey(xkb_keysym_t keysym, Qt::KeyboardModifiers modifiers,
                             xkb_state *state, xkb_keycode_t code,
                             bool superAsMeta = false, bool hyperAsMeta = false);

    // xkbcommon_* API is part of libxkbcommon internals, with modifications as
    // desribed in the header of the implementation file.
    static void xkbcommon_XConvertCase(xkb_keysym_t sym, xkb_keysym_t *lower, xkb_keysym_t *upper);
    static xkb_keysym_t qxkbcommon_xkb_keysym_to_upper(xkb_keysym_t ks);

    static Qt::KeyboardModifiers modifiers(struct xkb_state *state);

    static QList<int> possibleKeys(xkb_state *state, const QKeyEvent *event,
                                   bool superAsMeta = false, bool hyperAsMeta = false);

    static void verifyHasLatinLayout(xkb_keymap *keymap);
    static xkb_keysym_t lookupLatinKeysym(xkb_state *state, xkb_keycode_t keycode);

    static bool isLatin1(xkb_keysym_t sym) {
        return sym <= 0xff;
    }
    static bool isKeypad(xkb_keysym_t sym) {
        switch (sym) {
        case XKB_KEY_KP_Space:
        case XKB_KEY_KP_Tab:
        case XKB_KEY_KP_Enter:
        case XKB_KEY_KP_F1:
        case XKB_KEY_KP_F2:
        case XKB_KEY_KP_F3:
        case XKB_KEY_KP_F4:
        case XKB_KEY_KP_Home:
        case XKB_KEY_KP_Left:
        case XKB_KEY_KP_Up:
        case XKB_KEY_KP_Right:
        case XKB_KEY_KP_Down:
        case XKB_KEY_KP_Prior:
        case XKB_KEY_KP_Next:
        case XKB_KEY_KP_End:
        case XKB_KEY_KP_Begin:
        case XKB_KEY_KP_Insert:
        case XKB_KEY_KP_Delete:
        case XKB_KEY_KP_Equal:
        case XKB_KEY_KP_Multiply:
        case XKB_KEY_KP_Add:
        case XKB_KEY_KP_Separator:
        case XKB_KEY_KP_Subtract:
        case XKB_KEY_KP_Decimal:
        case XKB_KEY_KP_Divide:
        case XKB_KEY_KP_0:
        case XKB_KEY_KP_1:
        case XKB_KEY_KP_2:
        case XKB_KEY_KP_3:
        case XKB_KEY_KP_4:
        case XKB_KEY_KP_5:
        case XKB_KEY_KP_6:
        case XKB_KEY_KP_7:
        case XKB_KEY_KP_8:
        case XKB_KEY_KP_9:
            return true;
        default:
            return false;
        }
    }

    static void setXkbContext(QPlatformInputContext *inputContext, struct xkb_context *context);

    struct XKBStateDeleter {
        void operator()(struct xkb_state *state) const { return xkb_state_unref(state); }
    };
    struct XKBKeymapDeleter {
        void operator()(struct xkb_keymap *keymap) const { return xkb_keymap_unref(keymap); }
    };
    struct XKBContextDeleter {
        void operator()(struct xkb_context *context) const { return xkb_context_unref(context); }
    };
    using ScopedXKBState = std::unique_ptr<struct xkb_state, XKBStateDeleter>;
    using ScopedXKBKeymap = std::unique_ptr<struct xkb_keymap, XKBKeymapDeleter>;
    using ScopedXKBContext = std::unique_ptr<struct xkb_context, XKBContextDeleter>;
};

QT_END_NAMESPACE

#endif // QXKBCOMMON_P_H
