# Shaders in this section

What's going on here: using the Blinn-Phong method we compute 2 dot products to get the specular contribution to the lighting (see the Blinn-Phong section fwhere this is implemented explicitly). Doing this in the frag shader one would multiply the texture normal by the tbn matrix. This requires performing this matrix-vector multiplication for each fragment.

However, for each of these dot products we can write (u, Mv) = (M^Tu, v). It does not make sense to use frag shader interpolation on the texture normals, hence we cannot move the multiplication Mv into the vertex shader. However, the vectors u are the light direction and camera direction - these we can happily perform frag shader interpolation on, and can therefore move the vertex shader. Therefore we will perform this matrix-vector multiplication only once per vertex instead of once-per fragment.

That is what is done using the model2.vert vertex shader and vert\_normal.frag fragment shader. The tbn matrix multiplies the two direction vectors mentioned above in the vertex shader.

To elaborate a little bit on why you don't want to interpolate the texture normal: Between the vertices of a square there can be very many fragments which draw from very many normals in the texture. Interpolating would mean you only see the normals defined at the vertices, and it would be like having no normal texture at all.
