Good — I read through your actual [main.c](src/main.c), [math.h](src/math/math.h), and [math.c](src/math/math.c) so this lesson builds on what you already have, in plain C, using your own vector library instead of GLM.

## Heads up before you start

A few things already in your files that will bite you if you don't know they're there (I'm not fixing these — just flagging so you don't waste an hour confused):

- [main.c:52](src/main.c#L52) — `in vec3 a_pos` is missing a semicolon.
- [main.c:58](src/main.c#L58) — `gl_position` should be `gl_Position`. GLSL is case-sensitive; you already print the compiler log in `compile_shader`, so this will show up there once you fix the semicolon.
- [main.c:189-205](src/main.c#L189-L205) — your cube only has front+back faces (8 vertices) and the index buffer only draws one face (6 indices, 2 triangles). A cube needs geometry/indices for all 6 faces — that's part of Stage 1 below.
- [math.c:65-70](src/math/math.c#L65-L70) — `vec3_dot` computes `a.z + b.z` instead of `a.z * b.z`. Doesn't affect `vec3_length`/`vec3_normalize` (they don't call it), but it will matter later once you use dot products to build a view matrix.
- [math.c:1](src/math/math.c#L1) — `sqrtf` is used but `<math.h>` (the standard library one — confusingly your own header is also named `math.h`) is never included. This may compile anyway under MSVC's implicit-declaration behavior, but it's worth including properly.

None of these stop you from starting Stage 1 — you'll just hit them as compiler/linker errors or subtly wrong numbers, which is normal.

## The mental model (same as any C++ version of this lesson)

```
local space (cube vertices)
        ↓  MODEL matrix     — where is the object in the world?
world space
        ↓  VIEW matrix      — where is the camera, what's it looking at?
camera space
        ↓  PROJECTION matrix — perspective: far things get smaller
NDC (-1..1)
        ↓  viewport transform (OpenGL does this for you)
pixels
```

Your current shader already declares `u_model`, `u_view`, `u_projection` — you just need to (a) fix its syntax, (b) actually query/set those uniform locations in C, and (c) have real matrices to upload.

## Why C instead of GLM changes the shape of the work

GLM gives you `mat4`, `translate()`, `rotate()`, `perspective()`, `lookAt()` for free. In C you're building the equivalent yourself in `math.h`/`math.c`. That's a good exercise, but it means "add a math library" isn't a side quest here — it's most of the lesson. Budget real time for it.

## Staged plan

### Stage 1 — Extend your math library with `Mat4`
Add a `Mat4` type to [math.h](src/math/math.h). Decide on a memory layout up front: OpenGL expects **column-major**, flat 16 floats, because `glUniformMatrix4fv(loc, 1, GL_FALSE, ptr)` reads it that way. Whether you store it as `float m[16]` or `float m[4][4]`, know which index is "row" vs "column" before you write anything else — getting this backwards is the single most common source of "my cube is smeared into nonsense" bugs.

Write `mat4_identity()` first. It's trivial and lets you test the plumbing (Stage 4) before any real math exists.

### Stage 2 — `mat4_multiply`
Standard matrix multiply: result[i][j] = row i of A dotted with column j of B. You'll need this to combine model/view/projection, and later to combine translation × rotation for the model matrix. Write a couple of manual test cases (e.g. identity × identity == identity, identity × X == X) before trusting it.

### Stage 3 — Finish the cube geometry
Decide: 8 shared vertices + a full index buffer (24-36 indices for 12 triangles), or 36 unindexed vertices (simpler to reason about, some duplication). Either is fine to learn on. Update the `glVertexAttribPointer`/`glDrawElements` (or `glDrawArrays`) call to match whichever you pick.

### Stage 4 — Wire up the uniforms with everything hardcoded
Query `u_model`, `u_view`, `u_projection` locations like you already do for `u_color`. Upload `mat4_identity()` for all three via `glUniformMatrix4fv`. You should see... your cube exactly as if there were no transforms at all (because there aren't yet). This stage exists purely to confirm the C ↔ shader wiring works before any real math is involved.

### Stage 5 — `mat4_perspective`
This is the one piece of "textbook" math I'd just look up rather than derive from scratch — it's the standard OpenGL perspective matrix from `fov`, `aspect`, `near`, `far`. Implement it as `mat4_perspective(float fov_radians, float aspect, float near, float far)`. Upload it as `u_projection`, keep model/view as identity. You should now see the cube's edges converge with distance instead of looking flat/orthographic.

### Stage 6 — Static camera via `mat4_look_at` (or equivalent)
You already have `vec3_cross`, `vec3_normalize`, and (once fixed) `vec3_dot` — that's everything you need to build a look-at matrix from `eye`, `target`, `world_up`:
1. `forward = normalize(eye - target)`
2. `right = normalize(cross(world_up, forward))`
3. `up = cross(forward, right)`
4. Pack `right`, `up`, `forward` as the matrix's rotation columns, and `-dot(right, eye)`, `-dot(up, eye)`, `-dot(forward, eye)` as the translation row.

Hardcode `eye = (0,0,5)`, `target = (0,0,0)`. This is exactly where your `vec3_dot` bug becomes load-bearing — watch for a cube that renders in a suspiciously wrong position and check that function first.

### Stage 7 — Animate yaw without touching the mouse
Before wiring up input, prove the camera math works by rotating it in code: `camera_yaw += 20.0f * delta_time`, recompute `target`/`front` each frame from yaw/pitch using:
```
front.x = cos(yaw) * cos(pitch)
front.y = sin(pitch)
front.z = sin(yaw) * cos(pitch)
```
(normalize the result). You should see the cube orbit smoothly. If it jumps or flips, that's a sign to debug now rather than after mouse input adds noise.

### Stage 8 — Mouse look
You already capture `xrel`/`yrel` into `mouse_x`/`mouse_y` at [main.c:280-284](src/main.c#L280-L284) but never use them, and `SDL_SetRelativeMouseMode(SDL_TRUE)` is already on. Feed the deltas into `camera_yaw`/`camera_pitch` with a sensitivity constant, clamp pitch to roughly ±89° so it can't flip over, and recompute `front` each frame as in Stage 7.

### Stage 9 — WASD relative to camera
Replace the `square_x`/`square_y`/`move_x`/`move_y` logic ([main.c:294-303](src/main.c#L294-L303)) with movement along `front`/`right`/`up` instead of screen axes. Watch for the diagonal-movement bug from the original lesson: if you add `front` and `right` contributions unnormalized, W+D moves ~41% faster than W alone. Normalize the combined move vector before scaling by speed and `delta_time`.

### Stage 10 — Polish
ESC-to-quit already works. Consider: click-to-recapture mouse, space/ctrl for vertical movement, and revisit `mat4_multiply`/`mat4_translate`/`mat4_rotate` so the model matrix can eventually do more than sit at the origin.

## Your first assignment

Stop at Stage 4. Concretely: fix the two shader syntax errors, finish the cube's vertex/index data, add `Mat4` + `mat4_identity` + `mat4_multiply` to your math library, and get a static, un-transformed cube rendering with identity matrices flowing through all three uniforms. Don't touch projection, view, or the camera yet — that's the next lesson once this compiles and draws correctly.

---

# Lesson 2 — Lighting and shadows

I read through [main.c](src/main.c), [cube.c](src/models/cube.c)/[cube.h](src/models/cube.h), and [math.c](src/math/math.c)/[math.h](src/math/math.h) as they stand now — the camera/MVP pipeline from Lesson 1 is working and three cubes are on screen. This lesson builds lighting (Phong) and then real-time shadows (shadow mapping) on top of that.

## Heads up before you start

- [cube.c:3-50](src/models/cube.c#L3-L50) — `CUBE_VERTICES` only stores position (3 floats/vertex). Lighting math needs a surface direction at every vertex, so you'll be adding a normal (3 more floats) per vertex. Because your cube is already 36 unindexed vertices (not a shared 8-vertex + index buffer), each face's 6 vertices can just get that face's constant normal baked in — no index-buffer rework needed.
- [cube.c:64](src/models/cube.c#L64) — `glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0)` assumes position-only, tightly-packed data. Once you interleave normals, the stride becomes `6 * sizeof(float)` and you'll add a second `glVertexAttribPointer`/`glEnableVertexAttribArray` call for location 1, with a `(void*)(3 * sizeof(float))` offset.
- [main.c:258](src/main.c#L258) — `cubes[1]`'s scale is `(1.0f, 2.0f, 1.0f)` — non-uniform. Keep an eye on this one: if you transform normals with the model matrix directly (or even `mat3(u_model)`), non-uniform scale skews them and this cube's lighting will look subtly wrong while the other two (uniform scale) look fine *by luck*. The fix is the inverse-transpose normal matrix in Stage 2 — this cube is your test case for whether you actually got it right.
- [main.c:191-193](src/main.c#L191-L193) and [main.c:326-328](src/main.c#L326-L328) — `cube_location`/`cube_scale`/`cube_rotation_x` and `model` are computed but never uploaded anywhere; the model matrix that's actually in use comes from `cube_get_model_matrix` inside [cube_draw](src/models/cube.c#L112-L120). Dead code, not a blocker, but don't go hunting for "the" model matrix in two places.
- [main.c:423](src/main.c#L423) — `u_color` is set once outside the per-cube loop, so all three cubes currently share one base color. Once lighting multiplies into it, they'll still all share that one hue — fine for this lesson, revisit later if you want per-cube materials.

## The mental model

**Lighting** is a per-fragment calculation (Blinn-Phong):

```
final_color = (ambient + diffuse + specular) * base_color

ambient  = ambient_strength * light_color              // fakes bounced/indirect light, always present
diffuse  = max(dot(N, L), 0) * light_color             // faces turned toward the light are brighter
specular = pow(max(dot(N, H), 0), shininess) * light_color  // the shiny highlight
```

- `N` — surface normal at the fragment (interpolated from vertex normals)
- `L` — direction from the surface point *to* the light
- `H` — halfway vector between `L` and the direction to the camera (Blinn-Phong uses this instead of a raw reflection vector — more stable, and the standard choice today)

**Shadows** (shadow mapping) add a second render pass, done once per frame *before* your normal draw:

```
Pass 1 — from the light's point of view, depth only
    light_space_matrix = light_projection * light_view
    render every cube into a small offscreen texture, writing only depth
    (this is your "shadow map": for every point on the ground the light can see,
     it records the depth of the *first* thing the light hits)

Pass 2 — from the camera's point of view (your existing pass)
    for each fragment:
        project its world position through light_space_matrix
        compare its depth to what's stored in the shadow map at that spot
        if something else was closer to the light -> this fragment is in shadow
```

Both passes render the *same cubes*, just with different matrices and different shaders — the depth pass doesn't care about color at all.

## Why shadows are a bigger jump than lighting

Lighting is "more math in the shaders you already have." Shadows require actual new GL machinery: framebuffers, a depth texture, a second shader program, and a second draw loop. Budget more time for Stage 8 than any single stage in Lesson 1.

Also, a deliberate simplification: true point-light shadows need six depth renders (a cubemap, one per cube face direction) because light radiates in every direction. That's a lesson on its own. Instead, for the shadow pass only, you'll point an orthographic "camera" from your light's position at the scene's center — i.e. treat the light as if it were directional, just for the purpose of building the shadow map. Your actual lighting math (Stages 3-4) keeps using `light_pos` as a real point for `L`. This is a common, deliberate cheat that looks fine for a light sitting outside a small, bounded scene like your three cubes.

## Staged plan

### Stage 1 — Add normals to the cube's vertex data
In [cube.c](src/models/cube.c), extend `CUBE_VERTICES` so each vertex is `x, y, z, nx, ny, nz`. Each face is flat, so all 6 vertices of a face share that face's normal: front `(0,0,1)`, back `(0,0,-1)`, left `(-1,0,0)`, right `(1,0,0)`, top `(0,1,0)`, bottom `(0,-1,0)`. Tedious but mechanical — go face by face.

### Stage 2 — Wire up the second vertex attribute
In `cube_mesh_create`, bump the stride on attribute 0 to `6 * sizeof(float)`, then add attribute 1: `glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)))` + `glEnableVertexAttribArray(1)`. In the vertex shader, add `layout (location = 1) in vec3 a_normal;`, compute `mat3 normal_matrix = transpose(inverse(mat3(u_model)));` and output `v_normal = normalize(normal_matrix * a_normal)` and `v_world_pos = vec3(u_model * vec4(a_pos, 1.0))` as `out` varyings (GLSL 330's `inverse()` works fine here — it's been in core since 1.40).

Before touching real lighting math: sanity-check by setting `frag_color = vec4(normalize(v_normal) * 0.5 + 0.5, 1.0)` in the fragment shader. Each face should render as a flat, distinct color, including on `cubes[1]`. If that non-uniform-scaled cube's faces look skewed/blended instead of flat and distinct, the normal matrix is wrong — fix it before moving on.

### Stage 3 — Ambient + diffuse
Add uniforms `u_light_pos` (vec3) and `u_light_color` (vec3). Hardcode a light position in C somewhere above/outside the cubes, e.g. `(3.0f, 4.0f, 3.0f)`, and upload it alongside view/projection in the main loop (same pattern as `loc_view`/`loc_projection`). In the fragment shader:
```
vec3 N = normalize(v_normal);
vec3 L = normalize(u_light_pos - v_world_pos);
vec3 ambient = 0.1 * u_light_color;
vec3 diffuse = max(dot(N, L), 0.0) * u_light_color;
frag_color = vec4((ambient + diffuse) * u_color.rgb, u_color.a);
```
You should see each cube shaded darker/lighter per face depending on which way it faces the light — and `cubes[1]` should look like a consistent, correctly-scaled box, not a lighting mess.

### Stage 4 — Specular (Blinn-Phong)
Add `u_view_pos` (upload `camera_position` each frame). In the fragment shader:
```
vec3 view_dir = normalize(u_view_pos - v_world_pos);
vec3 H = normalize(L + view_dir);
float spec = pow(max(dot(N, H), 0.0), 32.0);
vec3 specular = 0.5 * spec * u_light_color;
```
Add `specular` into the final sum. You should now see a small bright highlight on each cube that shifts as you move the camera (mouse look) — that movement is the tell that it's really using `view_pos` and not baked in.

### Stage 5 — Confirm it's reacting to geometry, not faked
Before moving to shadows, prove the lighting is really per-surface: temporarily animate `light_pos` in a small circle (reuse the `camera_yaw`-style accumulation pattern) and watch the diffuse/specular shift live across all three cubes independently based on their own position/rotation. Once you're convinced, you can go back to a fixed light position — shadows are easier to debug against a light that isn't moving.

### Stage 6 — `mat4_ortho`
Add `mat4_ortho(float left, float right, float bottom, float top, float near, float far)` to your math library, same shape as `mat4_perspective`. This is the light's projection matrix — directional light has no vanishing point, so no perspective divide. Pick bounds that comfortably contain all three cubes (look at their positions/scales in [main.c:256-259](src/main.c#L256-L259) to size the box).

### Stage 7 — The light-space matrix
Build `Mat4 light_view = mat4_look_at(light_pos, scene_center, vec3_create(0,1,0))` (you already have this function) and `Mat4 light_projection = mat4_ortho(...)` from Stage 6. Combine: `light_space_matrix = mat4_multiply(light_projection, light_view)`. This single matrix is what both the depth pass and the main pass will use to agree on "where is this point from the light's perspective."

### Stage 8 — The depth-only render pass
New GL objects, created once at startup:
- A framebuffer (`glGenFramebuffers`).
- A depth texture (`glGenTextures`, `glTexImage2D` with `GL_DEPTH_COMPONENT`/`GL_FLOAT`, reasonable size like 1024x1024), attached to the framebuffer via `glFramebufferTexture2D(..., GL_DEPTH_ATTACHMENT, ...)`.
- `glDrawBuffer(GL_NONE)` / `glReadBuffer(GL_NONE)` on that framebuffer — you're writing depth only, no color.
- Set the texture's wrap mode to `GL_CLAMP_TO_BORDER` with a border color of `(1,1,1,1)` — this makes anything outside the light's view read as "far away" (never shadowed) instead of wrapping/repeating.

A second, minimal shader program: vertex shader does `gl_Position = u_light_space_matrix * u_model * vec4(a_pos, 1.0);`, fragment shader can be an empty `void main() {}` — you don't need color output for a depth-only pass.

Each frame, before your normal draw: bind the depth framebuffer, `glViewport` to the shadow map's resolution, clear depth, loop over `cubes[]` calling `cube_draw` with this depth program's model-matrix location (note `cube_draw` already takes `loc_model` as a parameter and doesn't call `glUseProgram` itself — you can reuse it as-is for this pass). Then unbind (back to the default framebuffer) and restore your real viewport before the main draw.

### Stage 9 — Sample the shadow map in the main pass
Pass `u_light_space_matrix` to your *main* vertex shader too, and output `v_light_space_pos = u_light_space_matrix * vec4(v_world_pos, 1.0)` as another varying. In the main fragment shader:
```
vec3 proj_coords = v_light_space_pos.xyz / v_light_space_pos.w;
proj_coords = proj_coords * 0.5 + 0.5;  // [-1,1] -> [0,1] to match texture space
float closest_depth = texture(u_shadow_map, proj_coords.xy).r;
float current_depth = proj_coords.z;
float bias = 0.005;
float shadow = (current_depth - bias) > closest_depth ? 1.0 : 0.0;
```
Multiply `diffuse + specular` by `(1.0 - shadow)` (leave `ambient` alone — shadowed spots aren't pitch black). On the C side, bind the depth texture to a texture unit and set `u_shadow_map` via `glUniform1i` before the main draw loop, same place you set `loc_view`/`loc_projection`.

### Stage 10 — Known artifacts (expect at least one of these)
- **Shadow acne** — flickery self-shadowing stripes on lit faces. Increase `bias`, or render the depth pass with `glCullFace(GL_FRONT)` (only during that pass) so back faces write the depth instead of front faces.
- **Peter-panning** — shadow visibly detached from the object's base. Usually means your bias is now too large; it's a direct tradeoff against acne.
- **Hard edges** — a single shadow-map sample gives a binary shadow/no-shadow edge. Optional stretch goal: percentage-closer filtering (PCF) — average several samples around `proj_coords.xy` (e.g. a 3x3 texel neighborhood) instead of one, for a softer edge.

## Your second assignment

Stop at Stage 5. Get ambient + diffuse + specular lighting working correctly on all three cubes — including the non-uniformly-scaled `cubes[1]` — with a light position you can move and watch react live. Don't start the shadow map until this looks right with a static light; shadows are much harder to debug if you're not already sure the underlying lighting is correct.