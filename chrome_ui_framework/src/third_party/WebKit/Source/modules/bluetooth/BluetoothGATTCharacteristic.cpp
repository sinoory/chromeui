// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "config.h"
#include "modules/bluetooth/BluetoothGATTCharacteristic.h"

#include "bindings/core/v8/CallbackPromiseAdapter.h"
#include "bindings/core/v8/ScriptPromise.h"
#include "bindings/core/v8/ScriptPromiseResolver.h"
#include "core/dom/DOMException.h"
#include "core/dom/ExceptionCode.h"
#include "modules/bluetooth/BluetoothError.h"
#include "modules/bluetooth/BluetoothSupplement.h"
#include "modules/bluetooth/ConvertWebVectorToArrayBuffer.h"
#include "public/platform/modules/bluetooth/WebBluetooth.h"

namespace blink {

BluetoothGATTCharacteristic::BluetoothGATTCharacteristic(ExecutionContext* context, PassOwnPtr<WebBluetoothGATTCharacteristicInit> webCharacteristic)
    : ActiveDOMObject(context)
    , m_webCharacteristic(webCharacteristic)
    , m_stopped(false)
{
    // See example in Source/platform/heap/ThreadState.h
    ThreadState::current()->registerPreFinalizer(this);
}

BluetoothGATTCharacteristic* BluetoothGATTCharacteristic::take(ScriptPromiseResolver* resolver, PassOwnPtr<WebBluetoothGATTCharacteristicInit> webCharacteristic)
{
    if (!webCharacteristic) {
        return nullptr;
    }
    BluetoothGATTCharacteristic* characteristic = new BluetoothGATTCharacteristic(resolver->executionContext(), webCharacteristic);
    // See note in ActiveDOMObject about suspendIfNeeded.
    characteristic->suspendIfNeeded();
    return characteristic;
}

void BluetoothGATTCharacteristic::stop()
{
    notifyCharacteristicObjectRemoved();
}

void BluetoothGATTCharacteristic::dispose()
{
    notifyCharacteristicObjectRemoved();
}

void BluetoothGATTCharacteristic::notifyCharacteristicObjectRemoved()
{
    if (!m_stopped) {
        m_stopped = true;
        WebBluetooth* webbluetooth = BluetoothSupplement::fromExecutionContext(ActiveDOMObject::executionContext());
        webbluetooth->characteristicObjectRemoved(m_webCharacteristic->characteristicInstanceID, this);
    }
}

ScriptPromise BluetoothGATTCharacteristic::readValue(ScriptState* scriptState)
{
    WebBluetooth* webbluetooth = BluetoothSupplement::fromScriptState(scriptState);

    ScriptPromiseResolver* resolver = ScriptPromiseResolver::create(scriptState);
    ScriptPromise promise = resolver->promise();
    webbluetooth->readValue(m_webCharacteristic->characteristicInstanceID, new CallbackPromiseAdapter<ConvertWebVectorToArrayBuffer, BluetoothError>(resolver));

    return promise;
}

ScriptPromise BluetoothGATTCharacteristic::writeValue(ScriptState* scriptState, const DOMArrayPiece& value)
{
    WebBluetooth* webbluetooth = BluetoothSupplement::fromScriptState(scriptState);
    // Partial implementation of writeValue algorithm:
    // https://webbluetoothchrome.github.io/web-bluetooth/#dom-bluetoothgattcharacteristic-writevalue

    // Let valueVector be a copy of the bytes held by value.
    std::vector<uint8_t> valueVector(value.bytes(), value.bytes() + value.byteLength());
    // If bytes is more than 512 bytes long (the maximum length of an attribute
    // value, per Long Attribute Values) return a promise rejected with an
    // InvalidModificationError and abort.
    if (valueVector.size() > 512)
        return ScriptPromise::rejectWithDOMException(scriptState, DOMException::create(InvalidModificationError, "Value can't exceed 512 bytes."));

    ScriptPromiseResolver* resolver = ScriptPromiseResolver::create(scriptState);

    ScriptPromise promise = resolver->promise();
    webbluetooth->writeValue(m_webCharacteristic->characteristicInstanceID, valueVector, new CallbackPromiseAdapter<void, BluetoothError>(resolver));

    return promise;
}

ScriptPromise BluetoothGATTCharacteristic::startNotifications(ScriptState* scriptState)
{
    WebBluetooth* webbluetooth = BluetoothSupplement::fromScriptState(scriptState);
    ScriptPromiseResolver* resolver = ScriptPromiseResolver::create(scriptState);
    ScriptPromise promise = resolver->promise();
    webbluetooth->startNotifications(m_webCharacteristic->characteristicInstanceID, this, new CallbackPromiseAdapter<void, BluetoothError>(resolver));
    return promise;
}

ScriptPromise BluetoothGATTCharacteristic::stopNotifications(ScriptState* scriptState)
{
    WebBluetooth* webbluetooth = BluetoothSupplement::fromScriptState(scriptState);
    ScriptPromiseResolver* resolver = ScriptPromiseResolver::create(scriptState);
    ScriptPromise promise = resolver->promise();
    webbluetooth->stopNotifications(m_webCharacteristic->characteristicInstanceID, this, new CallbackPromiseAdapter<void, BluetoothError>(resolver));
    return promise;
}

DEFINE_TRACE(BluetoothGATTCharacteristic)
{
    ActiveDOMObject::trace(visitor);
}

} // namespace blink
