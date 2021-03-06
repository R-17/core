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

#include <memory>
#include <comphelper/processfactory.hxx>
#include <osl/diagnose.h>

#include <vcl/taskpanelist.hxx>
#include <vcl/settings.hxx>
#include <vcl/commandevent.hxx>
#include <vcl/event.hxx>
#include <vcl/svapp.hxx>

#include <svtools/framestatuslistener.hxx>
#include <svtools/valueset.hxx>
#include <svtools/toolbarmenu.hxx>

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::frame;
using namespace ::com::sun::star::accessibility;

namespace svtools {

static vcl::Window* GetTopMostParentSystemWindow( vcl::Window* pWindow )
{
    OSL_ASSERT( pWindow );
    if ( pWindow )
    {
        // ->manually search topmost system window
        // required because their might be another system window between this and the top window
        pWindow = pWindow->GetParent();
        SystemWindow* pTopMostSysWin = nullptr;
        while ( pWindow )
        {
            if ( pWindow->IsSystemWindow() )
                pTopMostSysWin = static_cast<SystemWindow*>(pWindow);
            pWindow = pWindow->GetParent();
        }
        pWindow = pTopMostSysWin;
        OSL_ASSERT( pWindow );
        return pWindow;
    }

    return nullptr;
}

class ToolbarPopupStatusListener : public svt::FrameStatusListener
{
public:
    ToolbarPopupStatusListener( const css::uno::Reference< css::frame::XFrame >& xFrame,
                                ToolbarPopupBase& rToolbarPopup );

    virtual void SAL_CALL dispose() override;
    virtual void SAL_CALL statusChanged( const css::frame::FeatureStateEvent& Event ) override;

    ToolbarPopupBase* mpPopup;
};


ToolbarPopupStatusListener::ToolbarPopupStatusListener(
    const css::uno::Reference< css::frame::XFrame >& xFrame,
    ToolbarPopupBase& rToolbarPopup )
: svt::FrameStatusListener( ::comphelper::getProcessComponentContext(), xFrame )
, mpPopup( &rToolbarPopup )
{
}


void SAL_CALL ToolbarPopupStatusListener::dispose()
{
    mpPopup = nullptr;
    svt::FrameStatusListener::dispose();
}


void SAL_CALL ToolbarPopupStatusListener::statusChanged( const css::frame::FeatureStateEvent& Event )
{
    if( mpPopup )
        mpPopup->statusChanged( Event );
}

ToolbarPopupBase::ToolbarPopupBase(const css::uno::Reference<css::frame::XFrame>& rFrame)
    : mxFrame(rFrame)
{
}

ToolbarPopupBase::~ToolbarPopupBase()
{
    if (mxStatusListener.is())
    {
        mxStatusListener->dispose();
        mxStatusListener.clear();
    }
}

ToolbarPopup::ToolbarPopup( const css::uno::Reference<css::frame::XFrame>& rFrame, vcl::Window* pParentWindow,
                            const OString& rID, const OUString& rUIXMLDescription )
    : DockingWindow(pParentWindow, rID, rUIXMLDescription, rFrame)
    , ToolbarPopupBase(rFrame)
{
    init();
}

void ToolbarPopup::init()
{
    vcl::Window* pWindow = GetTopMostParentSystemWindow( this );
    if ( pWindow )
        static_cast<SystemWindow*>(pWindow)->GetTaskPaneList()->AddWindow( this );
}

ToolbarPopup::~ToolbarPopup()
{
    disposeOnce();
}

void ToolbarPopup::dispose()
{
    vcl::Window* pWindow = GetTopMostParentSystemWindow( this );
    if ( pWindow )
        static_cast<SystemWindow*>(pWindow)->GetTaskPaneList()->RemoveWindow( this );

    if ( mxStatusListener.is() )
    {
        mxStatusListener->dispose();
        mxStatusListener.clear();
    }

    mxFrame.clear();
    DockingWindow::dispose();
}

void ToolbarPopupBase::AddStatusListener( const OUString& rCommandURL )
{
    if( !mxStatusListener.is() )
        mxStatusListener.set( new ToolbarPopupStatusListener( mxFrame, *this ) );

    mxStatusListener->addStatusListener( rCommandURL );
}

void ToolbarPopupBase::statusChanged( const css::frame::FeatureStateEvent& /*Event*/ )
{
}

bool ToolbarPopup::IsInPopupMode() const
{
    return GetDockingManager()->IsInPopupMode(this);
}

void ToolbarPopup::EndPopupMode()
{
    GetDockingManager()->EndPopupMode(this);
}


}

WeldToolbarPopup::WeldToolbarPopup(const css::uno::Reference<css::frame::XFrame>& rFrame,
                                   weld::Widget* pParent, const OUString& rUIFile,
                                   const OString& rId)
    : ToolbarPopupBase(rFrame)
    , m_xBuilder(Application::CreateBuilder(pParent, rUIFile))
    , m_xTopLevel(m_xBuilder->weld_container(rId))
    , m_xContainer(m_xBuilder->weld_container("container"))
{
    m_xTopLevel->connect_focus_in(LINK(this, WeldToolbarPopup, FocusHdl));
}

WeldToolbarPopup::~WeldToolbarPopup()
{
}

IMPL_LINK_NOARG(WeldToolbarPopup, FocusHdl, weld::Widget&, void)
{
    GrabFocus();
}

ToolbarPopupContainer::ToolbarPopupContainer(weld::Widget* pParent)
    : m_xBuilder(Application::CreateBuilder(pParent, "svx/ui/toolbarpopover.ui"))
    , m_xTopLevel(m_xBuilder->weld_container("ToolbarPopover"))
    , m_xContainer(m_xBuilder->weld_container("container"))
{
    m_xTopLevel->connect_focus_in(LINK(this, ToolbarPopupContainer, FocusHdl));
}

void ToolbarPopupContainer::setPopover(std::unique_ptr<WeldToolbarPopup> xPopup)
{
    m_xPopup = std::move(xPopup);
    // move the WeldToolbarPopup contents into this toolbar so on-demand contents can appear inside a preexisting gtk popover
    // because the arrow for the popover is only enabled if there's a popover set
    m_xPopup->getTopLevel()->move(m_xPopup->getContainer(), m_xContainer.get());
    m_xPopup->GrabFocus();
}

void ToolbarPopupContainer::unsetPopover()
{
    if (!m_xPopup)
        return;
    m_xContainer->move(m_xPopup->getContainer(), m_xPopup->getTopLevel());
    m_xPopup.reset();
}

ToolbarPopupContainer::~ToolbarPopupContainer()
{
    unsetPopover();
}

IMPL_LINK_NOARG(ToolbarPopupContainer, FocusHdl, weld::Widget&, void)
{
    if (m_xPopup)
        m_xPopup->GrabFocus();
}

InterimToolbarPopup::InterimToolbarPopup(const css::uno::Reference<css::frame::XFrame>& rFrame, vcl::Window* pParent,
                                         std::unique_ptr<WeldToolbarPopup> xPopup)
    : ToolbarPopup(rFrame, pParent, "InterimDockParent", "svx/ui/interimdockparent.ui")
    , m_xBox(get<vcl::Window>("box"))
    , m_xBuilder(Application::CreateInterimBuilder(m_xBox.get(), "svx/ui/interimparent.ui"))
    , m_xContainer(m_xBuilder->weld_container("container"))
    , m_xPopup(std::move(xPopup))
{
    // move the WeldToolbarPopup contents into this interim toolbar so welded contents can appear as a dropdown in an unwelded toolbar
    m_xPopup->getTopLevel()->move(m_xPopup->getContainer(), m_xContainer.get());
}

void InterimToolbarPopup::GetFocus()
{
    ToolbarPopup::GetFocus();
    m_xPopup->GrabFocus();
}

void InterimToolbarPopup::dispose()
{
    // if we have focus when disposed, pick the document window as destination
    // for focus rather than let it go to an arbitrary windows
    if (HasFocus())
    {
        if (auto xWindow = mxFrame->getContainerWindow())
            xWindow->setFocus();
    }
    // move the contents back where it belongs
    m_xContainer->move(m_xPopup->getContainer(), m_xPopup->getTopLevel());
    m_xPopup.reset();
    m_xContainer.reset();
    m_xBox.clear();
    ToolbarPopup::dispose();
}

InterimToolbarPopup::~InterimToolbarPopup()
{
    disposeOnce();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
