// Copyright (c) 2021 LibreSprite Authors (cf. AUTHORS.md)
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#include <gui/Flow.hpp>
#include <gui/Node.hpp>

namespace ui {
    void Flow::absolute(std::shared_ptr<Node> child, Rect& parentRect) {
        child->localRect.width = child->width->toPixel(parentRect.width, parentRect.width);
        child->localRect.height = child->height->toPixel(parentRect.height, parentRect.height);
        S32 outerWidth = child->localRect.width + child->margin->x + S32(child->margin->width);
        S32 outerHeight = child->localRect.height + child->margin->y + S32(child->margin->height);
        child->localRect.x = child->x->toPixel(parentRect.width, outerWidth);
        child->localRect.y = child->y->toPixel(parentRect.height, outerHeight);
        child->globalRect.x = child->localRect.x + parentRect.x;
        child->globalRect.y = child->localRect.y + parentRect.y;
        child->globalRect.width = child->localRect.width;
        child->globalRect.height = child->localRect.height;
    }

    class FlowFill : public Flow {
    public:
        void update(Vector<std::shared_ptr<Node>>& children, Rect& parentRect) override {
            for (auto& child : children) {
                if (*child->absolute) {
                    absolute(child, parentRect);
                } else {
                    child->localRect.x = 0;
                    child->localRect.y = 0;
                    child->localRect.width = parentRect.width;
                    child->localRect.height = parentRect.height;
                    child->globalRect.x = parentRect.x;
                    child->globalRect.y = parentRect.y;
                    child->globalRect.width = parentRect.width;
                    child->globalRect.height = parentRect.height;
                    child->localRect.width -= child->margin->x + S32(child->margin->width);
                    child->localRect.height -= child->margin->y + S32(child->margin->height);
                    child->globalRect.width -= child->margin->x + S32(child->margin->width);
                    child->globalRect.height -= child->margin->y + S32(child->margin->height);
                }
                child->localRect.x += child->margin->x;
                child->localRect.y += child->margin->y;
                child->globalRect.x += child->margin->x;
                child->globalRect.y += child->margin->y;
                child->onResize();
            }
        }
    };

    static Flow::Singleton<FlowFill> fill{"fill"};


    class FlowRow : public Flow {
    public:
        struct Size {
            Node* child;
            Unit given, min, max;
            bool done;
            U32 result;
            U32 offset;
            S32 margins;
        };

        void update(Vector<std::shared_ptr<Node>>& children, Rect& parentRect) override {
            Vector<Size> sizes;
            for (auto& child : children) {
                if (*child->absolute) {
                    absolute(child, parentRect);
                    child->localRect.x += child->margin->x;
                    child->localRect.y += child->margin->y;
                    child->globalRect.x += child->margin->x;
                    child->globalRect.y += child->margin->y;
                    child->onResize();
                } else {
                    sizes.push_back({
                            .child = child.get(),
                            .given = child->width,
                            .min = child->minWidth,
                            .max = child->maxWidth,
                            .margins = child->margin->x + S32(child->margin->width)
                        });
                }
            }
            fit(sizes, parentRect.width);
            position(sizes, parentRect.width);
            for (auto& size : sizes) {
                auto child = size.child;
                child->localRect.x = size.offset;
                child->localRect.y = 0;
                child->localRect.width = size.result;
                child->localRect.height = parentRect.height;
                child->globalRect.x = parentRect.x + size.offset;
                child->globalRect.y = parentRect.y;
                child->globalRect.width = size.result;
                child->globalRect.height = parentRect.height;
                child->localRect.x += child->margin->x;
                child->localRect.y += child->margin->y;
                child->globalRect.x += child->margin->x;
                child->globalRect.y += child->margin->y;
                child->localRect.width -= child->margin->x + S32(child->margin->width);
                child->localRect.height -= child->margin->y + S32(child->margin->height);
                child->globalRect.width -= child->margin->x + S32(child->margin->width);
                child->globalRect.height -= child->margin->y + S32(child->margin->height);
                child->onResize();
            }
        }

        virtual void position(Vector<Size>& sizes, U32 parent) {
            U32 offset = 0;
            for (auto& size : sizes) {
                size.offset = offset;
                offset += size.result;
            }
        }

        virtual void fit(Vector<Size>& sizes, U32 parent) {
            U32 totalWeight = 0;
            U32 fillWidth = parent;

            for (auto& size : sizes) {
                size.done = false;
                switch (size.given.getType()) {
                case Unit::Type::Pixel: {
                    size.done = true;
                    size.result = size.given.toPixel(parent, parent) + size.margins;
                    fillWidth -= size.result;
                    break;
                }
                case Unit::Type::Default: {
                    totalWeight += 1000 + size.margins;
                    size.result = 1000 + size.margins;
                    break;
                }
                case Unit::Type::Percent: {
                    size.result = size.given.toPixel(1000, 1000) + size.margins;
                    totalWeight += size.result + size.margins;
                    break;
                }
                }
            }

            for (auto& size : sizes) {
                if (!size.done) {
                    S32 result = fillWidth * (size.result / F32(totalWeight));
                    S32 adjusted = result;
                    if (size.min.getType() != Unit::Type::Default)
                        adjusted = std::max(adjusted, size.min.toPixel(parent, parent));
                    if (size.max.getType() != Unit::Type::Default)
                        adjusted = std::min(adjusted, size.max.toPixel(parent, parent));
                    fillWidth -= result - adjusted;
                    size.result = adjusted;
                    size.done = true;
                }
            }
        }
    };

    static Flow::Singleton<FlowRow> row{"row"};


    class FlowColumn : public FlowRow {
    public:
        void update(Vector<std::shared_ptr<Node>>& children, Rect& parentRect) override {
            Vector<Size> sizes;
            for (auto& child : children) {
                if (*child->absolute) {
                    absolute(child, parentRect);
                    child->localRect.x += child->margin->x;
                    child->localRect.y += child->margin->y;
                    child->globalRect.x += child->margin->x;
                    child->globalRect.y += child->margin->y;
                    child->onResize();
                } else {
                    sizes.push_back({
                            .child = child.get(),
                            .given = child->height,
                            .min = child->minHeight,
                            .max = child->maxHeight,
                            .margins = child->margin->y + S32(child->margin->height)
                        });
                }
            }
            fit(sizes, parentRect.height);
            position(sizes, parentRect.height);
            for (auto& size : sizes) {
                auto& child = size.child;
                child->localRect.x = 0;
                child->localRect.y = size.offset;
                child->localRect.width = parentRect.width;
                child->localRect.height = size.result;
                child->globalRect.x = parentRect.x;
                child->globalRect.y = parentRect.y + size.offset;
                child->globalRect.width = parentRect.width;
                child->globalRect.height = size.result;
                child->localRect.x += child->margin->x;
                child->localRect.y += child->margin->y;
                child->globalRect.x += child->margin->x;
                child->globalRect.y += child->margin->y;
                child->localRect.width -= child->margin->x + S32(child->margin->width);
                child->localRect.height -= child->margin->y + S32(child->margin->height);
                child->globalRect.width -= child->margin->x + S32(child->margin->width);
                child->globalRect.height -= child->margin->y + S32(child->margin->height);
                child->onResize();
            }
        }
    };

    static Flow::Singleton<FlowColumn> column{"column"};


}
