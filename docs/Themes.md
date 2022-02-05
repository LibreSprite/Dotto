# LibreSprite Dotto! Themes

## General Overview

Dotto's user interface is made of scripts, images, XML and INI files, in a setup that is heavily
inspired by HTML and CSS. Unlike HTML, there are very few built-in element types:

* Node: an invisible element that is the base of all other elements.
* Window: an actual OS-native window.
* Span: an element that shows a simple line of text.
* Image: displays an image or a solid color.

All other UI elements are made up of these four types. 

To better understand how this works, let's make a "Hello world" theme.

- Save this as `skins/hello/gui/MainWindow.xml`, 

```xml
<window>
  <span font="%appdata/skins/default/DejaVuSans.ttf">Hello World</span>
</window>
```

- activate it in settings.ini:
`skin     = %appdata/skins/hello`

Run Dotto and you should see a window with "Hello world" on the top-left corner.
Note that span needs a font to work, there isn't a default one. It would be really
annoying if you had to specify the font for each and every span element, but this
can be avoided by using a `gui/style.ini` file:

```ini
[span]
font  = %skin/DejaVuSans.ttf
color = rgb{255,0,0}
```

The style.ini file is to Dotto XMLs as CSS is to HTML: you use it to apply
values to a given element. In this case, it will set the font and color of
all spans. You can also set a specific element's properties by using an id:

XML:
`<span id="title">Title</span>`

style.ini:

```ini
[span]
font  = %skin/DejaVuSans.ttf
color = rgb{255,0,0}

[@title]
size  = 64px
```

While it is possible to have multiple elements with the same id, it is
preferrable to create a custom node in that case:

`gui/title.xml`:

```xml
<span/>
```

`gui/style.ini`:

```ini
[span]
font  = %appdata/skins/default/DejaVuSans.ttf
color = rgb{255,0,0}
size  = 32px

[title]
priority = 1
size  = 64px
```

Here we increase the priority to ensure that the title's properties will 
override the span's.
By creating a `title.xml`, we can use it as a custom tag:

`gui/MainWindow.xml`:

```xml
<window>
  <title>Hello World Title</title>
  <span>Just some text</span>
</window>
```

With this, you may have noticed that MainWindow itself is also a custom tag.

---

## Flow

How an element organizes its children depends on its *flow*.
At the moment, there are three flows:
- Fill: Each child node occupies 100% of the parent area, like pancakes in a plate.
- Row: Each child is placed next to the other on the same line, like linked sausages.
- Column: Each child is on its own line, like bread in a dual-slot toaster.

In a row flow, children can specify their width, but they must occupy the full height
of the row. You can specify the width in pixels or in *weight* using percentage:

```xml
<node flow="row" height="100px">
   <image width="50%" multiply="#FF0000"/>
   <image width="100px" multiply="#00FF00"/>
   <image width="150%" multiply="#0000FF"/>
</node>
```

Here, the third element is 3 times bigger than the first, so it will occupy that
much of the parent's width after the 100px of the second child is subtracted:

`[ X pixels wide ][ 100 pixels wide ][    3X pixels wide    ]`

X + 100 + 3X = parent width

Note that parent size isn't affected by the children, like in HTML.
It is the parents' size that dictates the children's size.

The column flow is similar, but with height instead of width:

```xml
<node flow="column" width="100px">
   <image height="50%" multiply="#FF0000"/>
   <image height="100px" multiply="#00FF00"/>
   <image height="150%" multiply="#0000FF"/>
</node>
```

Results in:

```
[ X pixels tall   ]
[ 100px tall      ]
[ 3 X pixels tall ]
```

X + 100 + 3X = parent height

It is possible for a child to ignore a parent's flow and be positioned freely, 
using `absolute="true"`. With this, the child is free to set its x/y/width/height properties.
With absolute positioning, coordinates are relative to the parent's inner bounding box, 
that is, the area inside the parent's padding.

Examples:

Node fills the parents inner box completely:
`<image x="0" y="0" width="100%" height="100%"/>`

Node has a set offset and height, occupies half of the parent area:
`<image x="10px" y="10px" width="50%" height="10px"/>`

Node is on the bottom-right of the parent area:
`<image x="100%-10px" y="100%-10px" width="10px" height="10px"/>`

Same as above, but less math:
`<image x="right" y="bottom" width="10px" height="10px"/>`

Node has a set width and height, is in the center of the parent area:
`<image x="50%-100px" y="50%-100px" width="200px" height="200px"/>`

Same as above, but no need to calculate 50% of 200px manually:
`<image x="50%-50%" y="50%-50%" width="200px" height="200px"/>`

Same as above, but less math:
`<image x="center" y="center" width="200px" height="200px/>`

---

## Behaviour

By themselves, UI elements don't do anything. For that, nodes need to 
have one or more *controllers*. The button controller, for example, can
trigger a command once it is clicked. There are many built-in controllers
and custom ones can be written as scripts.



