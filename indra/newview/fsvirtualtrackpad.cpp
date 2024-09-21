/**
* @file irtualTrackpad.cpp
* @author Angeldark Raymaker; derived from llVirtualTrackball by Andrey Lihatskiy
* @brief Header file for FSVirtualTrackpad
*
* $LicenseInfo:firstyear=2001&license=viewerlgpl$
* Second Life Viewer Source Code
* Copyright (C) 2018, Linden Research, Inc.
*
* This library is free software; you can redistribute it and/or
* modify it under the terms of the GNU Lesser General Public
* License as published by the Free Software Foundation;
* version 2.1 of the License only.
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
* Linden Research, Inc., 945 Battery Street, San Francisco, CA  94111  USA
* $/LicenseInfo$
*/

// A two dimensional slider control with optional pinch-mode.

#include "fsvirtualtrackpad.h"
#include "llrect.h"
#include "lluictrlfactory.h"

// Globals
static LLDefaultChildRegistry::Register<FSVirtualTrackpad> register_virtual_trackball("fs_virtual_trackpad");

FSVirtualTrackpad::Params::Params()
  : border("border"),
    image_moon_back("image_moon_back"),
    image_moon_front("image_moon_front"),
    image_sphere("image_sphere"),
    image_sun_back("image_sun_back"),
    image_sun_front("image_sun_front"),
    pinch_mode("pinch_mode")
{
}

FSVirtualTrackpad::FSVirtualTrackpad(const FSVirtualTrackpad::Params &p)
  : LLUICtrl(p),
    mImgMoonBack(p.image_moon_back),
    mImgMoonFront(p.image_moon_front),
    mImgSunBack(p.image_sun_back),
    mImgSunFront(p.image_sun_front),
    mImgSphere(p.image_sphere),
    mAllowPinchMode(p.pinch_mode)
{
    LLRect border_rect = getLocalRect();
    _valueX = _lastValueX = _pinchValueX = _lastPinchValueX = border_rect.getCenterX();
    _valueY = _lastValueY = _pinchValueY = _lastPinchValueY = border_rect.getCenterY();
    _thumbClickOffsetX = _thumbClickOffsetY = _pinchThumbClickOffsetX = _pinchThumbClickOffsetY = 0;
    _valueWheelClicks = _pinchValueWheelClicks                                                  = 0;

    LLViewBorder::Params border = p.border;
    border.rect(border_rect);
    mBorder = LLUICtrlFactory::create<LLViewBorder>(border);
    addChild(mBorder);

    LLPanel::Params touch_area;
    touch_area.rect = LLRect(border_rect);
    mTouchArea = LLUICtrlFactory::create<LLPanel>(touch_area);
    addChild(mTouchArea);
}

FSVirtualTrackpad::~FSVirtualTrackpad() {}

bool FSVirtualTrackpad::postBuild()
{
    return true;
}

void FSVirtualTrackpad::drawThumb(bool isPinchThumb)
{
    LLUIImage *thumb;
    if (mTouchArea->isInEnabledChain())
        thumb = isPinchThumb ? mImgSunFront : mImgMoonFront;
    else
        thumb = isPinchThumb ? mImgSunBack : mImgMoonBack;

    S32 x = isPinchThumb ? _pinchValueX : _valueX;
    S32 y = isPinchThumb ? _pinchValueY : _valueY;

    S32 halfThumbWidth  = thumb->getWidth() / 2;
    S32 halfThumbHeight = thumb->getHeight() / 2;

    thumb->draw(LLRect(x - halfThumbWidth,
                       y + halfThumbHeight,
                       x + halfThumbWidth,
                       y - halfThumbHeight));
}

bool FSVirtualTrackpad::isPointInTouchArea(S32 x, S32 y) const
{
    if (!mTouchArea)
        return false;

    return mTouchArea->getRect().localPointInRect(x, y);
}

void FSVirtualTrackpad::determineThumbClickError(S32 x, S32 y)
{
    if (!mTouchArea)
        return;
    if (!mImgSunFront)
        return;

    _thumbClickOffsetX = 0;
    _thumbClickOffsetY = 0;

    S32 errorX = _valueX - x;
    S32 errorY = _valueY - y;
    if (fabs(errorX) > mImgSunFront->getWidth() / 2.0)
        return;
    if (fabs(errorY) > mImgSunFront->getHeight() / 2.0)
        return;

    _thumbClickOffsetX = errorX;
    _thumbClickOffsetY = errorY;
}

void FSVirtualTrackpad::determineThumbClickErrorForPinch(S32 x, S32 y)
{
    if (!mTouchArea)
        return;
    if (!mImgMoonFront)
        return;

    _pinchThumbClickOffsetX = 0;
    _pinchThumbClickOffsetY = 0;

    S32 errorX = _pinchValueX - x;
    S32 errorY = _pinchValueY - y;
    if (fabs(errorX) > mImgMoonFront->getWidth() / 2.0)
        return;
    if (fabs(errorY) > mImgMoonFront->getHeight() / 2.0)
        return;

    _pinchThumbClickOffsetX = errorX;
    _pinchThumbClickOffsetY = errorY;
}

void FSVirtualTrackpad::draw()
{
    mImgSphere->draw(mTouchArea->getRect(), mTouchArea->isInEnabledChain() ? UI_VERTEX_COLOR : UI_VERTEX_COLOR % 0.5f);

    if (mAllowPinchMode)
        drawThumb(true);

    drawThumb(false);

    LLView::draw();
}

void FSVirtualTrackpad::setValue(const LLSD& value)
{
    if (value.isArray() && value.size() == 2)
    {
        LLVector2 vec2;
        vec2.setValue(value);
        setValue(vec2.mV[VX], vec2.mV[VY]);
    }
}

void FSVirtualTrackpad::setValue(F32 x, F32 y) { convertNormalizedToPixelPos(x, y, &_valueX, &_valueY); }

void FSVirtualTrackpad::setPinchValue(F32 x, F32 y) { convertNormalizedToPixelPos(x, y, &_pinchValueX, &_pinchValueY); }

void FSVirtualTrackpad::undoLastValue() { setValueAndCommit(_lastValueX, _lastValueY); }

void FSVirtualTrackpad::undoLastSetPinchValue() { setPinchValueAndCommit(_lastPinchValueX, _lastPinchValueY); }

void FSVirtualTrackpad::setValueAndCommit(const S32 x, const S32 y)
{
    _valueX                    = x;
    _valueY                    = y;
    _valueWheelClicks          = -1 * _wheelClicksSinceMouseDown;
    _wheelClicksSinceMouseDown = 0;
    onCommit();
}

void FSVirtualTrackpad::setPinchValueAndCommit(const S32 x, const S32 y)
{
    _pinchValueX               = x;
    _pinchValueY               = y;
    _pinchValueWheelClicks     = -1 * _wheelClicksSinceMouseDown;
    _wheelClicksSinceMouseDown = 0;
    onCommit();
}

LLSD FSVirtualTrackpad::getValue()
{
    LLSD result   = normalizePixelPos(_valueX, _valueY, _valueWheelClicks).getValue();
    _valueWheelClicks = 0;

    return result;
}

LLSD FSVirtualTrackpad::getPinchValue()
{
    LLSD result        = normalizePixelPos(_pinchValueX, _pinchValueY, _pinchValueWheelClicks).getValue();
    _pinchValueWheelClicks = 0;

    return result;
}

bool FSVirtualTrackpad::handleHover(S32 x, S32 y, MASK mask)
{
    if (!hasMouseCapture())
        return true;

    S32 correctedX, correctedY;
    if (doingPinchMode)
    {
        correctedX = x + _pinchThumbClickOffsetX;
        correctedY = y + _pinchThumbClickOffsetY;
    }
    else
    {
        correctedX = x + _thumbClickOffsetX;
        correctedY = y + _thumbClickOffsetY;
    }

    LLRect rect = mTouchArea->getRect();
    rect.clampPointToRect(correctedX, correctedY);

    if (doingPinchMode)
    {
        _pinchValueX = correctedX;
        _pinchValueX = correctedY;
    }
    else
    {
        _valueX = correctedX;
        _valueY = correctedY;
    }

    onCommit();

    return true;
}

LLVector3 FSVirtualTrackpad::normalizePixelPos(S32 x, S32 y, S32 z) const
{
    LLVector3 result;
    if (!mTouchArea)
        return result;

    LLRect rect = mTouchArea->getRect();
    S32    centerX = rect.getCenterX();
    S32    centerY = rect.getCenterY();
    S32    width = rect.getWidth();
    S32    height = rect.getHeight();

    result.mV[VX] = (F32) (x - centerX) / width * 2;
    result.mV[VY] = (F32) (y - centerY) / height * 2;
    result.mV[VZ] = (F32) z;

    return result;
}

void FSVirtualTrackpad::convertNormalizedToPixelPos(F32 x, F32 y, S32 *valX, S32 *valY)
{
    if (!mTouchArea)
        return;

    LLRect rect    = mTouchArea->getRect();
    S32    centerX = rect.getCenterX();
    S32    centerY = rect.getCenterY();
    S32    width   = rect.getWidth();
    S32    height  = rect.getHeight();

    *valX = centerX + ll_round(llclamp(x, -1, 1) * width / 2);
    *valY = centerY + ll_round(llclamp(y, -1, 1) * height / 2);
}

bool FSVirtualTrackpad::handleMouseUp(S32 x, S32 y, MASK mask)
{
    if (hasMouseCapture())
    {
        doingPinchMode = false; 
        gFocusMgr.setMouseCapture(NULL);

        make_ui_sound("UISndClickRelease");
    }

    return LLView::handleMouseUp(x, y, mask);
}

bool FSVirtualTrackpad::handleMouseDown(S32 x, S32 y, MASK mask)
{
    if (isPointInTouchArea(x, y))
    {
        determineThumbClickError(x, y);
        _valueWheelClicks          = 0;
        _lastValueX                = _valueX;
        _lastValueY                = _valueY;
        _wheelClicksSinceMouseDown = 0;
        gFocusMgr.setMouseCapture(this);

        make_ui_sound("UISndClick");
    }

    return LLView::handleMouseDown(x, y, mask);
}

// move pinch cursor
bool FSVirtualTrackpad::handleRightMouseDown(S32 x, S32 y, MASK mask)
{
    if (!mAllowPinchMode)
        return LLView::handleRightMouseDown(x, y, mask);

    if (isPointInTouchArea(x, y))
    {
        determineThumbClickErrorForPinch(x, y);
        _pinchValueWheelClicks     = 0;
        _lastPinchValueX           = _pinchValueX;
        _lastPinchValueY           = _pinchValueY;
        _wheelClicksSinceMouseDown = 0;
        doingPinchMode = true;
        gFocusMgr.setMouseCapture(this);

        make_ui_sound("UISndClick");
    }

    return LLView::handleRightMouseDown(x, y, mask);
}

// pass wheel-clicks to third axis
bool FSVirtualTrackpad::handleScrollWheel(S32 x, S32 y, S32 clicks)
{
    if (hasMouseCapture())
    {
        if (doingPinchMode)
            _pinchValueWheelClicks = clicks;
        else
            _valueWheelClicks = clicks;

        _wheelClicksSinceMouseDown += clicks;

        return true;
    }
    else
        return LLUICtrl::handleScrollWheel(x, y, clicks);
}
