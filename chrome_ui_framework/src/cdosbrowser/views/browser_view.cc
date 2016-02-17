// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "cdosbrowser/views/browser_view.h"

#include <algorithm>
#include <string>

#include "base/memory/scoped_vector.h"
#include "base/strings/utf_string_conversions.h"
#include "ui/base/models/combobox_model.h"
#include "ui/base/ui_base_paths.h"
#include "ui/gfx/canvas.h"
#include "ui/gfx/skia_util.h"
#include "ui/views/view.h"
#include "ui/views/widget/widget.h"

namespace views {

void ShowBrowserWindow() {
  views::Widget* widget = new Widget;
  Widget::InitParams params;
  params.delegate = new BrowserView();
  params.context = NULL;
  widget->Init(params);
  widget->Show();
}

BrowserView::BrowserView() {
  Init();
}

BrowserView::~BrowserView() {
}

base::string16 BrowserView::GetWindowTitle() const {
  return base::ASCIIToUTF16("CDosBrowser");
}

gfx::Size BrowserView::GetPreferredSize() const  {
  return gfx::Size(800, 300); 
}
 
void BrowserView::WindowClosing() {
  base::MessageLoopForUI::current()->QuitWhenIdle();
}

bool BrowserView::CanResize() const {
  return true; 
}

bool BrowserView::CanMaximize() const  { 
  return true; 
}

bool BrowserView::CanMinimize() const { 
  return true; 
}

View* BrowserView::GetContentsView()  { 
  return this; 
}

void BrowserView::OnPaint(gfx::Canvas* canvas) {
  views::View::OnPaint(canvas);
  // background set to white
  SkColor background_color = SK_ColorWHITE;
  canvas->FillRect(gfx::Rect(0, 0, width(), height()), background_color);
};

void BrowserView::Init() {
  // todo : add the child views here ...

}
}  // namespace views
