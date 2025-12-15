#include <SDL2/SDL.h>
#include <SDL2/SDL_opengles2.h>
#include <stdio.h>
#include <stdlib.h>

/*
gcc -o gldraw gldraw.cpp \
    $(sdl2-config --cflags --libs) \
    -lGLESv2 -lEGL -lgbm -ldrm

*/

// --- OpenGL ES 2.0 Shaders (Minimal) ---

const char *vertexShaderSource =
    "attribute vec4 vPosition;\n"
    "void main() {\n"
    "  gl_Position = vPosition;\n"
    "}\n";

const char *fragmentShaderSource =
    "precision mediump float;\n"
    "void main() {\n"
    "  gl_FragColor = vec4(1.0, 0.0, 0.0, 1.0);\n" // Solid Red
    "}\n";

// --- Main Application Code ---

int main(int argc, char *argv[]) {
    SDL_Window *window = NULL;
    SDL_GLContext context;
    int success = 1;

    // 1. **CRITICAL STEP**: Force SDL to use the KMSDRM video driver.
    if (SDL_setenv("SDL_VIDEODRIVER", "kmsdrm", 1) < 0) {
        fprintf(stderr, "Warning: Could not set SDL_VIDEODRIVER to kmsdrm.\n");
    } else {
        printf("Set SDL_VIDEODRIVER=kmsdrm successfully.\n");
    }
    
    // Set OpenGL ES 2.0 attributes
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);

    // Set up double buffering
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);

    // 2. Initialize SDL2 (Video, Events, and OpenGL)
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        fprintf(stderr, "SDL could not initialize! SDL Error: %s\n", SDL_GetError());
        return 1;
    }
    printf("SDL Initialized successfully (KMSDRM driver in use).\n");

    // 3. Create a Window (Full screen is usually required/implied for KMSDRM)
    window = SDL_CreateWindow(
        "KMSDRM GLES Test",
        SDL_WINDOWPOS_UNDEFINED,
        SDL_WINDOWPOS_UNDEFINED,
        640, // Window size often ignored; KMS uses full screen mode
        480, // determined by the driver.
        SDL_WINDOW_OPENGL | SDL_WINDOW_FULLSCREEN | SDL_WINDOW_SHOWN
    );

    if (window == NULL) {
        fprintf(stderr, "Window creation failed! SDL Error: %s\n", SDL_GetError());
        success = 0;
    }

    // 4. Create the OpenGL Context (which uses EGL and GBM underneath)
    if (success) {
        context = SDL_GL_CreateContext(window);
        if (context == NULL) {
            fprintf(stderr, "OpenGL ES context creation failed! SDL Error: %s\n", SDL_GetError());
            success = 0;
        } else {
            printf("OpenGL ES Context created successfully.\n");
        }
    }
    
    // --- OpenGL Setup (Only if successful) ---
    GLuint program = 0;
    if (success) {
        // Simple error checking omitted for brevity, but this compiles/links the shaders
        GLuint vShader = glCreateShader(GL_VERTEX_SHADER);
        glShaderSource(vShader, 1, &vertexShaderSource, NULL);
        glCompileShader(vShader);

        GLuint fShader = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(fShader, 1, &fragmentShaderSource, NULL);
        glCompileShader(fShader);

        program = glCreateProgram();
        glAttachShader(program, vShader);
        glAttachShader(program, fShader);
        glBindAttribLocation(program, 0, "vPosition"); // Bind VBO location
        glLinkProgram(program);
        glDeleteShader(vShader);
        glDeleteShader(fShader);

        glUseProgram(program);
    }
    
    // 5. Drawing Loop & Present
    if (success) {
        printf("Drawing a red triangle using GLES...\n");
        
        // Define the vertices for a triangle
        GLfloat vertices[] = {
             0.0f,  0.5f, 0.0f,  // Top
            -0.5f, -0.5f, 0.0f,  // Bottom Left
             0.5f, -0.5f, 0.0f   // Bottom Right
        };

        // --- Render Frame ---
        
        // Clear the screen to blue
        glClearColor(0.0f, 0.0f, 1.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        // Tell GLES where the triangle vertices are
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, vertices);
        glEnableVertexAttribArray(0);

        // Draw the triangle
        glDrawArrays(GL_TRIANGLES, 0, 3);
        
        glDisableVertexAttribArray(0);

        // Swap the buffers (present the drawing to the screen using KMS)
        SDL_GL_SwapWindow(window);
        
        // Wait a few seconds
        printf("Displaying for 5 seconds...\n");
        SDL_Delay(5000); 
    }

    // 6. Cleanup
    if (program) glDeleteProgram(program);
    if (context) SDL_GL_DeleteContext(context);
    if (window) SDL_DestroyWindow(window);
    SDL_Quit();
    
    printf("Program finished.\n");
    return success ? 0 : 1;
}