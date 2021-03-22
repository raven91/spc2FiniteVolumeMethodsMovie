//
// Created by Nikita Kruk on 16.07.18.
//

#include "Renderer.hpp"

#include <iostream>
#include <cmath>
#include <map>
#include <cassert>
#include <iterator> // std::istream_iterator
#include <iomanip>
#include <sstream>

bool Renderer::stop_flag_ = true;
bool Renderer::pause_flag_ = true;
bool Renderer::take_screenshot_flag_ = false;
bool Renderer::show_time_flag_ = true;
int Renderer::frame_speed_ = 1; // 1 - the basic frame rate
GLfloat Renderer::x_rot_ = -0.1f;
GLfloat Renderer::y_rot_ = 0.1f;
GLfloat Renderer::z_rot_ = 0.2f;
glm::vec3 Renderer::camera_pos_ = glm::vec3(1.31f, 1.22f, 1.31f);
glm::vec3 Renderer::camera_front_ = glm::normalize(glm::vec3(-2.0f, -2.0f, -2.0f));
glm::vec3 Renderer::camera_up_ = glm::vec3(0.0f, 1.0f, 0.0f);
float Renderer::delta_time_ = 0.0f;  // Time between current frame and last frame
float Renderer::last_frame_ = 0.0f; // Time of last frame
float Renderer::last_x_ = 512.0f;
float Renderer::last_y_ = 512.0f;
GLfloat Renderer::yaw_ = 0.0f;
GLfloat Renderer::pitch_ = 0.0f;
bool Renderer::first_mouse_ = true;
float Renderer::fov_ = 45.0f;
int Renderer::screenshot_count_ = 0;
int Renderer::t_start_ = 0;

Renderer::Renderer(SimulationModel *simulation_model, ParameterContainer *parameter_container) :
    screenshot_handler_(),
    parameter_container_(parameter_container),
    simulation_model_(simulation_model)
{
  std::cout << "Renderer Created" << std::endl;

  simulation_model_->SkipTimeUnits(t_start_, parameter_container_->GetDtRecip());
}

Renderer::~Renderer()
{
  std::cout << "Renderer Deleted" << std::endl;
}

void Renderer::Start()
{
  GLFWwindow *window;

  glfwSetErrorCallback(Renderer::ErrorCallback);

  if (!glfwInit())
  {
    std::cerr << "Initialization of GLFW failure" << std::endl;
    exit(EXIT_FAILURE);
  }

  glfwWindowHint(GLFW_SAMPLES, 4);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
  glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_SAMPLES, 4);//MSAA

  window = glfwCreateWindow(1280/4, 1280/4, "Chimera In Continuum Limit", NULL, NULL);
  if (!window)
  {
    glfwTerminate();
    std::cerr << "Window opening failure" << std::endl;
    exit(EXIT_FAILURE);
  }

  glfwMakeContextCurrent(window);
  glfwSwapInterval(1);

  glfwSetKeyCallback(window, Renderer::KeyCallback);

//	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);
//	glfwSetCursorPosCallback(window, Renderer::MouseCallback);
  glfwSetScrollCallback(window, Renderer::ScrollCallback);

  int major, minor, rev;
  major = glfwGetWindowAttrib(window, GLFW_CONTEXT_VERSION_MAJOR);
  minor = glfwGetWindowAttrib(window, GLFW_CONTEXT_VERSION_MINOR);
  rev = glfwGetWindowAttrib(window, GLFW_CONTEXT_REVISION);
  std::cout << "OpenGL - " << major << "." << minor << "." << rev << std::endl;

  glewExperimental = GL_TRUE;
  if (glewInit() != GLEW_OK)
  {
    std::cerr << "GLEW initialization failure" << std::endl;
    glfwTerminate();
    exit(EXIT_FAILURE);
  }

  // contexts for grid, cells, text
  GLuint vao[3] = {0};
  GLuint vbo[3] = {0};
  glGenVertexArrays(3, &vao[0]);
  glGenBuffers(3, &vbo[0]);

  GLuint shader_program[3] = {0};
  InitShaders(shader_program);

  while (!glfwWindowShouldClose(window))
  {
    DisplayFunc(window, vao, vbo, shader_program);

    if (take_screenshot_flag_)
    {
      int width, height;
      glfwGetFramebufferSize(window, &width, &height);
//	  glfwGetWindowSize(window, &width, &height);
      screenshot_handler_.TakeScreenshotPng(width, height, screenshot_count_++);
      take_screenshot_flag_ = false;
    }

//    if (simulation_model_->GetCurrentTime() >= 100.0f)
//    {
//      glfwSetWindowShouldClose(window, GL_TRUE);
//    }

    ReadNewState();
    glfwSwapBuffers(window);
    glfwPollEvents();
  }

  FinFunc();
  glfwDestroyWindow(window);
  glfwTerminate();
}

void Renderer::ErrorCallback(int error, const char *description)
{
  std::cerr << description << std::endl;
}

void Renderer::KeyCallback(GLFWwindow *window, int key, int scancode, int action, int mods)
{
  float camera_speed = 2.0f * delta_time_; // adjust accordingly

  if (GLFW_PRESS == action)
  {
    switch (key)
    {
      case GLFW_KEY_ESCAPE : glfwSetWindowShouldClose(window, GL_TRUE);
        break;

      case GLFW_KEY_R : stop_flag_ = !stop_flag_;
        break;

      case GLFW_KEY_P :
        if (stop_flag_)
        {
          pause_flag_ = !pause_flag_;
        }
        break;

      case GLFW_KEY_F13 : take_screenshot_flag_ = true;
        break;

      case GLFW_KEY_O : ++frame_speed_;
        break;

      case GLFW_KEY_L :
        if (frame_speed_ > 1)
        {
          --frame_speed_;
        }
        break;

      case GLFW_KEY_T : show_time_flag_ = !show_time_flag_;
        break;

      case GLFW_KEY_LEFT : z_rot_ -= 0.1f;
        break;

      case GLFW_KEY_RIGHT : z_rot_ += 0.1f;
        break;

      case GLFW_KEY_UP : x_rot_ -= 0.1f;
        break;

      case GLFW_KEY_DOWN : x_rot_ += 0.1f;
        break;

      case GLFW_KEY_PAGE_UP : y_rot_ -= 0.1f;
        break;

      case GLFW_KEY_PAGE_DOWN : y_rot_ += 0.1f;
        break;

      case GLFW_KEY_W : camera_pos_ += camera_speed * camera_front_;
        break;

      case GLFW_KEY_S : camera_pos_ -= camera_speed * camera_front_;
        break;

      case GLFW_KEY_A : camera_pos_ += glm::normalize(glm::cross(camera_up_, camera_front_)) * camera_speed;
        break;

      case GLFW_KEY_D : camera_pos_ -= glm::normalize(glm::cross(camera_up_, camera_front_)) * camera_speed;
        break;

      case GLFW_KEY_Q : camera_pos_ += camera_up_ * camera_speed;
        break;

      case GLFW_KEY_E : camera_pos_ -= camera_up_ * camera_speed;
        break;

      default : break;
    }
    /*std::cout << x_rot_ << "," << y_rot_ << "," << z_rot_ << ",(" << camera_pos_.x << "," << camera_pos_.y << ","
              << camera_pos_.z << ")" << std::endl;*/
  }
}

void Renderer::MouseCallback(GLFWwindow *window, double xpos, double ypos)
{
  if (first_mouse_) // this bool variable is initially set to true
  {
    last_x_ = xpos;
    last_y_ = ypos;
    first_mouse_ = false;
  }

  float xoffset = xpos - last_x_;
  float yoffset = last_y_ - ypos; // reversed since y-coordinates range from bottom to top
  last_x_ = xpos;
  last_y_ = ypos;

  float sensitivity = 0.05f;
  xoffset *= sensitivity;
  yoffset *= sensitivity;

  yaw_ += xoffset;
  pitch_ += yoffset;

  if (pitch_ > 89.0f)
  {
    pitch_ = 89.0f;
  }
  if (pitch_ < -89.0f)
  {
    pitch_ = -89.0f;
  }

  glm::vec3 front;
  front.x = std::cosf(glm::radians(pitch_)) * std::cosf(glm::radians(yaw_));
  front.y = std::sinf(glm::radians(pitch_));
  front.z = std::cosf(glm::radians(pitch_)) * std::sinf(glm::radians(yaw_));
  camera_front_ = glm::normalize(front);
}

void Renderer::ScrollCallback(GLFWwindow *window, double xoffset, double yoffset)
{
  if (fov_ >= 1.0f && fov_ <= 45.0f)
  {
    fov_ -= yoffset;
  }
  if (fov_ <= 1.0f)
  {
    fov_ = 1.0f;
  }
  if (fov_ >= 45.0f)
  {
    fov_ = 45.0f;
  }
}

void Renderer::InitShaders(GLuint *shader_program)
{
  shader_program[0] = CreateProgramFromShader(
      std::string("/Users/nikita/CLionProjects/spc2FiniteVolumeMethodsMovie/Shaders/grid_vertex_shader.shader"),
      std::string("/Users/nikita/CLionProjects/spc2FiniteVolumeMethodsMovie/Shaders/grid_fragment_shader.shader"));
  /*SetShaderParameter(shader_program[0],
                     glm::vec3(parameter_container_->GetN(),
                               parameter_container_->GetM(),
                               parameter_container_->GetL()),
                     "dims");*/
  shader_program[1] = CreateProgramFromShader(
      std::string("/Users/nikita/CLionProjects/spc2FiniteVolumeMethodsMovie/Shaders/cell_vertex_shader.shader"),
      std::string("/Users/nikita/CLionProjects/spc2FiniteVolumeMethodsMovie/Shaders/cell_geometry_shader_v1.shader"),
      std::string("/Users/nikita/CLionProjects/spc2FiniteVolumeMethodsMovie/Shaders/cell_fragment_shader.shader"));
  SetShaderParameter(shader_program[1],
                     glm::vec3(parameter_container_->GetN(),
                               parameter_container_->GetM(),
                               parameter_container_->GetL()),
                     "dims");

  shader_program[2] = CreateProgramFromShader(
      std::string("/Users/nikita/CLionProjects/spc2FiniteVolumeMethodsMovie/Shaders/text_vertex_shader.shader"),
      std::string("/Users/nikita/CLionProjects/spc2FiniteVolumeMethodsMovie/Shaders/text_fragment_shader.shader"));
  if (FT_Init_FreeType(&ft_))
  {
    std::cerr << "Could not init freetype library" << std::endl;
  }
  if (FT_New_Face(ft_, "/System/Library/Fonts/HelveticaNeue.ttc", 0, &face_))
  {
    std::cerr << "Could not open font" << std::endl;
  }
  FT_Set_Pixel_Sizes(face_, 0, 20);//48
  GLuint texture;
  glActiveTexture(GL_TEXTURE0);
  glGenTextures(1, &texture);
  glBindTexture(GL_TEXTURE_2D, texture);
  GLint tex_location = glGetUniformLocation(shader_program[2], "tex");
  glUniform1i(tex_location, 0);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
}

void Renderer::SetShaderParameter(GLuint shader_program,
                                  GLfloat parameter_value,
                                  const std::string &parameter_name_in_shader)
{
  GLint location = glGetUniformLocation(shader_program, parameter_name_in_shader.c_str());
  if (-1 != location)
  {
    glUniform1f(location, parameter_value);
  }
}

void Renderer::SetShaderParameter(GLuint shader_program,
                                  GLint parameter_value,
                                  const std::string &parameter_name_in_shader)
{
  GLint location = glGetUniformLocation(shader_program, parameter_name_in_shader.c_str());
  if (-1 != location)
  {
    glUniform1i(location, parameter_value);
  }
}

void Renderer::SetShaderParameter(GLuint shader_program,
                                  const glm::vec3 &parameter_value,
                                  const std::string &parameter_name_in_shader)
{
  GLint location = glGetUniformLocation(shader_program, parameter_name_in_shader.c_str());
  if (-1 != location)
  {
    glUniform3fv(location, 1, glm::value_ptr(parameter_value));
  }
}

void Renderer::FinFunc()
{

}

void Renderer::ReadNewState()
{
  if (!stop_flag_ || !pause_flag_)
  {
    simulation_model_->ReadNewState();
    // speed up or down the simulation output
    simulation_model_->SkipTimeUnits(frame_speed_ - 1, 1);
    pause_flag_ = true;
  }
}

void Renderer::CreateTransformationMatrices(int width,
                                            int height,
                                            glm::mat4 &model,
                                            glm::mat4 &view,
                                            glm::mat4 &projection)
{
  projection = glm::perspective(glm::radians(fov_), (float) width / (float) height, 0.1f, 100.0f);

  view = glm::lookAt(camera_pos_, camera_pos_ + camera_front_, camera_up_);

  model = glm::translate(glm::mat4(1.0f), glm::vec3(-0.5f, -0.5f, -0.5f));
  model = glm::rotate(glm::mat4(1.0f), z_rot_, glm::vec3(0.0f, 1.0f, 0.0f))
      * glm::rotate(glm::mat4(1.0f), y_rot_, glm::vec3(0.0f, 0.0f, 1.0f))
      * glm::rotate(glm::mat4(1.0f), x_rot_, glm::vec3(1.0f, 0.0f, 0.0f))
      * model;// * glm::rotate(glm::mat4(1.0f), y_rot, glm::vec3(0.0f, 0.0f, 1.0f));
//  model = glm::scale(model, glm::vec3(1.0f, 1.0f, 1.0f / (2.0f * M_PI)));
}

void Renderer::ImportTransformationMatrices(GLuint shader_program,
                                            const glm::mat4 &model,
                                            const glm::mat4 &view,
                                            const glm::mat4 &projection)
{
  glUseProgram(shader_program);
  GLint projection_id = glGetUniformLocation(shader_program, "projection");
  if (-1 != projection_id)
  {
    glUniformMatrix4fv(projection_id, 1, GL_FALSE, glm::value_ptr(projection));
  }
  GLint view_id = glGetUniformLocation(shader_program, "view");
  if (-1 != view_id)
  {
    glUniformMatrix4fv(view_id, 1, GL_FALSE, glm::value_ptr(view));
  }
  GLint model_id = glGetUniformLocation(shader_program, "model");
  if (-1 != model_id)
  {
    glUniformMatrix4fv(model_id, 1, GL_FALSE, glm::value_ptr(model));
  }
}

void Renderer::DisplayFunc(GLFWwindow *window, GLuint *vao, GLuint *vbo, GLuint *shader_program)
{
  // Enable depth test
  glEnable(GL_DEPTH_TEST);
  // Accept fragment if it closer to the camera than the former one
  glDepthFunc(GL_LESS);

  glEnable(GL_BLEND);
//	glBlendEquation(GL_MAX);
//	glBlendFuncSeparate(GL_DST_COLOR, GL_ONE_MINUS_DST_ALPHA, GL_ONE, GL_ZERO);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
//	glBlendFunc(GL_DST_ALPHA, GL_ONE_MINUS_SRC_COLOR);
//  glBlendFunc(GL_DST_ALPHA, GL_SRC_COLOR);
//  glBlendEquation(GL_FUNC_ADD);

  glShadeModel(GL_SMOOTH);
  glEnable(GL_CULL_FACE);
  glCullFace(GL_BACK);
  glEnable(GL_LINE_SMOOTH);
//	glLineWidth(2.0f);

//	glEnable(GL_PROGRAM_POINT_SIZE);
  glEnable(GL_MULTISAMPLE); // MSAA

  float current_frame = glfwGetTime();
  delta_time_ = current_frame - last_frame_;
  last_frame_ = current_frame;

  glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
  // Clear the screen
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  int width, height;
  glfwGetFramebufferSize(window, &width, &height);
  glm::mat4 projection;
  glm::mat4 view;
  glm::mat4 model;
  CreateTransformationMatrices(width, height, model, view, projection);

  ImportTransformationMatrices(shader_program[0], model, view, projection);
  ImportTransformationMatrices(shader_program[1], model, view, projection);
  ImportTransformationMatrices(shader_program[2], model, view, projection);

  glUseProgram(shader_program[0]);
  RenderGrid(vao[0], vbo[0], shader_program[0]);

  glUseProgram(shader_program[1]);
  RenderCells(model, vao[1], vbo[1], shader_program[1]);

  if (show_time_flag_)
  {
    glCullFace(GL_FRONT);
    glUseProgram(shader_program[2]);
    GLfloat sx = 2.0f / width;
    GLfloat sy = 2.0f / height;
    std::ostringstream buffer;
    buffer << std::fixed << std::setprecision(2) << "t = " << simulation_model_->GetCurrentTime();
    RenderText(buffer.str(), -1.0f + 8.0f * sx, 1.0f - 50.0f * sy/2.0f, sx, sy, vao[2], vbo[2], shader_program[2]);
  }
}

void Renderer::RenderGrid(GLuint vao, GLuint vbo, GLuint shader_program)
{
  GLfloat x_size = 1.0f;
  GLfloat y_size = 1.0f;
  GLfloat phi_size = 1.0f;
  const int number_of_lines = 12;
  const int number_of_points_per_line = 2;
  const int dim = 3;
  const static GLfloat border_vertices[number_of_lines * number_of_points_per_line * dim] =
      {
          0.0f, 0.0f, 0.0f, x_size, 0.0f, 0.0f,
          x_size, 0.0f, 0.0f, x_size, y_size, 0.0f,
          x_size, y_size, 0.0f, 0.0f, y_size, 0.0f,
          0.0f, y_size, 0.0f, 0.0f, 0.0f, 0.0f,
          0.0f, 0.0f, phi_size, x_size, 0.0f, phi_size,
          x_size, 0.0f, phi_size, x_size, y_size, phi_size,
          x_size, y_size, phi_size, 0.0f, y_size, phi_size,
          0.0f, y_size, phi_size, 0.0f, 0.0f, phi_size,
          0.0f, 0.0f, 0.0f, 0.0f, 0.0f, phi_size,
          x_size, 0.0f, 0.0f, x_size, 0.0f, phi_size,
          x_size, y_size, 0.0f, x_size, y_size, phi_size,
          0.0f, y_size, 0.0f, 0.0f, y_size, phi_size
      };

  glBindVertexArray(vao);
  glBindBuffer(GL_ARRAY_BUFFER, vbo);
  glBufferData(GL_ARRAY_BUFFER,
               number_of_lines * number_of_points_per_line * dim * sizeof(GLfloat),
               border_vertices,
               GL_STATIC_DRAW);

  GLint position_attribute = glGetAttribLocation(shader_program, "position");
  glVertexAttribPointer(position_attribute, dim, GL_FLOAT, GL_FALSE, 0, (void *) 0);
  glEnableVertexAttribArray(position_attribute);

  glDrawArrays(GL_LINES, 0, number_of_lines * number_of_points_per_line);

  glDisableVertexAttribArray(position_attribute);
}

void Renderer::RenderCells(const glm::mat4 &model, GLuint vao, GLuint vbo, GLuint shader_program)
{
  const std::vector<GLfloat> &system_state = simulation_model_->GetCurrentState();

  /*std::map<GLfloat, int> depth_sorted_indices;
  for (int idx = 0; idx < parameter_container_->GetNumberOfCells(); ++idx)
  {
    // note: comparison to the camera position results in visual artifacts (low precision?)
    glm::vec3 vector_difference =
        camera_pos_
            - glm::vec3(model * glm::vec4(system_state[parameter_container_->GetNumberOfStateVariables() * idx + 0],
                                          system_state[parameter_container_->GetNumberOfStateVariables() * idx + 2],
                                          1.0f - system_state[parameter_container_->GetNumberOfStateVariables() * idx
                                              + 1],
                                          1.0f));
    GLfloat distance = glm::length(vector_difference);
    depth_sorted_indices[distance] = idx;
  } // idx
  static std::vector<GLfloat> sorted_system_state
      (parameter_container_->GetNumberOfStateVariables() * parameter_container_->GetNumberOfCells(), 0.0f);
  int idx = 0;
  for (std::map<GLfloat, int>::reverse_iterator rit = depth_sorted_indices.rbegin(); rit != depth_sorted_indices.rend();
       ++rit, ++idx)
  {
    sorted_system_state[parameter_container_->GetNumberOfStateVariables() * idx + 0] =
        system_state[parameter_container_->GetNumberOfStateVariables() * rit->second + 0];
    sorted_system_state[parameter_container_->GetNumberOfStateVariables() * idx + 1] =
        system_state[parameter_container_->GetNumberOfStateVariables() * rit->second + 1];
    sorted_system_state[parameter_container_->GetNumberOfStateVariables() * idx + 2] =
        system_state[parameter_container_->GetNumberOfStateVariables() * rit->second + 2];
    sorted_system_state[parameter_container_->GetNumberOfStateVariables() * idx + 3] =
        system_state[parameter_container_->GetNumberOfStateVariables() * rit->second + 3];
  } // rit*/

  glBindVertexArray(vao);
  glBindBuffer(GL_ARRAY_BUFFER, vbo);
  glBufferData(GL_ARRAY_BUFFER,
               parameter_container_->GetNumberOfStateVariables() * parameter_container_->GetNumberOfCells()
                   * sizeof(GLfloat),
               &system_state[0],
               GL_DYNAMIC_DRAW);

  GLint position_attribute = glGetAttribLocation(shader_program, "position");
  glVertexAttribPointer(position_attribute,
                        3,
                        GL_FLOAT,
                        GL_FALSE,
                        parameter_container_->GetNumberOfStateVariables() * sizeof(GLfloat),
                        (GLvoid *) 0);
  glEnableVertexAttribArray(position_attribute);

  GLint density_attribute = glGetAttribLocation(shader_program, "density");
  glVertexAttribPointer(density_attribute,
                        1,
                        GL_FLOAT,
                        GL_FALSE,
                        parameter_container_->GetNumberOfStateVariables() * sizeof(GLfloat),
                        (GLvoid *) (3 * sizeof(GLfloat)));
  glEnableVertexAttribArray(density_attribute);

  glDrawArrays(GL_POINTS, 0, parameter_container_->GetNumberOfCells());

  glDisableVertexAttribArray(position_attribute);
  glDisableVertexAttribArray(density_attribute);
}

void Renderer::RenderText(const std::string &text,
                          GLfloat x,
                          GLfloat y,
                          GLfloat sx,
                          GLfloat sy,
                          GLuint vao,
                          GLuint vbo,
                          GLuint shader_program)
{
  glBindVertexArray(vao);
  glBindBuffer(GL_ARRAY_BUFFER, vbo);

  GLint coord_location = glGetAttribLocation(shader_program, "generic_coord");
  glVertexAttribPointer(coord_location, 4, GL_FLOAT, GL_FALSE, 0, 0);
  glEnableVertexAttribArray(coord_location);

  FT_GlyphSlot glyph_slot = face_->glyph;

  for (size_t i = 0; i < text.length(); ++i)
  {
    if (FT_Load_Char(face_, text[i], FT_LOAD_RENDER))
    {
      continue;
    }

    glTexImage2D(GL_TEXTURE_2D,
                 0,
                 GL_RED,
                 glyph_slot->bitmap.width,
                 glyph_slot->bitmap.rows,
                 0,
                 GL_RED,
                 GL_UNSIGNED_BYTE,
                 glyph_slot->bitmap.buffer);

    GLfloat x2 = x + glyph_slot->bitmap_left * sx;
    GLfloat y2 = -y - glyph_slot->bitmap_top * sy;
    GLfloat w = glyph_slot->bitmap.width * sx;
    GLfloat h = glyph_slot->bitmap.rows * sy;

    GLfloat box[4][4] =
        {
            {x2, -y2, 0.0f, 0.0f},
            {x2 + w, -y2, 1.0f, 0.0f},
            {x2, -y2 - h, 0.0f, 1.0f},
            {x2 + w, -y2 - h, 1.0f, 1.0f}
        };

    glBufferData(GL_ARRAY_BUFFER, sizeof box, box, GL_DYNAMIC_DRAW);
    glBindVertexArray(vao);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

    x += (glyph_slot->advance.x / 64) * sx;
    y += (glyph_slot->advance.y / 64) * sy;
  } // i
}

void Renderer::ReadShaderSource(const std::string &fname, std::vector<char> &buffer)
{
  std::ifstream in;
  in.open(fname, std::ios::binary | std::ios::in);

  if (in.is_open())
  {
    in.seekg(0, std::ios::end);
    size_t length = (size_t) in.tellg();

    in.seekg(0, std::ios::beg);

    buffer.resize(length + 1);
    in.read((char *) &buffer[0], length);
    buffer[length] = '\0';

    in.close();
  } else
  {
    std::cerr << "Unable to read the shader file \"" << fname << "\"" << std::endl;
    exit(EXIT_FAILURE);
  }
}

GLuint Renderer::LoadAndCompileShader(const std::string &fname, GLenum shader_type)
{
  std::vector<char> buffer;
  ReadShaderSource(fname, buffer);
  const char *src = &buffer[0];

  GLuint shader = glCreateShader(shader_type);
  glShaderSource(shader, 1, &src, NULL);
  glCompileShader(shader);

  GLint compilation_test;
  glGetShaderiv(shader, GL_COMPILE_STATUS, &compilation_test);
  if (!compilation_test)
  {
    std::cerr << "Shader compilation failed with the following message: " << std::endl;
    std::vector<char> compilation_log(512, '\0');
    glGetShaderInfoLog(shader, (GLsizei) compilation_log.size(), NULL, &compilation_log[0]);
    std::cerr << &compilation_log[0] << std::endl;
    glfwTerminate();
    exit(EXIT_FAILURE);
  }

  return shader;
}

GLuint Renderer::CreateProgramFromShader(const std::string &vertex_shader_path, const std::string &fragment_shader_path)
{
  GLuint vertex_shader = LoadAndCompileShader(vertex_shader_path, GL_VERTEX_SHADER);
  GLuint fragment_shader = LoadAndCompileShader(fragment_shader_path, GL_FRAGMENT_SHADER);

  GLuint shader_program = glCreateProgram();
  glAttachShader(shader_program, vertex_shader);
  glAttachShader(shader_program, fragment_shader);

  glDeleteShader(vertex_shader);
  glDeleteShader(fragment_shader);

  glLinkProgram(shader_program);
  glUseProgram(shader_program);

  return shader_program;
}

GLuint Renderer::CreateProgramFromShader(const std::string &vertex_shader_path,
                                         const std::string &geometry_shader_path,
                                         const std::string &fragment_shader_path)
{
  GLuint vertex_shader = LoadAndCompileShader(vertex_shader_path, GL_VERTEX_SHADER);
  GLuint geometry_shader = LoadAndCompileShader(geometry_shader_path, GL_GEOMETRY_SHADER);
  GLuint fragment_shader = LoadAndCompileShader(fragment_shader_path, GL_FRAGMENT_SHADER);

  GLuint shader_program = glCreateProgram();
  glAttachShader(shader_program, vertex_shader);
  glAttachShader(shader_program, geometry_shader);
  glAttachShader(shader_program, fragment_shader);

  glDeleteShader(vertex_shader);
  glDeleteShader(geometry_shader);
  glDeleteShader(fragment_shader);

  glLinkProgram(shader_program);
  glUseProgram(shader_program);

  return shader_program;
}