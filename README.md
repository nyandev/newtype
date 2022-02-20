# Newtype - A `nyandev` Library Classic.
Welcome, traveler!

**Newtype is a C++ middleware library and dependency wrapper for pretty text/font rendering for realtime graphics applications.**

More precisely, Newtype is a library that statically binds together an instance of **[FreeType](https://freetype.org/)** and an instance of **[HarfBuzz](https://github.com/harfbuzz/harfbuzz)**.
FreeType is perhaps the best known and most used cross-platform font rasterization engine, and HarfBuzz is a similarly mature and widely used cross-platform text shaping library.

### What's all that mean?
In simple terms, FreeType is the component that turns a font file's vector glyph into pretty pixels on a 2D texture, and HarfBuzz is the component that figures out which glyphs you'll need and with what sort of spacing and relative placement to render a text string of your choice, in any language and script, with pretty ligarures, diacritics and joining marks, correct orientation and direction and so forth.

These libraries each have supports built in for features of the other, but this circular nature of their co-dependency creates a [well-known dependency hell](https://groups.google.com/a/chromium.org/g/chromoting-reviews/c/fGzgqP6UPIs/m/ATJkFRlNAQAJ) for anyone looking to integrate and take full advantage of both libraries in their project.

### Right, so what's this Newtype thing then?
By far the easiest solution to this problem is just building both FreeType and HarfBuzz at once, statically, inside one wrapper library, and that is exactly what Newtype does.

Additionally, though, as Newtype was created for the needs of a (currently closed) game engine project, it is fairly opinionated in what features of each library are going to be used, and only in itself exports classes that wrap that functionality for even further simplicity.

**Newtype does not forward export any functions from either FreeType or Harfbuzz by themselves.**
Newtype has an API of its own, meant to be usable without the caller having to know anything about FreeType or HarfBuzz under the hood.

Newtype exports an interface for loading fonts, creating and updating text meshes, and dynamically manages atlas textures created from these fonts as needed.
Newtype currently does not support signed distance field rendering or textures, though those might well be added later.

### Licensing
**Newtype is licensed under the MIT license**. The author claims no right to any part of the underlying libraries, and even most of the functionality in Newtype has been mixed and matched from public sources elsewhere to create a minimum viable product rather quickly for a particular use case.

### Going forward
I wouldn't mind it at all if Newtype were to grow into something more mature and universally usable.  
All improvement suggestions, forks an pull requests are very welcome!  
If you'd just like to chat about the project, please visit the Discussions page.

### Thanks
Thanks for reading, and I hope're having a nice day, friend! :)
