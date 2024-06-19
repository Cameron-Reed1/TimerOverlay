#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <climits>
#include <cstdio>
#include <cstring>
#include <ctime>
#include <glm/common.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <wayland-client-protocol.h>

#include <unistd.h>
#include <iostream>


#include "shader_utils.h"
#include "shadersrc.h"
#include "character_utils.h"
#include "shortcuts.h"
#include "timer.h"
#include "config.h"


void error_callback(int error, const char* description)
{
    std::cerr << "Error " << error << ": " << description << std::endl;
}

int main()
{
    Config cfg = readConfig(getConfigPath("timer_overlay"));

    std::cout << glfwGetVersionString() << std::endl;
    glfwInitHint(GLFW_PLATFORM, GLFW_PLATFORM_WAYLAND);

    glfwSetErrorCallback(error_callback);
    if (glfwInit() != GLFW_TRUE) {
        std::cerr << "Error initializing glfw" << std::endl;
        return -1;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
    glfwWindowHint(GLFW_FOCUSED, GLFW_FALSE);
    glfwWindowHint(GLFW_FOCUS_ON_SHOW, GLFW_FALSE);
    glfwWindowHint(GLFW_DECORATED, GLFW_FALSE);
    glfwWindowHint(GLFW_SCALE_TO_MONITOR, GLFW_TRUE);
    glfwWindowHint(GLFW_SCALE_FRAMEBUFFER, GLFW_TRUE);
    glfwWindowHint(GLFW_TRANSPARENT_FRAMEBUFFER, GLFW_TRUE);
    glfwWindowHint(GLFW_MOUSE_PASSTHROUGH, GLFW_TRUE);
    glfwWindowHint(GLFW_POSITION_X, cfg.pos_x);
    glfwWindowHint(GLFW_POSITION_Y, cfg.pos_y);
    glfwWindowHintString(GLFW_WAYLAND_APP_ID, "timer-overlay");
    glfwWindowHintString(GLFW_X11_CLASS_NAME, "timer-overlay");
    glfwWindowHintString(GLFW_X11_INSTANCE_NAME, "timer-overlay");
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(250, 50, "Timer Overlay", nullptr, nullptr);
    if (window == nullptr) {
        std::cout << "Failed to create glfw window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);

    int width, height;
    glfwGetFramebufferSize(window, &width, &height);
    glViewport(0, 0, width, height);

    glewExperimental = GL_TRUE;
    int err = glewInit();
    if (err != GLEW_OK) {
        std::cerr << "Failed to initialize GLEW: " << glewGetErrorString(err) << std::endl;
        return -1;
    }

    glEnable(GL_CULL_FACE);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    GLuint vertShader = loadShaderDirect(shader_text_vert, GL_VERTEX_SHADER);
    GLuint fragShader = loadShaderDirect(shader_text_frag, GL_FRAGMENT_SHADER);
    GLuint shaderProgram = linkShaderProgram(vertShader, fragShader);
    glDeleteShader(vertShader);
    glDeleteShader(fragShader);

    glm::mat4 projection = glm::ortho(0.0f, 250.0f, 0.0f, 50.0f);
    glUseProgram(shaderProgram);
    glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "projection"), 1, GL_FALSE, glm::value_ptr(projection));


    Font font(cfg.font.c_str(), cfg.font_size);
    if (font.LoadError) {
        return -1;
    }

    GLuint VAO, VBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 6 * 4, NULL, GL_DYNAMIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), 0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    Timer timer;
    GlobalShortcuts shortcuts("timer_overlay");

    if (shortcuts.createSession() != 0) {
        std::cout << "Failed to create shortcuts session" << std::endl;
        glfwTerminate();
    }

    shortcuts.addShortcut("time1", "Adds 1 minute to the timer", "ALT+F5", [](void* timer_ptr)
        {
            ((Timer*)timer_ptr)->AddMinutes(1);
        }, &timer);
    shortcuts.addShortcut("time5", "Adds 5 minutes to the timer", "ALT+F6", [](void* timer_ptr)
        {
            ((Timer*)timer_ptr)->AddMinutes(5);
        }, &timer);
    shortcuts.addShortcut("time15", "Adds 15 minutes to the timer", "ALT+F7", [](void* timer_ptr)
        {
            ((Timer*)timer_ptr)->AddMinutes(15);
        }, &timer);
    shortcuts.addShortcut("time60", "Adds 1 hour to the timer", "ALT+F8", [](void* timer_ptr)
        {
            ((Timer*)timer_ptr)->AddMinutes(60);
        }, &timer);
    shortcuts.addShortcut("timeclear", "Clears the timer", "ALT+F9", [](void* timer_ptr)
        {
            ((Timer*)timer_ptr)->Clear();
        }, &timer);

    if (!shortcuts.alreadyBound()) {
        std::cout << "Requsting to bind keys" << std::endl;
        if (shortcuts.bindKeys() != 0) {
            std::cerr << "Failed to bind keys" << std::endl;
            return -1;
        }
    }

    shortcuts.listen();

    int64_t last_frame_seconds = LONG_MAX;
    while (!glfwWindowShouldClose(window)) {
        if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
            glfwSetWindowShouldClose(window, GLFW_TRUE);
        }

        glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        TimeDuration time_diff = timer.GetTimeLeft();
        if (time_diff.seconds == last_frame_seconds && !time_diff.negative) {
            continue;
        }

        char buf[50]; // max of hhh:mm:ss\0
        if (time_diff.negative) {
            memcpy(buf, "0:00", 5);
        } else if (time_diff.hours > 999) {
            memcpy(buf, "999:59:59", 10);
        } else if (time_diff.hours < 1) {
            sprintf(buf, "%ld:%02ld", time_diff.minutes, time_diff.seconds);
        } else {
            sprintf(buf, "%ld:%02ld:%02ld", time_diff.hours, time_diff.minutes, time_diff.seconds);
        }
        
        if (!time_diff.negative || (time_diff.seconds_absolute < 3 && time_diff.milliseconds > 500)) {
            font.RenderText(VAO, VBO, shaderProgram, buf, 0.0f, 10.0f, 1.0f);
        } else {
            usleep(500);
        }

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();
}
