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

#include "XMLStylesImportHelper.hxx"
#include "xmlimprt.hxx"
#include <com/sun/star/util/NumberFormat.hpp>
#include <osl/diagnose.h>

using namespace com::sun::star;
using ::std::list;

void ScMyStyleNumberFormats::AddStyleNumberFormat(const OUString& rStyleName, const sal_Int32 nNumberFormat)
{
    aSet.insert(ScMyStyleNumberFormat(rStyleName, nNumberFormat));
}

sal_Int32 ScMyStyleNumberFormats::GetStyleNumberFormat(const OUString& rStyleName)
{
    ScMyStyleNumberFormat aStyleNumberFormat(rStyleName);
    ScMyStyleNumberFormatSet::iterator aItr(aSet.find(aStyleNumberFormat));
    if (aItr == aSet.end())
        return -1;
    else
        return aItr->nNumberFormat;
}

ScMyStyleRanges::ScMyStyleRanges()
{
}

ScMyStyleRanges::~ScMyStyleRanges()
{
}

void ScMyStyleRanges::AddRange(const ScRange& rRange, const sal_Int16 nType)
{
    switch (nType)
    {
        case util::NumberFormat::NUMBER:
        {
            if (!mpNumberList)
                mpNumberList = std::make_shared<ScSimpleRangeList>();
            mpNumberList->addRange(rRange);
        }
        break;
        case util::NumberFormat::TEXT:
        {
            if (!mpTextList)
                mpTextList = std::make_shared<ScSimpleRangeList>();
            mpTextList->addRange(rRange);
        }
        break;
        case util::NumberFormat::TIME:
        {
            if (!mpTimeList)
                mpTimeList = std::make_shared<ScSimpleRangeList>();
            mpTimeList->addRange(rRange);
        }
        break;
        case util::NumberFormat::DATETIME:
        {
            if (!mpDateTimeList)
                mpDateTimeList = std::make_shared<ScSimpleRangeList>();
            mpDateTimeList->addRange(rRange);
        }
        break;
        case util::NumberFormat::PERCENT:
        {
            if (!mpPercentList)
                mpPercentList = std::make_shared<ScSimpleRangeList>();
            mpPercentList->addRange(rRange);
        }
        break;
        case util::NumberFormat::LOGICAL:
        {
            if (!mpLogicalList)
                mpLogicalList = std::make_shared<ScSimpleRangeList>();
            mpLogicalList->addRange(rRange);
        }
        break;
        case util::NumberFormat::UNDEFINED:
        {
            if (!mpUndefinedList)
                mpUndefinedList = std::make_shared<ScSimpleRangeList>();
            mpUndefinedList->addRange(rRange);
        }
        break;
        default:
        {
            OSL_FAIL("wrong type");
        }
        break;
    }
}

void ScMyStyleRanges::AddCurrencyRange(const ScRange& rRange, const o3tl::optional<OUString> & pCurrency)
{
    if (!pCurrencyList)
        pCurrencyList.reset( new ScMyCurrencyStylesSet );
    ScMyCurrencyStyle aStyle;
    if (pCurrency)
        aStyle.sCurrency = *pCurrency;
    auto itPair = pCurrencyList->insert(aStyle);
    itPair.first->mpRanges->addRange(rRange);
}

void ScMyStyleRanges::InsertCol(const sal_Int32 nCol, const sal_Int32 nTab)
{
    if (mpTextList)
        mpTextList->insertCol(static_cast<SCCOL>(nCol), static_cast<SCTAB>(nTab));
    if (mpNumberList)
        mpNumberList->insertCol(static_cast<SCCOL>(nCol), static_cast<SCTAB>(nTab));
    if (mpTimeList)
        mpTimeList->insertCol(static_cast<SCCOL>(nCol), static_cast<SCTAB>(nTab));
    if (mpDateTimeList)
        mpDateTimeList->insertCol(static_cast<SCCOL>(nCol), static_cast<SCTAB>(nTab));
    if (mpPercentList)
        mpPercentList->insertCol(static_cast<SCCOL>(nCol), static_cast<SCTAB>(nTab));
    if (mpLogicalList)
        mpLogicalList->insertCol(static_cast<SCCOL>(nCol), static_cast<SCTAB>(nTab));
    if (mpUndefinedList)
        mpUndefinedList->insertCol(static_cast<SCCOL>(nCol), static_cast<SCTAB>(nTab));

    if (pCurrencyList)
    {
        for (auto& rCurrency : *pCurrencyList)
        {
            rCurrency.mpRanges->insertCol(static_cast<SCCOL>(nCol), static_cast<SCTAB>(nTab));
        }
    }
}

void ScMyStyleRanges::SetStylesToRanges(const list<ScRange>& rRanges,
    const OUString* pStyleName, const sal_Int16 nCellType,
    const OUString* pCurrency, ScXMLImport& rImport)
{
    for (const auto& rRange : rRanges)
        rImport.SetStyleToRange(rRange, pStyleName, nCellType, pCurrency);
}

void ScMyStyleRanges::SetStylesToRanges(const OUString* pStyleName, ScXMLImport& rImport)
{
    if (mpNumberList)
    {
        list<ScRange> aList;
        mpNumberList->getRangeList(aList);
        SetStylesToRanges(aList, pStyleName, util::NumberFormat::NUMBER, nullptr, rImport);
        mpNumberList->clear();
    }
    if (mpTextList)
    {
        list<ScRange> aList;
        mpTextList->getRangeList(aList);
        SetStylesToRanges(aList, pStyleName, util::NumberFormat::TEXT, nullptr, rImport);
        mpTextList->clear();
    }
    if (mpTimeList)
    {
        list<ScRange> aList;
        mpTimeList->getRangeList(aList);
        SetStylesToRanges(aList, pStyleName, util::NumberFormat::TIME, nullptr, rImport);
        mpTimeList->clear();
    }
    if (mpDateTimeList)
    {
        list<ScRange> aList;
        mpDateTimeList->getRangeList(aList);
        SetStylesToRanges(aList, pStyleName, util::NumberFormat::DATETIME, nullptr, rImport);
        mpDateTimeList->clear();
    }
    if (mpPercentList)
    {
        list<ScRange> aList;
        mpPercentList->getRangeList(aList);
        SetStylesToRanges(aList, pStyleName, util::NumberFormat::PERCENT, nullptr, rImport);
        mpPercentList->clear();
    }
    if (mpLogicalList)
    {
        list<ScRange> aList;
        mpLogicalList->getRangeList(aList);
        SetStylesToRanges(aList, pStyleName, util::NumberFormat::LOGICAL, nullptr, rImport);
        mpLogicalList->clear();
    }
    if (mpUndefinedList)
    {
        list<ScRange> aList;
        mpUndefinedList->getRangeList(aList);
        SetStylesToRanges(aList, pStyleName, util::NumberFormat::UNDEFINED, nullptr, rImport);
        mpUndefinedList->clear();
    }
    if (pCurrencyList)
    {
        for (const auto& rCurrency : *pCurrencyList)
        {
            list<ScRange> aList;
            rCurrency.mpRanges->getRangeList(aList);
            SetStylesToRanges(aList, pStyleName, util::NumberFormat::CURRENCY, &rCurrency.sCurrency, rImport);
        }
    }
}

ScMyStylesImportHelper::ScMyStylesImportHelper(ScXMLImport& rTempImport)
    :
    aRowDefaultStyle(aCellStyles.end()),
    rImport(rTempImport),
    nCellType(0),
    nPrevCellType(0),
    bPrevRangeAdded(true)
{
}

ScMyStylesImportHelper::~ScMyStylesImportHelper()
{
}

void ScMyStylesImportHelper::ResetAttributes()
{
    pPrevStyleName = std::move(pStyleName);
    pPrevCurrency = std::move(pCurrency);
    nPrevCellType = nCellType;
    nCellType = 0;
}

ScMyStylesMap::iterator ScMyStylesImportHelper::GetIterator(const OUString & rStyleName)
{
    auto it = aCellStyles.find(rStyleName);
    if (it == aCellStyles.end())
        it = aCellStyles.emplace_hint(it, rStyleName, std::make_unique<ScMyStyleRanges>());
    return it;
}

void ScMyStylesImportHelper::AddDefaultRange(const ScRange& rRange)
{
    OSL_ENSURE(aRowDefaultStyle != aCellStyles.end(), "no row default style");
    if (aRowDefaultStyle->first.isEmpty())
    {
        SCCOL nStartCol(rRange.aStart.Col());
        SCCOL nEndCol(rRange.aEnd.Col());
        if (aColDefaultStyles.size() > sal::static_int_cast<sal_uInt32>(nStartCol))
        {
            ScMyStylesMap::iterator aPrevItr(aColDefaultStyles[nStartCol]);
            OSL_ENSURE(aColDefaultStyles.size() > sal::static_int_cast<sal_uInt32>(nEndCol), "too many columns");
            for (SCCOL i = nStartCol + 1; (i <= nEndCol) && (i < sal::static_int_cast<SCCOL>(aColDefaultStyles.size())); ++i)
            {
                if (aPrevItr != aColDefaultStyles[i])
                {
                    OSL_ENSURE(aPrevItr != aCellStyles.end(), "no column default style");
                    ScRange aRange(rRange);
                    aRange.aStart.SetCol(nStartCol);
                    aRange.aEnd.SetCol(i - 1);
                    pPrevStyleName = aPrevItr->first;
                    AddSingleRange(aRange);
                    nStartCol = i;
                    aPrevItr = aColDefaultStyles[i];
                }
            }
            if (aPrevItr != aCellStyles.end())
            {
                ScRange aRange(rRange);
                aRange.aStart.SetCol(nStartCol);
                pPrevStyleName = aPrevItr->first;
                AddSingleRange(aRange);
            }
            else
            {
                OSL_FAIL("no column default style");
            }
        }
        else
        {
            OSL_FAIL("too many columns");
        }
    }
    else
    {
        pPrevStyleName = aRowDefaultStyle->first;
        AddSingleRange(rRange);
    }
}

void ScMyStylesImportHelper::AddSingleRange(const ScRange& rRange)
{
    ScMyStylesMap::iterator aItr(GetIterator(*pPrevStyleName));
    if (nPrevCellType != util::NumberFormat::CURRENCY)
        aItr->second->AddRange(rRange, nPrevCellType);
    else
        aItr->second->AddCurrencyRange(rRange, pPrevCurrency);
}

void ScMyStylesImportHelper::AddRange()
{
    if (pPrevStyleName && !pPrevStyleName->isEmpty())
        AddSingleRange(aPrevRange);
    else
        AddDefaultRange(aPrevRange);
    ResetAttributes();
}

void ScMyStylesImportHelper::AddColumnStyle(const OUString& sStyleName, const sal_Int32 nColumn, const sal_Int32 nRepeat)
{
    OSL_ENSURE(static_cast<sal_uInt32>(nColumn) == aColDefaultStyles.size(), "some columns are absent");
    ScMyStylesMap::iterator aItr(GetIterator(sStyleName));
    aColDefaultStyles.reserve(aColDefaultStyles.size() + nRepeat);
    for (sal_Int32 i = 0; i < nRepeat; ++i)
        aColDefaultStyles.push_back(aItr);
}

void ScMyStylesImportHelper::SetRowStyle(const OUString& sStyleName)
{
    aRowDefaultStyle = GetIterator(sStyleName);
}

void ScMyStylesImportHelper::SetAttributes(o3tl::optional<OUString> pStyleNameP,
    o3tl::optional<OUString> pCurrencyP, const sal_Int16 nCellTypeP)
{
    pStyleName = std::move(pStyleNameP);
    pCurrency = std::move(pCurrencyP);
    nCellType = nCellTypeP;
}

void ScMyStylesImportHelper::AddRange(const ScRange& rRange)
{
    if (!bPrevRangeAdded)
    {
        bool bAddRange(false);
        if (nCellType == nPrevCellType &&
            pStyleName == pPrevStyleName &&
            pCurrency == pPrevCurrency)
        {
            if (rRange.aStart.Row() == aPrevRange.aStart.Row())
            {
                if (rRange.aEnd.Row() == aPrevRange.aEnd.Row())
                {
                    OSL_ENSURE(aPrevRange.aEnd.Col() + 1 == rRange.aStart.Col(), "something went wrong");
                    aPrevRange.aEnd.SetCol(rRange.aEnd.Col());
                }
                else
                    bAddRange = true;
            }
            else
            {
                if (rRange.aStart.Col() == aPrevRange.aStart.Col() &&
                    rRange.aEnd.Col() == aPrevRange.aEnd.Col())
                {
                    OSL_ENSURE(aPrevRange.aEnd.Row() + 1 == rRange.aStart.Row(), "something went wrong");
                    aPrevRange.aEnd.SetRow(rRange.aEnd.Row());
                }
                else
                    bAddRange = true;
            }
        }
        else
            bAddRange = true;
        if (bAddRange)
        {
            AddRange();
            aPrevRange = rRange;
        }
    }
    else
    {
        aPrevRange = rRange;
        ResetAttributes();
        bPrevRangeAdded = false;
    }
}

void ScMyStylesImportHelper::AddCell(const ScAddress& rAddress)
{
    ScRange aScRange( rAddress, rAddress );
    AddRange(aScRange);
}

void ScMyStylesImportHelper::InsertCol(const sal_Int32 nCol, const sal_Int32 nTab)
{
    ScXMLImport::MutexGuard aGuard(rImport);
    for (auto& rCellStyle : aCellStyles)
    {
        rCellStyle.second->InsertCol(nCol, nTab);
    }
}

void ScMyStylesImportHelper::EndTable()
{
    if (!bPrevRangeAdded)
    {
        AddRange();
        bPrevRangeAdded = true;
    }
}

void ScMyStylesImportHelper::SetStylesToRanges()
{
    for (auto& rCellStyle : aCellStyles)
    {
        rCellStyle.second->SetStylesToRanges(&rCellStyle.first, rImport);
    }
    aColDefaultStyles.clear();
    aCellStyles.clear();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
