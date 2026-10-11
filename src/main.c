// learning-sdl
//
// The bare minimum needed to open a window with SDL2, stand up an OpenGL 3.3
// core context via glad, and draw one colored square that you can move
// around with the arrow keys / WASD. No engine, no abstractions -- just the
// raw pieces so you can see how they fit together.
//
// Loosely modeled after the structure of engine-from-scratch's main loop
// (poll events -> update -> render), but collapsed into a single file.

#include <stdio.h>
#include <stdbool.h>

#include <glad/glad.h>   // loads OpenGL function pointers
#define SDL_MAIN_HANDLED // we provide our own main(), not SDL's
#include <SDL2/SDL.h>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include "math/math.h"
#include "models/cube.h"
#include "models/sphere.h"

#define MAX_CUBES 3
#define MAX_SPHERES 9

static void error_exit(const char *message)
{
    fprintf(stderr, "%s\n", message);
    exit(1);
}

static char *read_file(const char *path)
{
    FILE *f = fopen(path, "rb");
    if (!f)
        error_exit("couldn't open shader file");
    fseek(f, 0, SEEK_END);
    long size = ftell(f);
    fseek(f, 0, SEEK_SET);
    char *buf = malloc(size + 1);
    fread(buf, 1, size, f);
    buf[size] = '\0';
    fclose(f);
    return buf;
}

static unsigned int compile_shader(unsigned int type, const char *source)
{
    unsigned int shader = glCreateShader(type);
    glShaderSource(shader, 1, &source, NULL);
    glCompileShader(shader);

    int success;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        char log[512];
        glGetShaderInfoLog(shader, sizeof(log), NULL, log);
        fprintf(stderr, "Shader compile error: %s\n", log);
        exit(1);
    }

    return shader;
}

static unsigned int create_shader_program(const char *vertex_source, const char *fragment_source)
{
    unsigned int vertex_shader = compile_shader(GL_VERTEX_SHADER, vertex_source);
    unsigned int fragment_shader = compile_shader(GL_FRAGMENT_SHADER, fragment_source);

    unsigned int program = glCreateProgram();
    glAttachShader(program, vertex_shader);
    glAttachShader(program, fragment_shader);
    glLinkProgram(program);

    int success;
    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if (!success)
    {
        char log[512];
        glGetProgramInfoLog(program, sizeof(log), NULL, log);
        fprintf(stderr, "Shader link error: %s\n", log);
        exit(1);
    }

    // Once linked, the individual shader objects are no longer needed.
    glDeleteShader(vertex_shader);
    glDeleteShader(fragment_shader);

    return program;
}

int main(int argc, char *argv[])
{
    // ---------------------------------------------------------------
    // 1. Window + OpenGL context
    // ---------------------------------------------------------------
    if (SDL_Init(SDL_INIT_VIDEO) < 0)
    {
        error_exit(SDL_GetError());
    }

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);

    // mouse cursor shouldn't hit edge of the window
    SDL_SetRelativeMouseMode(SDL_TRUE);

    int window_width = 800;
    int window_height = 600;

    SDL_Window *window = SDL_CreateWindow(
        "learning-sdl",
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        window_width,
        window_height,
        SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);

    if (!window)
    {
        error_exit(SDL_GetError());
    }

    SDL_GLContext gl_context = SDL_GL_CreateContext(window);

    if (!gl_context)
    {
        error_exit(SDL_GetError());
    }

    if (!gladLoadGLLoader((GLADloadproc)SDL_GL_GetProcAddress))
    {
        error_exit("Failed to load OpenGL via glad");
    }

    // depth test after creating context:
    glEnable(GL_DEPTH_TEST);

    printf("OpenGL loaded\n");
    printf("Vendor:   %s\n", glGetString(GL_VENDOR));
    printf("Renderer: %s\n", glGetString(GL_RENDERER));
    printf("Version:  %s\n", glGetString(GL_VERSION));

    SDL_GL_SetSwapInterval(1); // vsync, so we're not spinning at max fps

    // ---------------------------------------------------------------
    // 2. Geometry: one quad, reused every frame for the moving square
    // ---------------------------------------------------------------
    // float vertices[] = {
    //     // x,     y
    //     0.5f, 0.5f,
    //     0.5f, -0.5f,
    //     -0.5f, -0.5f,
    //     -0.5f, 0.5f};

    // Vertices for a cube

    Mat4 cube_location = mat4_translate(vec3_create(-1.0f, 0.0f, -1.0f));
    Mat4 cube_scale = mat4_scale(vec3_create(1.0f, 3.0f, 1.0f));
    Mat4 cube_rotation_x = mat4_rotate_x(45 * (M_PI / 180));

    Vec3 light_position = vec3_create(10.0f, 10.0f, 10.0f);
    Vec3 light_color = vec3_create(0.8f, 0.8f, 0.8f);
    CubeMesh cube_mesh = cube_mesh_create();
    SphereMesh sphere_mesh = sphere_mesh_create();

    // Cube cubes[MAX_CUBES] = {
    //     cube_create(vec3_create(-2.0f, 0.0f, -2.0f), vec3_create(0.8f, 0.8f, 0.8f), vec3_create(0.0f, 0.0f, 0.0f)),
    //     cube_create(vec3_create(0.0f, 1.0f, -4.0f), vec3_create(1.0f, 2.0f, 1.0f), vec3_create(0.5f, 0.0f, 0.0f)),
    //     cube_create(vec3_create(2.0f, -1.0f, -1.0f), vec3_create(0.5f, 0.5f, 0.5f), vec3_create(0.0f, 1.0f, 0.0f))};

    printf("Creating spheres:\n");

    Sphere spheres[MAX_SPHERES] = {
        sphere_create(vec3_create(0.0f, 0.0f, 0.0f), vec3_create(0.0f, 0.0f, 0.0f), 6.95f, "assets/sun.jpg"),       // sun
        sphere_create(vec3_create(10.0f, 0.0f, 0.0f), vec3_create(0.0f, 0.0f, 0.0f), 0.124f, "assets/mercury.jpg"), // mercury
        sphere_create(vec3_create(15.0f, 0.0f, 0.0f), vec3_create(0.0f, 0.0f, 0.0f), 0.161f, "assets/venus.jpg"),   // venus
        sphere_create(vec3_create(20.0f, 0.0f, 0.0f), vec3_create(0.0f, 0.0f, 0.0f), 0.164f, "assets/earth.jpg"),   // earth
        sphere_create(vec3_create(25.0f, 0.0f, 0.0f), vec3_create(0.0f, 0.0f, 0.0f), 0.134f, "assets/mars.jpg"),    // mars
        sphere_create(vec3_create(30.0f, 0.0f, 0.0f), vec3_create(0.0f, 0.0f, 0.0f), 0.71f, "assets/jupiter.jpg"),  // jupiter
        sphere_create(vec3_create(35.0f, 0.0f, 0.0f), vec3_create(0.0f, 0.0f, 0.0f), 0.60f, "assets/saturn.jpg"),   // saturn
        sphere_create(vec3_create(40.0f, 0.0f, 0.0f), vec3_create(0.0f, 0.0f, 0.0f), 0.25f, "assets/uranus.jpg"),   // uranus
        sphere_create(vec3_create(45.0f, 0.0f, 0.0f), vec3_create(0.0f, 0.0f, 0.0f), 0.25f, "assets/neptune.jpg")}; // neptune
    // Sphere spheres[MAX_SPHERES] = {
    //     sphere_create(vec3_create(0.0f, 0.0f, 0.0f), vec3_create(0.0f, 0.0f, 0.0f), 6.95f, "assets/sun.jpg"),       // sun
    //     sphere_create(vec3_create(10.0f, 0.0f, 0.0f), vec3_create(0.0f, 0.0f, 0.0f), 0.024f, "assets/mercury.jpg"), // mercury
    //     sphere_create(vec3_create(15.0f, 0.0f, 0.0f), vec3_create(0.0f, 0.0f, 0.0f), 0.061f, "assets/venus.jpg"),   // venus
    //     sphere_create(vec3_create(20.0f, 0.0f, 0.0f), vec3_create(0.0f, 0.0f, 0.0f), 0.064f, "assets/earth.jpg"),   // earth
    //     sphere_create(vec3_create(25.0f, 0.0f, 0.0f), vec3_create(0.0f, 0.0f, 0.0f), 0.034f, "assets/mars.jpg"),    // mars
    //     sphere_create(vec3_create(30.0f, 0.0f, 0.0f), vec3_create(0.0f, 0.0f, 0.0f), 0.71f, "assets/jupiter.jpg"),  // jupiter
    //     sphere_create(vec3_create(35.0f, 0.0f, 0.0f), vec3_create(0.0f, 0.0f, 0.0f), 0.60f, "assets/saturn.jpg"),   // saturn
    //     sphere_create(vec3_create(40.0f, 0.0f, 0.0f), vec3_create(0.0f, 0.0f, 0.0f), 0.25f, "assets/uranus.jpg"),   // uranus
    //     sphere_create(vec3_create(45.0f, 0.0f, 0.0f), vec3_create(0.0f, 0.0f, 0.0f), 0.25f, "assets/neptune.jpg")}; // neptune

    float sphere_thetas[MAX_SPHERES] = {
        0.0f, .0026, .001, .00063, .00033, .000053, .0000213, .0000075, .00000381};

    printf("Loading shaders\n");
    unsigned int shader = create_shader_program(read_file("src/shaders/cube.vert"), read_file("src/shaders/cube.frag"));
    printf("Finish loading shaders\n");

    int loc_model = glGetUniformLocation(shader, "u_model");
    int loc_view = glGetUniformLocation(shader, "u_view");
    int loc_projection = glGetUniformLocation(shader, "u_projection");
    int loc_color = glGetUniformLocation(shader, "u_color");
    int loc_light = glGetUniformLocation(shader, "u_light_pos");
    int loc_light_color = glGetUniformLocation(shader, "u_light_color");

    // mouse positions
    float mouse_x;
    float mouse_y;
    float sensitivity = 0.001f;

    float fov_radians = 90 * (M_PI / 180);
    float aspect = (float)window_width / (float)window_height;
    float angular_speed = 0.5f;
    // float camera_angle = 0.0f;
    float camera_yaw = 235 * (M_PI / 180); // camera's rotation around y axis
    float camera_pitch = 0.0f;             // camera's rotation around x axis

    Vec3 camera_position = vec3_create(10.0f, 10.0f, 10.0f);
    float camera_speed = 3.0f;
    Vec3 target = vec3_create(0.0f, 0.0f, 0.0f);
    Vec3 front = vec3_create(0.0f, 0.0f, 0.0f);

    // ---------------------------------------------------------------
    // 4. Main loop: poll events -> update -> render
    // ---------------------------------------------------------------
    bool should_quit = false;
    Uint64 last_ticks = SDL_GetPerformanceCounter();
    Uint64 perf_freq = SDL_GetPerformanceFrequency();

    // testing
    Mat4 model = mat4_multiply(cube_rotation_x, cube_scale); // rotate * scale
    model = mat4_multiply(cube_location, model);             // translate * (rotate * scale)

    Mat4 view = {0};
    Mat4 projection = {0};

    printf("Entering main game loop\n");

    while (!should_quit)
    {
        Uint64 now_ticks = SDL_GetPerformanceCounter();
        float delta_time = (float)(now_ticks - last_ticks) / (float)perf_freq;
        last_ticks = now_ticks;

        // camera_angle += angular_speed * delta_time;

        SDL_Event event;
        while (SDL_PollEvent(&event))
        {
            if (event.type == SDL_QUIT)
            {
                should_quit = true;
            }
            else if (event.type == SDL_WINDOWEVENT && event.window.event == SDL_WINDOWEVENT_RESIZED)
            {
                window_width = event.window.data1;
                window_height = event.window.data2;
                glViewport(0, 0, window_width, window_height);
            }
            else if (event.type == SDL_MOUSEMOTION)
            {
                mouse_x = event.motion.xrel;
                mouse_y = event.motion.yrel;
                camera_yaw += mouse_x * sensitivity;
                camera_pitch -= mouse_y * sensitivity;
            }
        }

        const Uint8 *keys = SDL_GetKeyboardState(NULL);

        if (keys[SDL_SCANCODE_ESCAPE])
        {
            should_quit = true;
        }

        // float move_x = 0.0f;
        // float move_y = 0.0f;

        // if (keys[SDL_SCANCODE_LEFT] || keys[SDL_SCANCODE_A]) move_x -= 1.0f;
        // if (keys[SDL_SCANCODE_RIGHT] || keys[SDL_SCANCODE_D]) move_x += 1.0f;
        // if (keys[SDL_SCANCODE_UP] || keys[SDL_SCANCODE_W]) move_y -= 1.0f;
        // if (keys[SDL_SCANCODE_DOWN] || keys[SDL_SCANCODE_S]) move_y += 1.0f;

        // square_x += move_x * square_speed * delta_time;
        // square_y += move_y * square_speed * delta_time;

        // // Keep the square fully inside the window.
        // float half = square_size * 0.5f;
        // if (square_x - half < 0) square_x = half;
        // if (square_y - half < 0) square_y = half;
        // if (square_x + half > window_width) square_x = window_width - half;
        // if (square_y + half > window_height) square_y = window_height - half;
        front.x = cosf(camera_yaw) * cosf(camera_pitch);
        front.y = sinf(camera_pitch);
        front.z = sinf(camera_yaw) * cosf(camera_pitch);

        front = vec3_normalize(front);

        // Flatten forward onto ground plane
        Vec3 flat_forward = vec3_normalize(vec3_create(front.x, 0.0f, front.z));
        Vec3 right = vec3_normalize(vec3_cross(flat_forward, vec3_create(0.0f, 1.0f, 0.0f)));

        float move_forward = 0.0f;
        float move_right = 0.0f;

        if (keys[SDL_SCANCODE_W])
            move_forward += 1.0f;
        if (keys[SDL_SCANCODE_S])
            move_forward -= 1.0f;
        if (keys[SDL_SCANCODE_D])
            move_right += 1.0f;
        if (keys[SDL_SCANCODE_A])
            move_right -= 1.0f;

        Vec3 move = vec3_add(vec3_scale(flat_forward, move_forward), vec3_scale(right, move_right));
        move = vec3_normalize(move);
        camera_position = vec3_add(camera_position, vec3_scale(move, camera_speed * delta_time));

        target = vec3_add(camera_position, front);

        // // update view and projection matrix
        // // camera_position.x = 5 * cosf(camera_angle);
        // // camera_position.z = 5 * sinf(camera_angle);
        projection = mat4_perspective(fov_radians, aspect, 0.1f, 100.0f);
        view = mat4_look_at(camera_position, target, vec3_create(0, 1, 0));

        glClearColor(0.1f, 0.1f, 0.12f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // need depth buffer bc 3d

        // glUseProgram(shader);

        // // TODO: loop over cube list

        // // Use GL_TRUE to transpose because I'm using row major not column major
        // glUniformMatrix4fv(loc_model, 1, GL_TRUE, model.m);
        // glUniformMatrix4fv(loc_view, 1, GL_TRUE, view.m);
        // glUniformMatrix4fv(loc_projection, 1, GL_TRUE, projection.m);
        // glUniform4f(loc_color, 0.9f, 0.4f, 0.2f, 1.0f);

        // glBindVertexArray(vao);
        // // glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, NULL); // for 2d
        // glDrawArrays(GL_TRIANGLES, 0, 36);

        // cubes[0].rotation.y += 0.5f * delta_time;

        glUseProgram(shader);

        glUniformMatrix4fv(loc_view, 1, GL_TRUE, view.m);             // Loads view matrix into shader
        glUniformMatrix4fv(loc_projection, 1, GL_TRUE, projection.m); // Loads projection matrix into shader
        glUniform4f(loc_color, 0.9f, 0.4f, 0.2f, 1.0f);
        glUniform3f(loc_light, light_position.x, light_position.y, light_position.z);
        glUniform3f(loc_light_color, light_color.x, light_color.y, light_color.z);

        // for (int i = 0; i < MAX_CUBES; i++)
        // {
        //     cube_draw(&cube_mesh, &cubes[i], loc_model);
        // }

        for (int j = 0; j < MAX_SPHERES; j++)
        {
            float sphere_coord_x = spheres[j].position.x;
            float sphere_coord_z = spheres[j].position.z;
            float theta = sphere_thetas[j];
            spheres[j].position.x = sphere_coord_x * cosf(theta) - sphere_coord_z * sinf(theta);
            spheres[j].position.z = sphere_coord_x * sinf(theta) + sphere_coord_z * cosf(theta);
            spheres[j].rotation.y += 0.5f * delta_time;

            sphere_draw(&sphere_mesh, &spheres[j], loc_model);
        }

        SDL_GL_SwapWindow(window);
    }

    // ---------------------------------------------------------------
    // 5. Cleanup
    // ---------------------------------------------------------------
    // glDeleteVertexArrays(1, &vao);
    // glDeleteBuffers(1, &vbo);
    // glDeleteBuffers(1, &ebo);
    cube_mesh_destroy(&cube_mesh);

    glDeleteProgram(shader);

    SDL_GL_DeleteContext(gl_context);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}
