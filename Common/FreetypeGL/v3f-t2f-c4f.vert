/* Freetype GL - A C OpenGL Freetype engine
 *
 * Distributed under the OSI-approved BSD 2-Clause License.  See accompanying
 * file `LICENSE` for more details.
 */
 
#version 330
 
uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

in vec3 vertex;
in vec2 tex_coord;
in vec4 color;

out vec2 TexCoord;
out vec4 Color;

void main()
{
    TexCoord = tex_coord.xy;
    Color     = color;
    gl_Position       = projection*(view*(model*vec4(vertex,1.0)));
}
