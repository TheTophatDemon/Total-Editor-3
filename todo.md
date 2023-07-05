# TODO

- Optimize all shapes for cullability

- Add shape that doesn't cull at all for representing invisible volumes / collision zones

    * Could also have a metadata file for textures that prevents them from being culled

- Update documentation

- Allow entities to be marked as "tile entities" that take control of the tile they are inside of

    * Will prevent the tile from being culled when exporting, and will assign its geometry to its own node

- Giving entities some viewing mode that allows them to be visible from within tiles  

- Use instancing when rendering entities

- Allow secondary textures on tiles

- Shape icons zoom out for bigger shapes

- Refactor UI code or replace with ImGUI

- Consider giving entities billboard / model viewing modes
