//
// Created by Nikita Kruk on 16.07.18.
//

#ifndef SPC2FVMNONLINNONLOCEQSMOVIE_RENDERER_HPP
#define SPC2FVMNONLINNONLOCEQSMOVIE_RENDERER_HPP

#include "Definitions.hpp"
#include "ParameterContainer.hpp"
#include "ScreenshotHandler.hpp"
#include "SimulationModel.hpp"

#include <GLEW/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp> // glm::perspective
#include <glm/gtc/type_ptr.hpp> // glm::value_ptr

#include <ft2build.h>
#include FT_FREETYPE_H

#include <fstream>
#include <vector>
#include <string>

class Renderer
{
 public:

  Renderer(SimulationModel *simulation_model, ParameterContainer *parameter_container);
  ~Renderer();

  void Start();

 private:

  ScreenshotHandler screenshot_handler_;
  ParameterContainer *parameter_container_;
  SimulationModel *simulation_model_;

  static bool stop_flag_;
  static bool pause_flag_;
  static bool take_screenshot_flag_;
  static bool show_time_flag_;
  static int frame_speed_; // 1 - the basic frame rate
  static GLfloat x_rot_;
  static GLfloat y_rot_;
  static GLfloat z_rot_;
  static glm::vec3 camera_pos_;
  static glm::vec3 camera_front_;
  static glm::vec3 camera_up_;
  static float delta_time_;  // Time between current frame and last frame
  static float last_frame_; // Time of last frame
  static float last_x_;
  static float last_y_;
  static GLfloat yaw_;
  static GLfloat pitch_;
  static bool first_mouse_;
  static float fov_;
  static int screenshot_count_;
  static int t_start_;

  FT_Library ft_; // FreeType library object
  FT_Face face_; // FreeType face object

  static void ErrorCallback(int error, const char *description);
  static void KeyCallback(GLFWwindow *window, int key, int scancode, int action, int mods);
  static void MouseCallback(GLFWwindow *window, double xpos, double ypos);
  static void ScrollCallback(GLFWwindow *window, double xoffset, double yoffset);
  void InitShaders(GLuint shader_program[]);
  void SetShaderParameter(GLuint shader_program, GLfloat parameter_value, const std::string &parameter_name_in_shader);
  void SetShaderParameter(GLuint shader_program, GLint parameter_value, const std::string &parameter_name_in_shader);
  void SetShaderParameter(GLuint shader_program,
                          const glm::vec3 &parameter_value,
                          const std::string &parameter_name_in_shader);
  void FinFunc();
  void ReadNewState();
  void CreateTransformationMatrices(int width, int height, glm::mat4 &model, glm::mat4 &view, glm::mat4 &projection);
  void ImportTransformationMatrices(GLuint shader_program,
                                    const glm::mat4 &model,
                                    const glm::mat4 &view,
                                    const glm::mat4 &projection);
  void DisplayFunc(GLFWwindow *window, GLuint vao[], GLuint vbo[], GLuint shader_program[]);
  void RenderGrid(GLuint vao, GLuint vbo, GLuint shader_program);
  void RenderCells(const glm::mat4 &model, GLuint vao, GLuint vbo, GLuint shader_program);
  void RenderText(const std::string &text,
                  GLfloat x,
                  GLfloat y,
                  GLfloat sx,
                  GLfloat sy,
                  GLuint vao,
                  GLuint vbo,
                  GLuint shader_program);
  void ReadShaderSource(const std::string &fname, std::vector<char> &buffer);
  GLuint LoadAndCompileShader(const std::string &fname, GLenum shader_type);
  GLuint CreateProgramFromShader(const std::string &vertex_shader_path, const std::string &fragment_shader_path);
  GLuint CreateProgramFromShader(const std::string &vertex_shader_path,
                                 const std::string &geometry_shader_path,
                                 const std::string &fragment_shader_path);

};

#endif //SPC2FVMNONLINNONLOCEQSMOVIE_RENDERER_HPP
