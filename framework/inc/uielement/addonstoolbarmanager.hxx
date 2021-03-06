/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#ifndef INCLUDED_FRAMEWORK_INC_UIELEMENT_ADDONSTOOLBARMANAGER_HXX
#define INCLUDED_FRAMEWORK_INC_UIELEMENT_ADDONSTOOLBARMANAGER_HXX

#include <uielement/toolbarmanager.hxx>

#include <com/sun/star/frame/XFrame.hpp>

#include <rtl/ustring.hxx>

class ToolBox;

namespace framework
{

class AddonsToolBarManager final : public ToolBarManager
{
    public:
        AddonsToolBarManager( const css::uno::Reference< css::uno::XComponentContext >& rxContext,
                              const css::uno::Reference< css::frame::XFrame >& rFrame,
                              const OUString& rResourceName,
                              ToolBox* pToolBar );
        virtual ~AddonsToolBarManager() override;

        // XComponent
        void SAL_CALL dispose() override;

        virtual void RefreshImages() override;
        using ToolBarManager::FillToolbar;
        void FillToolbar( const css::uno::Sequence< css::uno::Sequence< css::beans::PropertyValue > >& rAddonToolbar );

    private:
        DECL_LINK(Click, ToolBox *, void);
        DECL_LINK(DoubleClick, ToolBox *, void);
        DECL_LINK(Select, ToolBox *, void);
        DECL_LINK(StateChanged, StateChangedType const *, void );
        DECL_LINK(DataChanged, DataChangedEvent const *, void );

        virtual bool MenuItemAllowed( sal_uInt16 ) const override;
};

}

#endif // INCLUDED_FRAMEWORK_INC_UIELEMENT_ADDONSTOOLBARMANAGER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
