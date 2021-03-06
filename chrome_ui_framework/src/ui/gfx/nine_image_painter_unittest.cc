// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ui/gfx/nine_image_painter.h"

#include "testing/gtest/include/gtest/gtest.h"
#include "ui/gfx/canvas.h"
#include "ui/gfx/geometry/insets.h"
#include "ui/gfx/geometry/rect.h"
#include "ui/gfx/image/image_skia.h"

namespace gfx {

TEST(NineImagePainterTest, GetSubsetRegions) {
  SkBitmap src;
  src.allocN32Pixels(40, 50);
  const ImageSkia image_skia(ImageSkiaRep(src, 1.0));
  const Insets insets(1, 2, 3, 4);
  std::vector<Rect> rects;
  NineImagePainter::GetSubsetRegions(image_skia, insets, &rects);
  ASSERT_EQ(9u, rects.size());
  EXPECT_EQ(gfx::Rect(0, 0, 2, 1), rects[0]);
  EXPECT_EQ(gfx::Rect(2, 0, 34, 1), rects[1]);
  EXPECT_EQ(gfx::Rect(36, 0, 4, 1), rects[2]);
  EXPECT_EQ(gfx::Rect(0, 1, 2, 46), rects[3]);
  EXPECT_EQ(gfx::Rect(2, 1, 34, 46), rects[4]);
  EXPECT_EQ(gfx::Rect(36, 1, 4, 46), rects[5]);
  EXPECT_EQ(gfx::Rect(0, 47, 2, 3), rects[6]);
  EXPECT_EQ(gfx::Rect(2, 47, 34, 3), rects[7]);
  EXPECT_EQ(gfx::Rect(36, 47, 4, 3), rects[8]);
}

TEST(NineImagePainterTest, PaintScale) {
  SkBitmap src;
  src.allocN32Pixels(100, 100);
  src.eraseColor(SK_ColorRED);
  src.eraseArea(SkIRect::MakeXYWH(10, 10, 80, 80), SK_ColorGREEN);

  gfx::ImageSkia image(gfx::ImageSkiaRep(src, 0.0f));
  gfx::Insets insets(10, 10, 10, 10);
  gfx::NineImagePainter painter(image, insets);

  int image_scale = 2;
  bool is_opaque = true;
  gfx::Canvas canvas(gfx::Size(400, 400), image_scale, is_opaque);
  canvas.Scale(2, 1);

  gfx::Rect bounds(0, 0, 100, 100);
  painter.Paint(&canvas, bounds);

  SkBitmap result;
  const SkISize size = canvas.sk_canvas()->getDeviceSize();
  result.allocN32Pixels(size.width(), size.height());
  canvas.sk_canvas()->readPixels(&result, 0, 0);

  SkIRect green_rect = SkIRect::MakeLTRB(40, 20, 360, 180);
  for (int y = 0; y < 200; y++) {
    for (int x = 0; x < 400; x++) {
      if (green_rect.contains(x, y)) {
        ASSERT_EQ(SK_ColorGREEN, result.getColor(x, y));
      } else {
        ASSERT_EQ(SK_ColorRED, result.getColor(x, y));
      }
    }
  }
}

TEST(NineImagePainterTest, PaintStaysInBounds) {
  // In this test the bounds rect is 1x1 but each image is 2x2.
  // The NineImagePainter should not paint outside the bounds.

  SkBitmap src;
  src.allocN32Pixels(6, 6);
  src.eraseColor(SK_ColorRED);

  gfx::ImageSkia image(gfx::ImageSkiaRep(src, 0.0f));
  gfx::Insets insets(2, 2, 2, 2);
  gfx::NineImagePainter painter(image, insets);

  int image_scale = 1;
  bool is_opaque = true;
  gfx::Canvas canvas(gfx::Size(3, 3), image_scale, is_opaque);
  canvas.DrawColor(SK_ColorBLACK);

  gfx::Rect bounds(1, 1, 1, 1);
  painter.Paint(&canvas, bounds);

  SkBitmap result;
  const SkISize size = canvas.sk_canvas()->getDeviceSize();
  result.allocN32Pixels(size.width(), size.height());
  canvas.sk_canvas()->readPixels(&result, 0, 0);

  EXPECT_EQ(SK_ColorRED, result.getColor(1, 1));

  EXPECT_EQ(SK_ColorBLACK, result.getColor(0, 0));
  EXPECT_EQ(SK_ColorBLACK, result.getColor(0, 1));
  EXPECT_EQ(SK_ColorBLACK, result.getColor(0, 2));
  EXPECT_EQ(SK_ColorBLACK, result.getColor(1, 0));
  EXPECT_EQ(SK_ColorBLACK, result.getColor(1, 2));
  EXPECT_EQ(SK_ColorBLACK, result.getColor(2, 0));
  EXPECT_EQ(SK_ColorBLACK, result.getColor(2, 1));
  EXPECT_EQ(SK_ColorBLACK, result.getColor(2, 2));
}

TEST(NineImagePainterTest, PaintWithBoundOffset) {
  SkBitmap src;
  src.allocN32Pixels(10, 10);
  src.eraseColor(SK_ColorRED);
  src.eraseArea(SkIRect::MakeXYWH(1, 1, 8, 8), SK_ColorGREEN);

  gfx::ImageSkia image(gfx::ImageSkiaRep(src, 0.0f));
  gfx::Insets insets(1, 1, 1, 1);
  gfx::NineImagePainter painter(image, insets);

  bool is_opaque = true;
  gfx::Canvas canvas(gfx::Size(10, 10), 1, is_opaque);

  gfx::Rect bounds(1, 1, 10, 10);
  painter.Paint(&canvas, bounds);

  SkBitmap result;
  const SkISize size = canvas.sk_canvas()->getDeviceSize();
  result.allocN32Pixels(size.width(), size.height());
  canvas.sk_canvas()->readPixels(&result, 0, 0);

  SkIRect green_rect = SkIRect::MakeLTRB(2, 2, 10, 10);
  for (int y = 1; y < 10; y++) {
    for (int x = 1; x < 10; x++) {
      if (green_rect.contains(x, y)) {
        ASSERT_EQ(SK_ColorGREEN, result.getColor(x, y));
      } else {
        ASSERT_EQ(SK_ColorRED, result.getColor(x, y));
      }
    }
  }
}

TEST(NineImagePainterTest, PaintWithNagativeScale) {
  SkBitmap src;
  src.allocN32Pixels(100, 100);
  src.eraseColor(SK_ColorRED);
  src.eraseArea(SkIRect::MakeXYWH(10, 10, 80, 80), SK_ColorGREEN);

  gfx::ImageSkia image(gfx::ImageSkiaRep(src, 0.0f));
  gfx::Insets insets(10, 10, 10, 10);
  gfx::NineImagePainter painter(image, insets);

  int image_scale = 2;
  bool is_opaque = true;
  gfx::Canvas canvas(gfx::Size(400, 400), image_scale, is_opaque);
  canvas.Translate(gfx::Vector2d(200, 200));
  canvas.Scale(-2, -1);

  gfx::Rect bounds(0, 0, 100, 100);
  painter.Paint(&canvas, bounds);

  SkBitmap result;
  const SkISize size = canvas.sk_canvas()->getDeviceSize();
  result.allocN32Pixels(size.width(), size.height());
  canvas.sk_canvas()->readPixels(&result, 0, 0);

  SkIRect green_rect = SkIRect::MakeLTRB(40, 220, 360, 380);
  for (int y = 200; y < 400; y++) {
    for (int x = 0; x < 400; x++) {
      if (green_rect.contains(x, y)) {
        ASSERT_EQ(SK_ColorGREEN, result.getColor(x, y));
      } else {
        ASSERT_EQ(SK_ColorRED, result.getColor(x, y));
      }
    }
  }
}

}  // namespace gfx
