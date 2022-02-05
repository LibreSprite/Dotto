# LibreSprite Dotto! Theme Overview

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

```xml`
<window>
  <title>Hello World Title</title>
  <span>Just some text</span>
</window>
```

With this, you may have noticed that MainWindow itself is also a custom tag.

