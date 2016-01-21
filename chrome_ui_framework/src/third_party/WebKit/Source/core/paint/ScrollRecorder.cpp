// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "config.h"
#include "core/paint/ScrollRecorder.h"

#include "platform/graphics/GraphicsContext.h"
#include "platform/graphics/paint/PaintController.h"
#include "platform/graphics/paint/ScrollDisplayItem.h"

namespace blink {

ScrollRecorder::ScrollRecorder(GraphicsContext& context, const DisplayItemClientWrapper& client, PaintPhase phase, const IntSize& currentOffset)
    : m_client(client)
    , m_beginItemType(DisplayItem::paintPhaseToScrollType(phase))
    , m_context(context)
{
    m_context.paintController().createAndAppend<BeginScrollDisplayItem>(m_client, m_beginItemType, currentOffset);
}

ScrollRecorder::~ScrollRecorder()
{
    m_context.paintController().endItem<EndScrollDisplayItem>(m_client, DisplayItem::scrollTypeToEndScrollType(m_beginItemType));
}

} // namespace blink
