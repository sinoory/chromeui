// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "config.h"
#include "core/svg/SVGPathParser.h"

#include "core/svg/SVGPathStringBuilder.h"
#include "core/svg/SVGPathStringSource.h"

#include <gtest/gtest.h>

namespace blink {

namespace {

bool parsePath(const char* input, String& output)
{
    String inputString(input);
    SVGPathStringSource source(inputString);
    SVGPathStringBuilder builder;
    SVGPathParser parser(&source, &builder);
    bool hadError = parser.parsePathDataFromSource(UnalteredParsing, true);
    output = builder.result();
    // Coerce a null result to empty.
    if (output.isNull())
        output = emptyString();
    return hadError;
}

#define VALID(input, expected)                  \
    {                                           \
        String output;                          \
        EXPECT_TRUE(parsePath(input, output));  \
        EXPECT_EQ(expected, output);            \
    }

#define MALFORMED(input, expected)              \
    {                                           \
        String output;                          \
        EXPECT_FALSE(parsePath(input, output)); \
        EXPECT_EQ(expected, output);            \
    }

TEST(SVGPathParserTest, Simple)
{
    VALID("M1,2", "M 1 2");
    VALID("m1,2", "m 1 2");
    VALID("M100,200 m3,4", "M 100 200 m 3 4");
    VALID("M100,200 L3,4", "M 100 200 L 3 4");
    VALID("M100,200 l3,4", "M 100 200 l 3 4");
    VALID("M100,200 H3", "M 100 200 H 3");
    VALID("M100,200 h3", "M 100 200 h 3");
    VALID("M100,200 V3", "M 100 200 V 3");
    VALID("M100,200 v3", "M 100 200 v 3");
    VALID("M100,200 Z", "M 100 200 Z");
    VALID("M100,200 z", "M 100 200 Z");
    VALID("M100,200 C3,4,5,6,7,8", "M 100 200 C 3 4 5 6 7 8");
    VALID("M100,200 c3,4,5,6,7,8", "M 100 200 c 3 4 5 6 7 8");
    VALID("M100,200 S3,4,5,6", "M 100 200 S 3 4 5 6");
    VALID("M100,200 s3,4,5,6", "M 100 200 s 3 4 5 6");
    VALID("M100,200 Q3,4,5,6", "M 100 200 Q 3 4 5 6");
    VALID("M100,200 q3,4,5,6", "M 100 200 q 3 4 5 6");
    VALID("M100,200 T3,4", "M 100 200 T 3 4");
    VALID("M100,200 t3,4", "M 100 200 t 3 4");
    VALID("M100,200 A3,4,5,0,0,6,7", "M 100 200 A 3 4 5 0 0 6 7");
    VALID("M100,200 A3,4,5,1,0,6,7", "M 100 200 A 3 4 5 1 0 6 7");
    VALID("M100,200 A3,4,5,0,1,6,7", "M 100 200 A 3 4 5 0 1 6 7");
    VALID("M100,200 A3,4,5,1,1,6,7", "M 100 200 A 3 4 5 1 1 6 7");
    VALID("M100,200 a3,4,5,0,0,6,7", "M 100 200 a 3 4 5 0 0 6 7");
    VALID("M100,200 a3,4,5,0,1,6,7", "M 100 200 a 3 4 5 0 1 6 7");
    VALID("M100,200 a3,4,5,1,0,6,7", "M 100 200 a 3 4 5 1 0 6 7");
    VALID("M100,200 a3,4,5,1,1,6,7", "M 100 200 a 3 4 5 1 1 6 7");
    VALID("M100,200 a3,4,5,006,7", "M 100 200 a 3 4 5 0 0 6 7");
    VALID("M100,200 a3,4,5,016,7", "M 100 200 a 3 4 5 0 1 6 7");
    VALID("M100,200 a3,4,5,106,7", "M 100 200 a 3 4 5 1 0 6 7");
    VALID("M100,200 a3,4,5,116,7", "M 100 200 a 3 4 5 1 1 6 7");
    MALFORMED("M100,200 a3,4,5,2,1,6,7", "M 100 200");
    MALFORMED("M100,200 a3,4,5,1,2,6,7", "M 100 200");

    VALID("M100,200 a0,4,5,0,0,10,0 a4,0,5,0,0,0,10 a0,0,5,0,0,-10,0 z", "M 100 200 a 0 4 5 0 0 10 0 a 4 0 5 0 0 0 10 a 0 0 5 0 0 -10 0 Z");

    VALID("M1,2,3,4", "M 1 2 L 3 4");
    VALID("m100,200,3,4", "m 100 200 l 3 4");

    VALID("M 100-200", "M 100 -200");
    VALID("M 0.6.5", "M 0.6 0.5");

    VALID(" M1,2", "M 1 2");
    VALID("  M1,2", "M 1 2");
    VALID("\tM1,2", "M 1 2");
    VALID("\nM1,2", "M 1 2");
    VALID("\rM1,2", "M 1 2");
    MALFORMED("\vM1,2", "");
    MALFORMED("xM1,2", "");
    VALID("M1,2 ", "M 1 2");
    VALID("M1,2\t", "M 1 2");
    VALID("M1,2\n", "M 1 2");
    VALID("M1,2\r", "M 1 2");
    MALFORMED("M1,2\v", "M 1 2");
    MALFORMED("M1,2x", "M 1 2");
    MALFORMED("M1,2 L40,0#90", "M 1 2 L 40 0");

    VALID("", "");
    VALID(" ", "");
    MALFORMED("x", "");
    MALFORMED("L1,2", "");
    VALID("M.1 .2 L.3 .4 .5 .6", "M 0.1 0.2 L 0.3 0.4 L 0.5 0.6");

    MALFORMED("M", "");
    MALFORMED("M\0", "");

    MALFORMED("M1,1Z0", "M 1 1 Z");
}

#undef MALFORMED
#undef VALID

}

} // namespace blink
