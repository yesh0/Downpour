uniform sampler2D screenColorBuffer;
uniform sampler2D background;
uniform float t;

void main(void) {
  vec2 pos = gl_TexCoord[0].st;
  vec4 color = texture2D(screenColorBuffer, pos);
  if (color.a == 0.0) {
    /* Water-free area */
    gl_FragColor = texture2D(background, pos);
  } else {
    /* Water effect */

    /* Reflection effect */
    float tt = t + (pos.x + pos.y);
    float ti = 3.0 * (2.0 * tt - sin(tt) * cos(tt)) + (pos.x + pos.y) * 10.0;
    vec2 reflected = pos + vec2(sin(ti), cos(ti)) * 0.018;
    /* Blur effect */
    vec4 bg = texture2D(background, reflected);
    for (float d = -0.01; d < 0.01; d += 0.002) {
     bg += texture2D(background, reflected + vec2(0.0, d));
    }
    bg /= 10.0;
    /* Water color */
    if (bg.a == 0.0) {
      color.a = color.a * 0.75;
    } else {
      color = mix(bg, color, color.a * 0.5);
    }
    gl_FragColor = color;
  }
}