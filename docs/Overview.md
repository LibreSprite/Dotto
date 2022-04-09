# LibreSprite Dotto! Internal Overview

## About this document

This document gives a high-level top-down overview of how the source code is organized, what 
the main classes are and how they relate to each other. It is meant for people interested in
working on the application as a whole and assumes familiarity with C++, OOP. Those who are still
learning will have an easier time getting started from a bottom-up approach and looking at
how to write a filter as an introduction do the Dotto! codebase.

## Dependency Injection

Dotto! heavily relies on Dependency Injection. A detailed explanation of how it works is available
in src/common/inject.hpp.

TL;DR: 

Classes depend on injectable interfaces:

```cpp
class App {
    inject<System> system{"new"};
};
```

The `system` variable will be initialized with whatever class is associated with the given key:

```cpp
System::Shared<SDL2System> sys{"new"}; // creates a new SDL2System when a new System is requested
```

With this, we can have multiple, interchangable implementations, allowing Dotto! to grow without
turning into a ratking of classes and it allows features to be enabled/disabled with very little
impact to the rest of the codebase.

## Main Classes

- app/App: The main application class. Responsible for setup, execution and shutdown.
- common/Value: Basically a wrapper around std::any with some extra features.
- log/Log: Used for logging.
- common/System: Responsible for platform-specific setup and event management.
- common/Surface: Platform-agnostic image data in RAM.
- fs/FileSystem: A virtual filesystem that hides platform-specific details from the rest of the application.
- common/Parser: All file data is always read by the FileSystem and decoded by one of the many parsers.
- common/Writer: Encodes data and writes it to a file in the FileSystem.
- fs/FileDialog: Used for showing open/save dialogs.
- task/TaskManager: Used for multithreading support, even on systems that don't have multithreading.
- common/Config: Stores settings and translations.
- cmd/Command: Used for implementing all user actions (eg, SaveFile) and document manipulation (eg, AddLayer).

## GUI Classes

- gui/Node: The main UI base class. Has dimensions and children but no actual appearance. Like an HTML div.
- gui/Image: A Node for rendering images. Somewhat like an HTML img.
- gui/Span: A Node for rendering simple text. Somewhat like an HTML span.
- gui/Window: Can be a native window on platforms that support it or an embedded one.
- gui/Unit: Represents coordinates and dimensions. Can be in pixels ("10px") or percentages ("50%").
- gui/Controller: Implements any behavior that can be added to a node through composition.

## Document Classes

See DottoFormat.md

## Portability Notes

- ALL file IO *must* be done through the FileSystem and associated classes: `fopen` is not always available.
- ALL console output *must* be done through the Log class. A platform might not have an actual console to log to.
- Multithreading *should* be done through the TaskManager, unless it's for something platform-specific.
- NEVER block the main thread (eg, sleeping or waiting for user input).
