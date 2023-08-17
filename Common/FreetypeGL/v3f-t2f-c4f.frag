/* Freetype GL - A C OpenGL Freetype engine
 *
 * Distributed under the OSI-approved BSD 2-Clause License.  See accompanying
 * file `LICENSE` for more details.
 */
 
#version 330

in vec2 TexCoord;
in vec4 Color;

out vec4 FragColor;
 
uniform sampler2D texture;
void main()
{
    float a = texture2D(texture, TexCoord).r;
    FragColor = vec4(Color.rgb, Color.a*a);
}
