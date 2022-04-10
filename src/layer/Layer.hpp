// Copyright (c) 2021 LibreSprite Authors (cf. AUTHORS.md)
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#pragma once

#include <memory>

#include <common/Messages.hpp>
#include <common/PubSub.hpp>
#include <common/Surface.hpp>
#include <common/inject.hpp>
#include <common/types.hpp>
#include <doc/Cell.hpp>

class Layer : public Injectable<Layer>, public std::enable_shared_from_this<Layer> {
    std::shared_ptr<Cell> _cell;
    Rect _localCanvas;
    Rect _globalCanvas;
    Point3D _globalMouse;
    U32 _buttons = 0;
    std::shared_ptr<Surface> _overlay;

public:
    virtual void setCell(std::shared_ptr<Cell> cell) {_cell = cell;}
    Cell& cell() {return *_cell;}

    virtual void setGlobalCanvas(const Rect& rect) {_globalCanvas = rect;}
    const Rect& globalCanvas() const {return _globalCanvas;}

    virtual void setLocalCanvas(const Rect& rect) {_localCanvas = rect;}
    const Rect& localCanvas() const {return _localCanvas;}

    virtual void setGlobalMouse(const Point3D& coords) {
        _globalMouse = coords;
    }

    virtual void setButtons(U32 buttons) {
        _buttons = buttons;
    }

    U32 buttons() const {return _buttons;}

    const Point3D& globalMouse() const {return _globalMouse;}

    Point3D localMouse() {
        Point3D point {
            _globalMouse.x - _globalCanvas.x,
            _globalMouse.y - _globalCanvas.y,
            _globalMouse.z
        };
        point.x = (point.x * S32(_localCanvas.width)) / S32(_globalCanvas.width);
        point.y = (point.y * S32(_localCanvas.height)) / S32(_globalCanvas.height);
        return point;
    }

    void setOverlayLayer(std::shared_ptr<Surface> surface) {
        _overlay = surface;
    }
    Surface* overlayLayer() {
        return _overlay.get();
    }
    virtual void clearOverlays() = 0;

    virtual void update() = 0;
};
