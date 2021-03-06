// #include "Gui.h"
//
// #include <GL/glew.h>
// #include <GLFW/glfw3.h>
// #include "imgui.h"
// #include <backends/imgui_impl_opengl3.h>
// #include <ostream>
// #include <iostream>
// #include <backends/imgui_impl_glfw.h>
//
// GLFWwindow* main_window;
//
// bool lmb_down = false;
// bool mmb_down = false;
// bool rmb_down = false;
// GLuint mouse_x = 0;
// GLuint mouse_y = 0;
//
// static void glfw_error_callback(int error, const char* description)
// {
//     fprintf(stderr, "Glfw Error %d: %s\n", error, description);
// }
//
//
// /*void cursor_position_callback(GLFWwindow* window, double xpos, double ypos)
// {
//     int prev_mouse_x = mouse_x;
//     int prev_mouse_y = mouse_y;
//
//     mouse_x = xpos;
//     mouse_y = ypos;
//
//     //if (ImGui::IsAnyWindowHovered())
//     //    return;
//
//     int mouse_delta_x = mouse_x - prev_mouse_x;
//     int mouse_delta_y = prev_mouse_y - mouse_y;
//
//     if (true == lmb_down && (0 != mouse_delta_x || 0 != mouse_delta_y))
//     {
//     }
//     else if (true == rmb_down && (0 != mouse_delta_y))
//     {
//     }
// }
//
// void mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
// {
//     //if (ImGui::IsAnyWindowHovered())
//     //    return;
//
//     if (GLFW_MOUSE_BUTTON_LEFT == button)
//     {
//         if (action == GLFW_PRESS)
//             lmb_down = true;
//         else
//             lmb_down = false;
//     }
//     else if (GLFW_MOUSE_BUTTON_MIDDLE == button)
//     {
//         if (action == GLFW_PRESS)
//             mmb_down = true;
//         else
//             mmb_down = false;
//     }
//     else if (GLFW_MOUSE_BUTTON_RIGHT == button)
//     {
//         if (action == GLFW_PRESS)
//             rmb_down = true;
//         else
//             rmb_down = false;
//     }
// }*/
//
//
// void ShowGui()
// {
//     // Setup window
//     glfwSetErrorCallback(glfw_error_callback);
//     if (!glfwInit())
//     {
//         std::cout << "glfw init error" << std::endl;
//         return;
//     }
//
//     // Decide GL+GLSL versions
//
//     // GL 3.0 + GLSL 130
//     const char* glsl_version = "#version 330";
//     glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
//     glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
//     glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
// 	
//    // GLFWmonitor* monitor = glfwGetPrimaryMonitor();
//     //const GLFWvidmode* mode = glfwGetVideoMode(monitor);
//
//     // Create window with graphics context
//     main_window = glfwCreateWindow(500, 500, "Window", NULL, NULL);
//     if (!main_window)
//         return;
// 	
//     //glfwWindowHint(GLFW_DOUBLEBUFFER, GLFW_FALSE);
//
//     glfwMakeContextCurrent(main_window);
//     //glfwSwapInterval(1); // Enable vsync
//
//     if (glewInit() != GLEW_OK)
//     {
//         std::cerr << "Failed to initialize OpenGL loader" << std::endl;
//         return;
//     }
//
//     std::cout << "OpenGL version string: " << glGetString(GL_VERSION) << std::endl;
//
//
//     
//     // Setup Dear ImGui context
//     IMGUI_CHECKVERSION();
//     ImGui::CreateContext();
//     ImGuiIO& io = ImGui::GetIO();
//     //io.ConfigFlags |= ImGuiConfigFlags_NavEnableSetMousePos;     // Enable Keyboard Controls
//     //io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
//     //io.WantCaptureMouse = true;
//     
//     //glfwSetCursorPosCallback(main_window, cursor_position_callback);
//     //glfwSetMouseButtonCallback(main_window, mouse_button_callback);
//     
//     // Setup Dear ImGui style
//     ImGui::StyleColorsDark();
//     //ImGui::StyleColorsClassic();
//     
//     // Setup Platform/Renderer bindings
//     ImGui_ImplGlfw_InitForOpenGL(main_window, true);
//     ImGui_ImplOpenGL3_Init(glsl_version);
//     
//     ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);
//     	
//     // Main loop
//     while (!glfwWindowShouldClose(main_window))
//     {
//
// 		 static bool testFirstFrameStarted = false;
// 		 if (!testFirstFrameStarted)
// 			std::cout << "Opening GUI, this may take a second..." << std::endl;
//     	
// 		// Start the Dear ImGui frame
//         ImGui_ImplOpenGL3_NewFrame();
//         ImGui_ImplGlfw_NewFrame();
//         ImGui::NewFrame();
//      
//     	if (!testFirstFrameStarted)
//     	{
//             testFirstFrameStarted = true;
//             std::cout << "Opened GUI" << std::endl;
//     	}
//
//         {
//             int display_w, display_h;
//             glfwGetFramebufferSize(main_window, &display_w, &display_h);
//             glViewport(0, 0, display_w, display_h);
//
//
//             glClearColor(clear_color.x, clear_color.y, clear_color.z, clear_color.w);
//             glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
//
//             glDisable(GL_BLEND);
//
//             const GLfloat black[] = { 0.0f, 0.0f, 0.0f, 0.0f };
//             static const GLfloat one = 1.0f;
//             static const GLenum draw_buffers[] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1 };
//
//             glViewport(0, 0, display_w, display_h);
//         }
//     	
//         ImGui::ShowDemoWindow();
//
//         ImGui::Render();
//         ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
//
//         glfwSwapBuffers(main_window);
//
//         glfwPollEvents();
//         //glfwWaitEvents();
//     }
//
//     // Cleanup
//     ImGui_ImplOpenGL3_Shutdown();
//     ImGui_ImplGlfw_Shutdown();
//     ImGui::DestroyContext();
//
//     glfwDestroyWindow(main_window);
//     glfwTerminate();
//
//     std::cout << "Ended GUI" << std::endl;
// }
