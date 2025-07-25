## Version 3.2.1
- Fixed bug where multi-textured tiles are assigned the wrong textures after saving sometimes.
- Entities with non square sprites will now be scaled vertically to maintain aspect ratio.
- Fixed issue where missing textures are overwritten with other textures after saving and reloading.
- Removed Total Editor 2 importer.

## Version 3.2
- Fixed bug with entities being misplaced after map size is expanded.
- Made currently open file show up in the window title.
- Status bar will show cursor grid coordinates when nothing else is showing.
- Tile cursor will stay in place while using the scroll wheel if the mouse is not moved.
- Textures and shapes copied from other tiles will be highlighted properly in the texture/shape picker modes.
- Added setting for hiding certain file names from the texture / shape pickers.
- Tiles can now have primary and secondary textures.
- Removed hacky shapes and textures meant to work around 1 texture per tile limitations.
- Updated example maps to not use removed assets.
- Missing shape model files are now replaced by question marks.
- The editor will warn you about losing unsaved changes and converting map formats.
- Tweaks to enhance readability and visual organization of modals.
- Fixes to the .ti importer.
- Updated and corrected instructions.