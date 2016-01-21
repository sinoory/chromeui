// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// This file has been auto-generated by code_generator_v8.py. DO NOT MODIFY!

#include "config.h"
#include "V8DataView.h"

#include "bindings/core/v8/ExceptionState.h"
#include "bindings/core/v8/V8ArrayBuffer.h"
#include "bindings/core/v8/V8DOMConfiguration.h"
#include "bindings/core/v8/V8ObjectConstructor.h"
#include "bindings/core/v8/V8SharedArrayBuffer.h"
#include "core/dom/ContextFeatures.h"
#include "core/dom/Document.h"
#include "platform/RuntimeEnabledFeatures.h"
#include "platform/TraceEvent.h"
#include "wtf/GetPtr.h"
#include "wtf/RefPtr.h"

namespace blink {

// Suppress warning: global constructors, because struct WrapperTypeInfo is trivial
// and does not depend on another global objects.
#if defined(COMPONENT_BUILD) && defined(WIN32) && COMPILER(CLANG)
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wglobal-constructors"
#endif
const WrapperTypeInfo V8DataView::wrapperTypeInfo = { gin::kEmbedderBlink, 0, V8DataView::refObject, V8DataView::derefObject, V8DataView::trace, 0, 0, V8DataView::preparePrototypeAndInterfaceObject, V8DataView::installConditionallyEnabledProperties, "DataView", &V8ArrayBufferView::wrapperTypeInfo, WrapperTypeInfo::WrapperTypeObjectPrototype, WrapperTypeInfo::ObjectClassId, WrapperTypeInfo::NotInheritFromEventTarget, WrapperTypeInfo::Independent, WrapperTypeInfo::RefCountedObject };
#if defined(COMPONENT_BUILD) && defined(WIN32) && COMPILER(CLANG)
#pragma clang diagnostic pop
#endif

// This static member must be declared by DEFINE_WRAPPERTYPEINFO in TestDataView.h.
// For details, see the comment of DEFINE_WRAPPERTYPEINFO in
// bindings/core/v8/ScriptWrappable.h.
const WrapperTypeInfo& TestDataView::s_wrapperTypeInfo = V8DataView::wrapperTypeInfo;

bool V8DataView::hasInstance(v8::Local<v8::Value> v8Value, v8::Isolate* isolate)
{
    return v8Value->IsDataView();
}

TestDataView* V8DataView::toImpl(v8::Local<v8::Object> object)
{
    ASSERT(object->IsDataView());
    ScriptWrappable* scriptWrappable = toScriptWrappable(object);
    if (scriptWrappable)
        return scriptWrappable->toImpl<TestDataView>();

    v8::Local<v8::DataView> v8View = object.As<v8::DataView>();
    v8::Local<v8::Object> arrayBuffer = v8View->Buffer();
    RefPtr<TestDataView> typedArray;
    if (arrayBuffer->IsArrayBuffer()) {
        typedArray = TestDataView::create(V8ArrayBuffer::toImpl(arrayBuffer), v8View->ByteOffset(), v8View->ByteLength());
    } else if (arrayBuffer->IsSharedArrayBuffer()) {
        typedArray = TestDataView::create(V8SharedArrayBuffer::toImpl(arrayBuffer), v8View->ByteOffset(), v8View->ByteLength());
    } else {
        ASSERT_NOT_REACHED();
    }
    v8::Local<v8::Object> associatedWrapper = typedArray->associateWithWrapper(v8::Isolate::GetCurrent(), typedArray->wrapperTypeInfo(), object);
    ASSERT_UNUSED(associatedWrapper, associatedWrapper == object);

    return typedArray->toImpl<TestDataView>();
}

TestDataView* V8DataView::toImplWithTypeCheck(v8::Isolate* isolate, v8::Local<v8::Value> value)
{
    return hasInstance(value, isolate) ? toImpl(v8::Local<v8::Object>::Cast(value)) : 0;
}

void V8DataView::refObject(ScriptWrappable* scriptWrappable)
{
    scriptWrappable->toImpl<TestDataView>()->ref();
}

void V8DataView::derefObject(ScriptWrappable* scriptWrappable)
{
    scriptWrappable->toImpl<TestDataView>()->deref();
}

} // namespace blink
