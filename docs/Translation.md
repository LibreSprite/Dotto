# How Dotto! language support works

Inside the data/i18n directory are all the files related to supporting different
languages. A typical language file looks something like this:

```ini
# Translations for <<Language Name>> (<<region>>)

English Text = Translated Text
English Text 2 = Translated Text 2
```

Whenever Dotto! tries to display the text on the left, it will search for it
in the translation files and replace it with the version on the right.
This search happens in the order of most-specific to least-specific:
For example, if the user's language is set to `pt_BR`, it will search in the following order:
    `pt_BR.ini`
    `pt.ini`
    `all.ini`

It is possible to have an as many files as necessary, simply by adding them to the locale,
allowing for specific regional variations (`pt_BR_nordeste`).

Variations do not need to be a full translation, they only need to contain the parts where they
differ (`pt_PT.ini` contains only a few translations since most of `pt.ini` fits). When using a
variant (`pt_BR`) as a base (`pt.ini`), an empty variant file (`pt_BR.ini`) should still be
provided.

If no translation is found, the original text is used as-is, which is why there are translations
for `en_US`.

`all.ini` should contain human-readable translations of language names. When the user wants to
change the language, a list of names (Português, English, Español, etc) is friendlier than
a list of locales (`pt_BR`, `en_US`, `es_ES`).
