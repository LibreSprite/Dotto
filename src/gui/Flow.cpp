// Copyright (c) 2021 LibreSprite Authors (cf. AUTHORS.md)
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#include <gui/Flow.hpp>
#include <gui/Node.hpp>

namespace ui {

    class FlowFill : public Flow {
    public:
        void update(Vector<std::shared_ptr<Node>>& children, ui::Rect& parentRect) override {
            for (auto& child : children) {
                child->localRect.x = 0;
                child->localRect.y = 0;
                child->localRect.width = parentRect.width;
                child->localRect.height = parentRect.height;
                child->globalRect.x = parentRect.x;
                child->globalRect.y = parentRect.y;
                child->globalRect.width = parentRect.width;
                child->globalRect.height = parentRect.height;
            }
        }
    };

    static Flow::Singleton<FlowFill> fill{"fill"};


    class FlowRow : public Flow {
    public:
        struct Size {
            Unit given, min, max;
            bool done;
            U32 result;
            U32 offset;
        };

        void update(Vector<std::shared_ptr<Node>>& children, ui::Rect& parentRect) override {
            Vector<Size> sizes;
            for (auto& child : children) {
                sizes.push_back({
                        .given = child->width,
                        .min = child->minWidth,
                        .max = child->maxWidth
                    });
            }
            fit(sizes, parentRect.width);
            position(sizes, parentRect.width);
            for (U32 i = 0, max = sizes.size(); i < max; ++i) {
                auto& child = children[i];
                auto& size = sizes[i];
                child->localRect.x = size.offset;
                child->localRect.y = 0;
                child->localRect.width = size.result;
                child->localRect.height = parentRect.height;
                child->globalRect.x = parentRect.x + size.offset;
                child->globalRect.y = parentRect.y;
                child->globalRect.width = size.result;
                child->globalRect.height = parentRect.height;
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
                    size.result = size.given.toPixel(0);
                    fillWidth -= size.result;
                    break;
                }
                case Unit::Type::Default: {
                    totalWeight += 1000;
                    size.result = 1000;
                    break;
                }
                case Unit::Type::Percent: {
                    size.result = size.given.toPixel(1000);
                    totalWeight += size.result;
                    break;
                }
                }
            }

            for (auto& size : sizes) {
                if (!size.done) {
                    S32 result = fillWidth * (size.result / F32(totalWeight));
                    S32 adjusted = result;
                    if (size.min.getType() != Unit::Type::Default)
                        adjusted = std::max(adjusted, size.min.toPixel(parent));
                    if (size.max.getType() != Unit::Type::Default)
                        adjusted = std::min(adjusted, size.max.toPixel(parent));
                    fillWidth -= result - adjusted;
                    size.result = adjusted;
                    size.done = true;
                }
                logI("Result: ", size.result);
            }
        }
    };

    static Flow::Singleton<FlowRow> row{"row"};


    class FlowColumn : public FlowRow {
    public:
        void update(Vector<std::shared_ptr<Node>>& children, ui::Rect& parentRect) override {
            Vector<Size> sizes;
            logI("reflow child count: ", children.size());
            for (auto& child : children) {
                sizes.push_back({
                        .given = child->height,
                        .min = child->minHeight,
                        .max = child->maxHeight
                    });
            }
            fit(sizes, parentRect.height);
            position(sizes, parentRect.height);
            for (U32 i = 0, max = sizes.size(); i < max; ++i) {
                auto& child = children[i];
                auto& size = sizes[i];
                child->localRect.x = 0;
                child->localRect.y = size.offset;
                child->localRect.width = parentRect.width;
                child->localRect.height = size.result;
                child->globalRect.x = parentRect.x;
                child->globalRect.y = parentRect.y + size.offset;
                child->globalRect.width = parentRect.width;
                child->globalRect.height = size.result;
            }
        }
    };

    static Flow::Singleton<FlowColumn> column{"column"};


}
