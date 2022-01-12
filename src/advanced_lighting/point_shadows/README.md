# Usage

This is currently too confusing.

On the host side the light struct has just one texture entry called `depth_texture`. That is sometimes a regular 2D texture and sometimes a cube map. Since `depth_texture` is a GLuint in either case (the id of the texture) this is at least possible to do.

In the shader these two texture types are distinct, so the shader version of the light struct has *both* a sampler2D called `depth_texture` and a samplerCube called `cube_map`. The design of the light.c source is such that you either initialize the light as a directional light or as a point light (the `*_cube_map_init` version) and will return some error codes if you try to intialize both ways. Errors are thrown if storage is already allocated or not when the init functions are called. 

# Ways to reduce confusion

* It might be wise to introduce a typedef enum describing what kind of light is contained in the struct. This way initialization and matrix computation routines can throw if the wrong one is used.
* Another solution is to introduce two different texture address holders on the host-side struct, one intended to hold the regular 2d texture and the other intended to hold the cube map. This way a single light can hold all the information simultaneously. Advantage: host and device structs look the same. Disadvantage: It doesn't really make sense for one light to be both directional and point. Could lead to confusion and mistakes.
* Currently we find an unused texture unit by counting the number of textures present in the models being rendered. This works when we have one (or a small number of well known) models. We either should settle on a texture unit that will always and forever hold light texture information or we should add an array of light struct pointers as an argument to model drawing. Either way there is too much low level intervention in the drawing routine right now.
