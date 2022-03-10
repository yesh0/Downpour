uniform sampler2D screenColorBuffer;
uniform sampler2D background;
uniform float t;

void main(void) {
  vec2 pos = gl_TexCoord[0].st;
  vec4 color = texture2D(screenColorBuffer, pos);
  if (color.a == 0.0) {
    gl_FragColor = texture2D(background, pos);
  } else {
    float tt = t + (pos.x + pos.y);
    float ti = 3.0 * (2.0 * tt - sin(tt) * cos(tt)) + (pos.x + pos.y) * 10.0;
    vec4 background = texture2D(background, pos + vec2(sin(ti), cos(ti)) * 0.002);
    if (background.a != 0.0) {
      color = mix(background, color, color.a * 0.5);
    }
    gl_FragColor = color;
  }
}