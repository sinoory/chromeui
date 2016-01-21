/*
 * Copyright (C) 2003, 2009, 2012 Apple Inc. All rights reserved.
 * Copyright (C) 2013 Intel Corporation. All rights reserved.
 *
 * Portions are Copyright (C) 1998 Netscape Communications Corporation.
 *
 * Other contributors:
 *   Robert O'Callahan <roc+@cs.cmu.edu>
 *   David Baron <dbaron@fas.harvard.edu>
 *   Christian Biesinger <cbiesinger@web.de>
 *   Randall Jesup <rjesup@wgate.com>
 *   Roland Mainz <roland.mainz@informatik.med.uni-giessen.de>
 *   Josh Soref <timeless@mac.com>
 *   Boris Zbarsky <bzbarsky@mit.edu>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 *
 * Alternatively, the contents of this file may be used under the terms
 * of either the Mozilla Public License Version 1.1, found at
 * http://www.mozilla.org/MPL/ (the "MPL") or the GNU General Public
 * License Version 2.0, found at http://www.fsf.org/copyleft/gpl.html
 * (the "GPL"), in which case the provisions of the MPL or the GPL are
 * applicable instead of those above.  If you wish to allow use of your
 * version of this file only under the terms of one of those two
 * licenses (the MPL or the GPL) and not to allow others to use your
 * version of this file under the LGPL, indicate your decision by
 * deletingthe provisions above and replace them with the notice and
 * other provisions required by the MPL or the GPL, as the case may be.
 * If you do not delete the provisions above, a recipient may use your
 * version of this file under any of the LGPL, the MPL or the GPL.
 */

#ifndef PaintLayer_h
#define PaintLayer_h

#include "core/CoreExport.h"
#include "core/layout/LayoutBox.h"
#include "core/paint/PaintLayerClipper.h"
#include "core/paint/PaintLayerFilterInfo.h"
#include "core/paint/PaintLayerFragment.h"
#include "core/paint/PaintLayerReflectionInfo.h"
#include "core/paint/PaintLayerScrollableArea.h"
#include "core/paint/PaintLayerStackingNode.h"
#include "core/paint/PaintLayerStackingNodeIterator.h"
#include "platform/graphics/CompositingReasons.h"
#include "public/platform/WebBlendMode.h"
#include "wtf/Allocator.h"
#include "wtf/OwnPtr.h"

namespace blink {

class FilterEffectBuilder;
class FilterOperations;
class HitTestRequest;
class HitTestResult;
class HitTestingTransformState;
class ObjectPaintProperties;
class PaintLayerCompositor;
class CompositedLayerMapping;
class ComputedStyle;
class TransformationMatrix;

enum IncludeSelfOrNot { IncludeSelf, ExcludeSelf };

enum CompositingQueryMode {
    CompositingQueriesAreAllowed,
    CompositingQueriesAreOnlyAllowedInCertainDocumentLifecyclePhases
};

// FIXME: remove this once the compositing query ASSERTS are no longer hit.
class CORE_EXPORT DisableCompositingQueryAsserts {
    STACK_ALLOCATED();
    WTF_MAKE_NONCOPYABLE(DisableCompositingQueryAsserts);
public:
    DisableCompositingQueryAsserts();
private:
    TemporaryChange<CompositingQueryMode> m_disabler;
};

// PaintLayer is an old object that handles lots of unrelated operations.
//
// We want it to die at some point and be replaced by more focused objects,
// which would remove (or at least compartimentalize) a lot of complexity.
// See the STATUS OF PAINTLAYER section below.
//
// The class is central to painting and hit-testing. That's because it handles
// a lot of tasks (we included ones done by associated satellite objects for
// historical reasons):
// - Complex painting operations (opacity, clipping, filters, reflections, ...).
// - hardware acceleration (through PaintLayerCompositor).
// - scrolling (through PaintLayerScrollableArea).
// - some performance optimizations.
//
// The compositing code is also based on PaintLayer. The entry to it is the
// PaintLayerCompositor, which fills |m_compositedLayerMapping| for hardware
// accelerated layers.
//
// TODO(jchaffraix): Expand the documentation about hardware acceleration.
//
//
// ***** SELF-PAINTING LAYER *****
// One important concept about PaintLayer is "self-painting"
// (m_isSelfPaintingLayer).
// PaintLayer started as the implementation of a stacking context. This meant
// that we had to use PaintLayer's painting order (the code is now in
// PaintLayerPainter and PaintLayerStackingNode) instead of the LayoutObject's
// children order. Over the years, as more operations were handled by
// PaintLayer, some LayoutObjects that were not stacking context needed to have
// a PaintLayer for bookkeeping reasons. One such example is the overflow hidden
// case that wanted hardware acceleration and thus had to allocate a PaintLayer
// to get it. However overflow hidden is something LayoutObject can paint
// without a PaintLayer, which includes a lot of painting overhead. Thus the
// self-painting flag was introduced. The flag is a band-aid solution done for
// performance reason only. It just brush over the underlying problem, which is
// that its design doesn't match the system's requirements anymore.
//
//
// ***** STATUS OF PAINTLAYER *****
// We would like to remove this class in the future. The reasons for the removal
// are:
// - it has been a dumping ground for features for too long.
// - it is the wrong level of abstraction, bearing no correspondence to any CSS
//   concept.
//
// Its features need to be migrated to helper objects. This was started with the
// introduction of satellite objects: PaintLayer*. Those helper objects then
// need to be moved to the appropriate LayoutObject class, probably to a rare
// data field to avoid growing all the LayoutObjects.
//
// A good example of this is PaintLayerScrollableArea, which can only happen
// be instanciated for LayoutBoxes. With the current design, it's hard to know
// that by reading the code.
class CORE_EXPORT PaintLayer {
    WTF_MAKE_NONCOPYABLE(PaintLayer);
public:
    PaintLayer(LayoutBoxModelObject*, PaintLayerType);
    ~PaintLayer();

    String debugName() const;

    LayoutBoxModelObject* layoutObject() const { return m_layoutObject; }
    LayoutBox* layoutBox() const { return m_layoutObject && m_layoutObject->isBox() ? toLayoutBox(m_layoutObject) : 0; }
    PaintLayer* parent() const { return m_parent; }
    PaintLayer* previousSibling() const { return m_previous; }
    PaintLayer* nextSibling() const { return m_next; }
    PaintLayer* firstChild() const { return m_first; }
    PaintLayer* lastChild() const { return m_last; }

    // TODO(wangxianzhu): Find a better name for it. 'paintContainer' might be good but
    // we can't use it for now because it conflicts with PaintInfo::paintContainer.
    PaintLayer* compositingContainer() const;

    void addChild(PaintLayer* newChild, PaintLayer* beforeChild = 0);
    PaintLayer* removeChild(PaintLayer*);

    void removeOnlyThisLayer();
    void insertOnlyThisLayer();

    void styleChanged(StyleDifference, const ComputedStyle* oldStyle);

    // FIXME: Many people call this function while it has out-of-date information.
    bool isSelfPaintingLayer() const { return m_isSelfPaintingLayer; }

    void setLayerType(PaintLayerType layerType) { m_layerType = layerType; }

    bool isTransparent() const { return layoutObject()->isTransparent() || layoutObject()->style()->hasBlendMode() || layoutObject()->hasMask(); }

    bool isReflection() const { return layoutObject()->isReplica(); }
    PaintLayerReflectionInfo* reflectionInfo() { return m_reflectionInfo.get(); }
    const PaintLayerReflectionInfo* reflectionInfo() const { return m_reflectionInfo.get(); }

    const PaintLayer* root() const
    {
        const PaintLayer* curr = this;
        while (curr->parent())
            curr = curr->parent();
        return curr;
    }

    const LayoutPoint& location() const { ASSERT(!m_needsPositionUpdate); return m_location; }
    // FIXME: size() should ASSERT(!m_needsPositionUpdate) as well, but that fails in some tests,
    // for example, fast/repaint/clipped-relative.html.
    const IntSize& size() const { return m_size; }
    void setSizeHackForLayoutTreeAsText(const IntSize& size) { m_size = size; }

    LayoutRect rect() const { return LayoutRect(location(), LayoutSize(size())); }

    bool isRootLayer() const { return m_isRootLayer; }

    PaintLayerCompositor* compositor() const;

    // Notification from the layoutObject that its content changed (e.g. current frame of image changed).
    // Allows updates of layer content without invalidating paint.
    void contentChanged(ContentChangeType);

    void updateLayerPositionsAfterLayout();
    void updateLayerPositionsAfterOverflowScroll(const DoubleSize& scrollDelta);

    PaintLayer* enclosingPaginationLayer() const { return m_enclosingPaginationLayer; }

    void updateTransformationMatrix();
    PaintLayer* renderingContextRoot();

    const LayoutSize& offsetForInFlowPosition() const { return m_offsetForInFlowPosition; }

    void addBlockSelectionGapsBounds(const LayoutRect&);
    void clearBlockSelectionGapsBounds();
    void invalidatePaintForBlockSelectionGaps();
    IntRect blockSelectionGapsBounds() const;
    bool hasBlockSelectionGapBounds() const;

    PaintLayerStackingNode* stackingNode() { return m_stackingNode.get(); }
    const PaintLayerStackingNode* stackingNode() const { return m_stackingNode.get(); }

    bool subtreeIsInvisible() const { return !hasVisibleContent() && !hasVisibleDescendant(); }

    // FIXME: hasVisibleContent() should call updateDescendantDependentFlags() if m_visibleContentStatusDirty.
    bool hasVisibleContent() const { ASSERT(!m_visibleContentStatusDirty); return m_hasVisibleContent; }

    // FIXME: hasVisibleDescendant() should call updateDescendantDependentFlags() if m_visibleDescendantStatusDirty.
    bool hasVisibleDescendant() const { ASSERT(!m_visibleDescendantStatusDirty); return m_hasVisibleDescendant; }

    void dirtyVisibleContentStatus();
    void potentiallyDirtyVisibleContentStatus(EVisibility);

    bool hasBoxDecorationsOrBackground() const;
    bool hasVisibleBoxDecorations() const;
    // True if this layer container layoutObjects that paint.
    bool hasNonEmptyChildLayoutObjects() const;

    // Will ensure that hasNonCompositiedChild are up to date.
    void updateScrollingStateAfterCompositingChange();
    bool hasVisibleNonLayerContent() const { return m_hasVisibleNonLayerContent; }
    bool hasNonCompositedChild() const { ASSERT(isAllowedToQueryCompositingState()); return m_hasNonCompositedChild; }

    // Gets the ancestor layer that serves as the containing block of this layer. It is assumed
    // that this layer is established by an out-of-flow positioned layout object (i.e. either
    // absolutely or fixed positioned).
    // If |ancestor| is specified, |*skippedAncestor| will be set to true if |ancestor| is found in
    // the ancestry chain between this layer and the containing block layer; if not found, it will
    // be set to false. Either both |ancestor| and |skippedAncestor| should be nullptr, or none of
    // them should.
    PaintLayer* enclosingPositionedAncestor(const PaintLayer* ancestor = nullptr, bool* skippedAncestor = nullptr) const;

    bool isPaintInvalidationContainer() const;

    // Do *not* call this method unless you know what you are dooing. You probably want to call enclosingCompositingLayerForPaintInvalidation() instead.
    // If includeSelf is true, may return this.
    PaintLayer* enclosingLayerWithCompositedLayerMapping(IncludeSelfOrNot) const;

    // Returns the enclosing layer root into which this layer paints, inclusive of this one. Note that the enclosing layer may or may not have its own
    // GraphicsLayer backing, but is nevertheless the root for a call to the Layer::paint*() methods.
    PaintLayer* enclosingLayerForPaintInvalidation() const;

    PaintLayer* enclosingLayerForPaintInvalidationCrossingFrameBoundaries() const;

    bool hasAncestorWithFilterOutsets() const;

    bool canUseConvertToLayerCoords() const
    {
        // These LayoutObjects have an impact on their layers without the layoutObjects knowing about it.
        return !layoutObject()->hasTransformRelatedProperty() && !layoutObject()->isSVGRoot();
    }

    void convertToLayerCoords(const PaintLayer* ancestorLayer, LayoutPoint&) const;
    void convertToLayerCoords(const PaintLayer* ancestorLayer, LayoutRect&) const;

    // Does the same as convertToLayerCoords() when not in multicol. For multicol, however,
    // convertToLayerCoords() calculates the offset in flow-thread coordinates (what the layout
    // engine uses internally), while this method calculates the visual coordinates; i.e. it figures
    // out which column the layer starts in and adds in the offset. See
    // http://www.chromium.org/developers/design-documents/multi-column-layout for more info.
    LayoutPoint visualOffsetFromAncestor(const PaintLayer* ancestorLayer) const;

    // The hitTest() method looks for mouse events by walking layers that intersect the point from front to back.
    bool hitTest(HitTestResult&);

    bool intersectsDamageRect(const LayoutRect& layerBounds, const LayoutRect& damageRect, const LayoutPoint& offsetFromRoot) const;

    // Bounding box relative to some ancestor layer. Pass offsetFromRoot if known.
    LayoutRect physicalBoundingBox(const LayoutPoint& offsetFromRoot) const;
    LayoutRect physicalBoundingBox(const PaintLayer* ancestorLayer) const;
    LayoutRect physicalBoundingBoxIncludingReflectionAndStackingChildren(const LayoutPoint& offsetFromRoot) const;
    LayoutRect fragmentsBoundingBox(const PaintLayer* ancestorLayer) const;

    LayoutRect boundingBoxForCompositingOverlapTest() const;

    // If true, this layer's children are included in its bounds for overlap testing.
    // We can't rely on the children's positions if this layer has a filter that could have moved the children's pixels around.
    bool overlapBoundsIncludeChildren() const { return hasFilter() && layoutObject()->style()->filter().hasFilterThatMovesPixels(); }

    // MaybeIncludeTransformForAncestorLayer means that a transform on |ancestorLayer| may be applied to the bounding box,
    // in particular if paintsWithTransform() is true.
    enum CalculateBoundsOptions {
        MaybeIncludeTransformForAncestorLayer,
        NeverIncludeTransformForAncestorLayer,
    };
    LayoutRect boundingBoxForCompositing(const PaintLayer* ancestorLayer = 0, CalculateBoundsOptions = MaybeIncludeTransformForAncestorLayer) const;

    LayoutUnit staticInlinePosition() const { return m_staticInlinePosition; }
    LayoutUnit staticBlockPosition() const { return m_staticBlockPosition; }

    void setStaticInlinePosition(LayoutUnit position) { m_staticInlinePosition = position; }
    void setStaticBlockPosition(LayoutUnit position) { m_staticBlockPosition = position; }

    LayoutSize subpixelAccumulation() const;
    void setSubpixelAccumulation(const LayoutSize&);

    bool hasTransformRelatedProperty() const { return layoutObject()->hasTransformRelatedProperty(); }
    // Note that this transform has the transform-origin baked in.
    TransformationMatrix* transform() const { return m_transform.get(); }
    void setTransform(PassOwnPtr<TransformationMatrix> transform) { m_transform = transform; }
    void clearTransform() { m_transform.clear(); }

    // currentTransform computes a transform which takes accelerated animations into account. The
    // resulting transform has transform-origin baked in. If the layer does not have a transform,
    // returns the identity matrix.
    TransformationMatrix currentTransform() const;
    TransformationMatrix renderableTransform(GlobalPaintFlags) const;

    // Get the perspective transform, which is applied to transformed sublayers.
    // Returns true if the layer has a -webkit-perspective.
    // Note that this transform does not have the perspective-origin baked in.
    TransformationMatrix perspectiveTransform() const;
    FloatPoint perspectiveOrigin() const;
    bool preserves3D() const { return layoutObject()->style()->transformStyle3D() == TransformStyle3DPreserve3D; }
    bool has3DTransform() const { return m_transform && !m_transform->isAffine(); }

    // FIXME: reflections should force transform-style to be flat in the style: https://bugs.webkit.org/show_bug.cgi?id=106959
    bool shouldPreserve3D() const { return !layoutObject()->hasReflection() && layoutObject()->style()->transformStyle3D() == TransformStyle3DPreserve3D; }

    void filterNeedsPaintInvalidation();
    bool hasFilter() const { return layoutObject()->hasFilter(); }

    void* operator new(size_t);
    // Only safe to call from LayoutBoxModelObject::destroyLayer()
    void operator delete(void*);

    CompositingState compositingState() const;

    // This returns true if our document is in a phase of its lifestyle during which
    // compositing state may legally be read.
    bool isAllowedToQueryCompositingState() const;

    // Don't null check this.
    // FIXME: Rename.
    CompositedLayerMapping* compositedLayerMapping() const;
    GraphicsLayer* graphicsLayerBacking() const;
    GraphicsLayer* graphicsLayerBackingForScrolling() const;
    // NOTE: If you are using hasCompositedLayerMapping to determine the state of compositing for this layer,
    // (and not just to do bookkeeping related to the mapping like, say, allocating or deallocating a mapping),
    // then you may have incorrect logic. Use compositingState() instead.
    // FIXME: This is identical to null checking compositedLayerMapping(), why not just call that?
    bool hasCompositedLayerMapping() const { return m_compositedLayerMapping.get(); }
    void ensureCompositedLayerMapping();
    void clearCompositedLayerMapping(bool layerBeingDestroyed = false);
    CompositedLayerMapping* groupedMapping() const { return m_groupedMapping; }
    enum SetGroupMappingOptions {
        InvalidateLayerAndRemoveFromMapping,
        DoNotInvalidateLayerAndRemoveFromMapping
    };
    void setGroupedMapping(CompositedLayerMapping*, SetGroupMappingOptions);

    bool hasCompositedMask() const;
    bool hasCompositedClippingMask() const;
    bool needsCompositedScrolling() const { return m_scrollableArea && m_scrollableArea->needsCompositedScrolling(); }

    // Computes the position of the given layout object in the space of |paintInvalidationContainer|.
    // FIXME: invert the logic to have paint invalidation containers take care of painting objects into them, rather than the reverse.
    // This will allow us to clean up this static method messiness.
    static LayoutPoint positionFromPaintInvalidationBacking(const LayoutObject*, const LayoutBoxModelObject* paintInvalidationContainer, const PaintInvalidationState* = 0);

    static void mapPointToPaintBackingCoordinates(const LayoutBoxModelObject* paintInvalidationContainer, FloatPoint&);
    static void mapRectToPaintBackingCoordinates(const LayoutBoxModelObject* paintInvalidationContainer, LayoutRect&);

    // Adjusts the given rect (in the coordinate space of the LayoutObject) to the coordinate space of |paintInvalidationContainer|'s GraphicsLayer backing.
    static void mapRectToPaintInvalidationBacking(const LayoutObject*, const LayoutBoxModelObject* paintInvalidationContainer, LayoutRect&, const PaintInvalidationState* = 0);

    // Computes the bounding paint invalidation rect for |layoutObject|, in the coordinate space of |paintInvalidationContainer|'s GraphicsLayer backing.
    static LayoutRect computePaintInvalidationRect(const LayoutObject*, const PaintLayer* paintInvalidationContainer, const PaintInvalidationState* = 0);

    bool paintsWithTransparency(GlobalPaintFlags globalPaintFlags) const
    {
        return isTransparent() && ((globalPaintFlags & GlobalPaintFlattenCompositingLayers) || compositingState() != PaintsIntoOwnBacking);
    }

    bool paintsWithTransform(GlobalPaintFlags) const;

    // Returns true if background phase is painted opaque in the given rect.
    // The query rect is given in local coordinates.
    bool backgroundIsKnownToBeOpaqueInRect(const LayoutRect&) const;

    bool containsDirtyOverlayScrollbars() const { return m_containsDirtyOverlayScrollbars; }
    void setContainsDirtyOverlayScrollbars(bool dirtyScrollbars) { m_containsDirtyOverlayScrollbars = dirtyScrollbars; }

    FilterOperations computeFilterOperations(const ComputedStyle&);
    FilterOperations computeBackdropFilterOperations(const ComputedStyle&);
    bool paintsWithFilters() const;
    bool paintsWithBackdropFilters() const;
    FilterEffectBuilder* filterEffectBuilder() const
    {
        PaintLayerFilterInfo* filterInfo = this->filterInfo();
        return filterInfo ? filterInfo->builder() : 0;
    }

    PaintLayerFilterInfo* filterInfo() const { return hasFilterInfo() ? PaintLayerFilterInfo::filterInfoForLayer(this) : 0; }
    PaintLayerFilterInfo* ensureFilterInfo() { return PaintLayerFilterInfo::createFilterInfoForLayerIfNeeded(this); }
    void removeFilterInfoIfNeeded()
    {
        if (hasFilterInfo())
            PaintLayerFilterInfo::removeFilterInfoForLayer(this);
    }

    bool hasFilterInfo() const { return m_hasFilterInfo; }
    void setHasFilterInfo(bool hasFilterInfo) { m_hasFilterInfo = hasFilterInfo; }

    void updateFilters(const ComputedStyle* oldStyle, const ComputedStyle& newStyle);

    Node* enclosingElement() const;

    bool isInTopLayer() const;

    bool scrollsWithViewport() const;
    bool scrollsWithRespectTo(const PaintLayer*) const;

    void addLayerHitTestRects(LayerHitTestRects&) const;

    // Compute rects only for this layer
    void computeSelfHitTestRects(LayerHitTestRects&) const;

    // FIXME: This should probably return a ScrollableArea but a lot of internal methods are mistakenly exposed.
    PaintLayerScrollableArea* scrollableArea() const { return m_scrollableArea.get(); }
    PaintLayerClipper& clipper() { return m_clipper; }
    const PaintLayerClipper& clipper() const { return m_clipper; }

    inline bool isPositionedContainer() const
    {
        // FIXME: This is not in sync with containingBlock.
        // LayoutObject::canContainFixedPositionObjects() should probably be used
        // instead.
        LayoutBoxModelObject* layerlayoutObject = layoutObject();
        return isRootLayer() || layerlayoutObject->isPositioned() || hasTransformRelatedProperty();
    }

    bool scrollsOverflow() const;

    CompositingReasons potentialCompositingReasonsFromStyle() const { return m_potentialCompositingReasonsFromStyle; }
    void setPotentialCompositingReasonsFromStyle(CompositingReasons reasons) { ASSERT(reasons == (reasons & CompositingReasonComboAllStyleDeterminedReasons)); m_potentialCompositingReasonsFromStyle = reasons; }

    bool hasStyleDeterminedDirectCompositingReasons() const { return m_potentialCompositingReasonsFromStyle & CompositingReasonComboAllDirectStyleDeterminedReasons; }

    class AncestorDependentCompositingInputs {
        DISALLOW_ALLOCATION();
    public:
        AncestorDependentCompositingInputs()
            : opacityAncestor(0)
            , transformAncestor(0)
            , filterAncestor(0)
            , clippingContainer(0)
            , ancestorScrollingLayer(0)
            , nearestFixedPositionLayer(0)
            , scrollParent(0)
            , clipParent(0)
            , hasAncestorWithClipPath(false)
        { }

        IntRect clippedAbsoluteBoundingBox;
        const PaintLayer* opacityAncestor;
        const PaintLayer* transformAncestor;
        const PaintLayer* filterAncestor;
        const LayoutObject* clippingContainer;
        const PaintLayer* ancestorScrollingLayer;
        const PaintLayer* nearestFixedPositionLayer;

        // A scroll parent is a compositor concept. It's only needed in blink
        // because we need to use it as a promotion trigger. A layer has a
        // scroll parent if neither its compositor scrolling ancestor, nor any
        // other layer scrolled by this ancestor, is a stacking ancestor of this
        // layer. Layers with scroll parents must be scrolled with the main
        // scrolling layer by the compositor.
        const PaintLayer* scrollParent;

        // A clip parent is another compositor concept that has leaked into
        // blink so that it may be used as a promotion trigger. Layers with clip
        // parents escape the clip of a stacking tree ancestor. The compositor
        // needs to know about clip parents in order to circumvent its normal
        // clipping logic.
        const PaintLayer* clipParent;

        unsigned hasAncestorWithClipPath : 1;
    };

    class DescendantDependentCompositingInputs {
        DISALLOW_ALLOCATION();
    public:
        DescendantDependentCompositingInputs()
            : hasDescendantWithClipPath(false)
            , hasNonIsolatedDescendantWithBlendMode(false)
        { }

        unsigned hasDescendantWithClipPath : 1;
        unsigned hasNonIsolatedDescendantWithBlendMode : 1;
    };

    void setNeedsCompositingInputsUpdate();
    bool childNeedsCompositingInputsUpdate() const { return m_childNeedsCompositingInputsUpdate; }
    bool needsCompositingInputsUpdate() const
    {
        // While we're updating the compositing inputs, these values may differ.
        // We should never be asking for this value when that is the case.
        ASSERT(m_needsDescendantDependentCompositingInputsUpdate == m_needsAncestorDependentCompositingInputsUpdate);
        return m_needsDescendantDependentCompositingInputsUpdate;
    }

    void updateAncestorDependentCompositingInputs(const AncestorDependentCompositingInputs&);
    void updateDescendantDependentCompositingInputs(const DescendantDependentCompositingInputs&);
    void didUpdateCompositingInputs();

    const AncestorDependentCompositingInputs& ancestorDependentCompositingInputs() const { ASSERT(!m_needsAncestorDependentCompositingInputsUpdate); return m_ancestorDependentCompositingInputs; }
    const DescendantDependentCompositingInputs& descendantDependentCompositingInputs() const { ASSERT(!m_needsDescendantDependentCompositingInputsUpdate); return m_descendantDependentCompositingInputs; }

    IntRect clippedAbsoluteBoundingBox() const { return ancestorDependentCompositingInputs().clippedAbsoluteBoundingBox; }
    const PaintLayer* opacityAncestor() const { return ancestorDependentCompositingInputs().opacityAncestor; }
    const PaintLayer* transformAncestor() const { return ancestorDependentCompositingInputs().transformAncestor; }
    const PaintLayer* filterAncestor() const { return ancestorDependentCompositingInputs().filterAncestor; }
    const LayoutObject* clippingContainer() const { return ancestorDependentCompositingInputs().clippingContainer; }
    const PaintLayer* ancestorScrollingLayer() const { return ancestorDependentCompositingInputs().ancestorScrollingLayer; }
    const PaintLayer* nearestFixedPositionLayer() const { return ancestorDependentCompositingInputs().nearestFixedPositionLayer; }
    PaintLayer* scrollParent() const { return const_cast<PaintLayer*>(ancestorDependentCompositingInputs().scrollParent); }
    PaintLayer* clipParent() const { return const_cast<PaintLayer*>(ancestorDependentCompositingInputs().clipParent); }
    bool hasAncestorWithClipPath() const { return ancestorDependentCompositingInputs().hasAncestorWithClipPath; }
    bool hasDescendantWithClipPath() const { return descendantDependentCompositingInputs().hasDescendantWithClipPath; }
    bool hasNonIsolatedDescendantWithBlendMode() const;

    bool lostGroupedMapping() const { ASSERT(isAllowedToQueryCompositingState()); return m_lostGroupedMapping; }
    void setLostGroupedMapping(bool b) { m_lostGroupedMapping = b; }

    CompositingReasons compositingReasons() const { ASSERT(isAllowedToQueryCompositingState()); return m_compositingReasons; }
    void setCompositingReasons(CompositingReasons, CompositingReasons mask = CompositingReasonAll);

    bool hasCompositingDescendant() const { ASSERT(isAllowedToQueryCompositingState()); return m_hasCompositingDescendant; }
    void setHasCompositingDescendant(bool);

    bool shouldIsolateCompositedDescendants() const { ASSERT(isAllowedToQueryCompositingState()); return m_shouldIsolateCompositedDescendants; }
    void setShouldIsolateCompositedDescendants(bool);

    void updateDescendantDependentFlags();

    void updateOrRemoveFilterEffectBuilder();

    void updateSelfPaintingLayer();

    PaintLayer* enclosingTransformedAncestor() const;
    LayoutPoint computeOffsetFromTransformedAncestor() const;

    void didUpdateNeedsCompositedScrolling();

    bool hasSelfPaintingLayerDescendant() const
    {
        if (m_hasSelfPaintingLayerDescendantDirty)
            updateHasSelfPaintingLayerDescendant();
        ASSERT(!m_hasSelfPaintingLayerDescendantDirty);
        return m_hasSelfPaintingLayerDescendant;
    }
    LayoutRect paintingExtent(const PaintLayer* rootLayer, const LayoutSize& subPixelAccumulation, GlobalPaintFlags);
    void appendSingleFragmentIgnoringPagination(PaintLayerFragments&, const PaintLayer* rootLayer, const LayoutRect& dirtyRect, ClipRectsCacheSlot, OverlayScrollbarSizeRelevancy = IgnoreOverlayScrollbarSize, ShouldRespectOverflowClip = RespectOverflowClip, const LayoutPoint* offsetFromRoot = 0, const LayoutSize& subPixelAccumulation = LayoutSize());
    void collectFragments(PaintLayerFragments&, const PaintLayer* rootLayer, const LayoutRect& dirtyRect,
        ClipRectsCacheSlot, OverlayScrollbarSizeRelevancy inOverlayScrollbarSizeRelevancy = IgnoreOverlayScrollbarSize,
        ShouldRespectOverflowClip = RespectOverflowClip, const LayoutPoint* offsetFromRoot = 0,
        const LayoutSize& subPixelAccumulation = LayoutSize(), const LayoutRect* layerBoundingBox = 0);

    LayoutPoint layoutBoxLocation() const { return layoutObject()->isBox() ? toLayoutBox(layoutObject())->location() : LayoutPoint(); }

    enum TransparencyClipBoxBehavior {
        PaintingTransparencyClipBox,
        HitTestingTransparencyClipBox
    };

    enum TransparencyClipBoxMode {
        DescendantsOfTransparencyClipBox,
        RootOfTransparencyClipBox
    };

    static LayoutRect transparencyClipBox(const PaintLayer*, const PaintLayer* rootLayer, TransparencyClipBoxBehavior transparencyBehavior,
        TransparencyClipBoxMode transparencyMode, const LayoutSize& subPixelAccumulation, GlobalPaintFlags = GlobalPaintNormalPhase);

    bool needsRepaint() const { return m_needsRepaint; }
    void setNeedsRepaint();
    void clearNeedsRepaintRecursively();

    IntSize previousScrollOffsetAccumulationForPainting() const { return m_previousScrollOffsetAccumulationForPainting; }
    void setPreviousScrollOffsetAccumulationForPainting(const IntSize& s) { m_previousScrollOffsetAccumulationForPainting = s; }

    // For subsequence display items.
    DisplayItemClient displayItemClient() const { return toDisplayItemClient(this); }

    // Paint properties encode the hierarchical transform, clip, scroll, and
    // effect information used for painting. Properties should only be changed
    // during UpdatePaintProperties, and should only be used during Paint.
    ObjectPaintProperties& mutablePaintProperties();
    const ObjectPaintProperties* paintProperties() const;

private:
    // Bounding box in the coordinates of this layer.
    LayoutRect logicalBoundingBox() const;

    bool hasOverflowControls() const;

    void dirtyAncestorChainHasSelfPaintingLayerDescendantStatus();

    // Returns true if the position changed.
    bool updateLayerPosition();

    void updateLayerPositionRecursive();
    void updateLayerPositionsAfterScrollRecursive(const DoubleSize& scrollDelta, bool paintInvalidationContainerWasScrolled);

    void setNextSibling(PaintLayer* next) { m_next = next; }
    void setPreviousSibling(PaintLayer* prev) { m_previous = prev; }
    void setFirstChild(PaintLayer* first) { m_first = first; }
    void setLastChild(PaintLayer* last) { m_last = last; }

    void updateHasSelfPaintingLayerDescendant() const;
    PaintLayer* hitTestLayer(PaintLayer* rootLayer, PaintLayer* containerLayer, HitTestResult&,
        const LayoutRect& hitTestRect, const HitTestLocation&, bool appliedTransform,
        const HitTestingTransformState* = 0, double* zOffset = 0);
    PaintLayer* hitTestLayerByApplyingTransform(PaintLayer* rootLayer, PaintLayer* containerLayer, HitTestResult&,
        const LayoutRect& hitTestRect, const HitTestLocation&, const HitTestingTransformState* = 0, double* zOffset = 0,
        const LayoutPoint& translationOffset = LayoutPoint());
    PaintLayer* hitTestChildren(ChildrenIteration, PaintLayer* rootLayer, HitTestResult&,
        const LayoutRect& hitTestRect, const HitTestLocation&,
        const HitTestingTransformState*, double* zOffsetForDescendants, double* zOffset,
        const HitTestingTransformState* unflattenedTransformState, bool depthSortDescendants);

    PassRefPtr<HitTestingTransformState> createLocalTransformState(PaintLayer* rootLayer, PaintLayer* containerLayer,
        const LayoutRect& hitTestRect, const HitTestLocation&,
        const HitTestingTransformState* containerTransformState,
        const LayoutPoint& translationOffset = LayoutPoint()) const;

    bool hitTestContents(HitTestResult&, const LayoutRect& layerBounds, const HitTestLocation&, HitTestFilter) const;
    bool hitTestContentsForFragments(const PaintLayerFragments&, HitTestResult&, const HitTestLocation&, HitTestFilter, bool& insideClipRect) const;
    PaintLayer* hitTestTransformedLayerInFragments(PaintLayer* rootLayer, PaintLayer* containerLayer, HitTestResult&,
        const LayoutRect& hitTestRect, const HitTestLocation&, const HitTestingTransformState*, double* zOffset, ClipRectsCacheSlot);

    bool childBackgroundIsKnownToBeOpaqueInRect(const LayoutRect&) const;

    bool shouldBeSelfPaintingLayer() const;

    // FIXME: We should only create the stacking node if needed.
    bool requiresStackingNode() const { return true; }
    void updateStackingNode();

    void updateReflectionInfo(const ComputedStyle*);

    // FIXME: We could lazily allocate our ScrollableArea based on style properties ('overflow', ...)
    // but for now, we are always allocating it for LayoutBox as it's safer. crbug.com/467721.
    bool requiresScrollableArea() const { return layoutBox(); }
    void updateScrollableArea();

    void dirtyAncestorChainVisibleDescendantStatus();

    bool attemptDirectCompositingUpdate(StyleDifference, const ComputedStyle* oldStyle);
    void updateTransform(const ComputedStyle* oldStyle, const ComputedStyle& newStyle);

    void dirty3DTransformedDescendantStatus();
    // Both updates the status, and returns true if descendants of this have 3d.
    bool update3DTransformedDescendantStatus();

    void updateOrRemoveFilterClients();

    void updatePaginationRecursive(bool needsPaginationUpdate = false);
    void clearPaginationRecursive();

    void blockSelectionGapsBoundsChanged();

    void markAncestorChainForNeedsRepaint();

    PaintLayerType m_layerType;

    // Self-painting layer is an optimization where we avoid the heavy Layer painting
    // machinery for a Layer allocated only to handle the overflow clip case.
    // FIXME(crbug.com/332791): Self-painting layer should be merged into the overflow-only concept.
    unsigned m_isSelfPaintingLayer : 1;

    // If have no self-painting descendants, we don't have to walk our children during painting. This can lead to
    // significant savings, especially if the tree has lots of non-self-painting layers grouped together (e.g. table cells).
    mutable unsigned m_hasSelfPaintingLayerDescendant : 1;
    mutable unsigned m_hasSelfPaintingLayerDescendantDirty : 1;

    const unsigned m_isRootLayer : 1;

    unsigned m_visibleContentStatusDirty : 1;
    unsigned m_hasVisibleContent : 1;
    unsigned m_visibleDescendantStatusDirty : 1;
    unsigned m_hasVisibleDescendant : 1;

    unsigned m_hasVisibleNonLayerContent : 1;

#if ENABLE(ASSERT)
    unsigned m_needsPositionUpdate : 1;
#endif

    unsigned m_3DTransformedDescendantStatusDirty : 1;
    // Set on a stacking context layer that has 3D descendants anywhere
    // in a preserves3D hierarchy. Hint to do 3D-aware hit testing.
    unsigned m_has3DTransformedDescendant : 1;

    unsigned m_containsDirtyOverlayScrollbars : 1;

    unsigned m_hasFilterInfo : 1;
    unsigned m_needsAncestorDependentCompositingInputsUpdate : 1;
    unsigned m_needsDescendantDependentCompositingInputsUpdate : 1;
    unsigned m_childNeedsCompositingInputsUpdate : 1;

    // Used only while determining what layers should be composited. Applies to the tree of z-order lists.
    unsigned m_hasCompositingDescendant : 1;

    // Applies to the real layout layer tree (i.e., the tree determined by the layer's parent and children and
    // as opposed to the tree formed by the z-order and normal flow lists).
    unsigned m_hasNonCompositedChild : 1;

    // Should be for stacking contexts having unisolated blending descendants.
    unsigned m_shouldIsolateCompositedDescendants : 1;

    // True if this layout layer just lost its grouped mapping due to the CompositedLayerMapping being destroyed,
    // and we don't yet know to what graphics layer this Layer will be assigned.
    unsigned m_lostGroupedMapping : 1;

    unsigned m_needsRepaint : 1;

    LayoutBoxModelObject* m_layoutObject;

    PaintLayer* m_parent;
    PaintLayer* m_previous;
    PaintLayer* m_next;
    PaintLayer* m_first;
    PaintLayer* m_last;

    // Our current relative position offset.
    LayoutSize m_offsetForInFlowPosition;

    // Our (x,y) coordinates are in our parent layer's coordinate space.
    LayoutPoint m_location;

    // The layer's size.
    //
    // If the associated LayoutBoxModelObject is a LayoutBox, it's its border
    // box. Otherwise, this is the LayoutInline's lines' bounding box.
    IntSize m_size;

    // Cached normal flow values for absolute positioned elements with static left/top values.
    LayoutUnit m_staticInlinePosition;
    LayoutUnit m_staticBlockPosition;

    OwnPtr<TransformationMatrix> m_transform;

    // Pointer to the enclosing Layer that caused us to be paginated. It is 0 if we are not paginated.
    //
    // See LayoutMultiColumnFlowThread and
    // https://sites.google.com/a/chromium.org/dev/developers/design-documents/multi-column-layout
    // for more information about the multicol implementation. It's important to understand the
    // difference between flow thread coordinates and visual coordinates when working with multicol
    // in Layer, since Layer is one of the few places where we have to worry about the
    // visual ones. Internally we try to use flow-thread coordinates whenever possible.
    PaintLayer* m_enclosingPaginationLayer;

    // These compositing reasons are updated whenever style changes, not while updating compositing layers.
    // They should not be used to infer the compositing state of this layer.
    CompositingReasons m_potentialCompositingReasonsFromStyle;

    // Once computed, indicates all that a layer needs to become composited using the CompositingReasons enum bitfield.
    CompositingReasons m_compositingReasons;

    DescendantDependentCompositingInputs m_descendantDependentCompositingInputs;
    AncestorDependentCompositingInputs m_ancestorDependentCompositingInputs;

    IntRect m_blockSelectionGapsBounds;

    OwnPtr<CompositedLayerMapping> m_compositedLayerMapping;
    OwnPtrWillBePersistent<PaintLayerScrollableArea> m_scrollableArea;

    CompositedLayerMapping* m_groupedMapping;

    PaintLayerClipper m_clipper; // FIXME: Lazily allocate?
    OwnPtr<PaintLayerStackingNode> m_stackingNode;
    OwnPtr<PaintLayerReflectionInfo> m_reflectionInfo;

    // Lazily-allocated paint properties for m_layoutObject, stored here to
    // reduce memory usage as PaintLayers are created for all paint property
    // layout objects.
    OwnPtr<ObjectPaintProperties> m_paintProperties;

    LayoutSize m_subpixelAccumulation; // The accumulated subpixel offset of a composited layer's composited bounds compared to absolute coordinates.

    IntSize m_previousScrollOffsetAccumulationForPainting;
};

} // namespace blink

#ifndef NDEBUG
// Outside the WebCore namespace for ease of invocation from gdb.
void showLayerTree(const blink::PaintLayer*);
void showLayerTree(const blink::LayoutObject*);
#endif

#endif // Layer_h
