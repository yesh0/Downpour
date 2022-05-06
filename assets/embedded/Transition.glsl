uniform sampler2D screenColorBuffer;
uniform sampler2D background;
uniform float t;

void main(void) {
  vec2 pos = gl_TexCoord[0].st;
  vec4 color;
  if (texture2D(background, pos).a == 0.0) {
    color = texture2D(screenColorBuffer, pos);
  }
  color.a = color.a - (sin(0.2 * t + pos.x + pos.y) + 1.0) / 20.0;
  gl_FragColor = color;
}