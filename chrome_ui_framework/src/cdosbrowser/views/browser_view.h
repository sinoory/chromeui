// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef UI_VIEWS_EXAMPLES_EXAMPLES_WINDOW_H_
#define UI_VIEWS_EXAMPLES_EXAMPLES_WINDOW_H_

#include "base/memory/scoped_ptr.h"
#include "base/memory/scoped_vector.h"
#include "ui/gfx/native_widget_types.h"
#include "cdosbrowser/views_examples_export.h"
#include "ui/views/widget/widget_delegate.h"

namespace aura {
class Window;
}

namespace views {

// Shows a window with the views examples in it. |extra_examples| contains any
// additional examples to add. |window_context| is used to determine where the
// window should be created (see |Widget::InitParams::context| for details).
VIEWS_EXAMPLES_EXPORT void ShowBrowserWindow();

class BrowserView : public WidgetDelegateView {
public:
  BrowserView();
  ~BrowserView() override;
  
  gfx::Size GetPreferredSize() const override;
  void OnPaint(gfx::Canvas* canvas) override; 
  void Init();

private:
  // WidgetDelegateView:
  bool CanResize() const override;
  bool CanMaximize() const override;
  bool CanMinimize() const override;
  base::string16 GetWindowTitle() const override;
  View* GetContentsView() override ;
  void WindowClosing() override;


  DISALLOW_COPY_AND_ASSIGN(BrowserView);
};
}  // namespace views

#endif  // UI_VIEWS_EXAMPLES_EXAMPLES_WINDOW_H_
