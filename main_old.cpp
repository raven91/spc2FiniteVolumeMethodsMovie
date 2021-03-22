#include <GLEW/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp> // glm::perspective
#include <glm/gtc/type_ptr.hpp> // glm::value_ptr

#include <iostream>
#include <fstream>
#include <cmath>
#include <sstream>
#include <vector>
#include <string>
#include <algorithm> // std::copy std::max_element
#include <iomanip>   // std::set_precision
#include <cassert>
#include <numeric>   // std::accumulate
#include <list>
#include <map>
#include <iterator>

#include <ft2build.h>
#include FT_FREETYPE_H

typedef float Real;

const GLfloat kXsize = 1.0f;
const GLfloat kYsize = 1.0f;
const GLfloat kXrsize = 1.0f / kXsize;
const GLfloat kYrsize = 1.0f / kYsize;

const int kN = 40;
const int kM = 40;
const int kL = 40;
const int mS = 4;
const int kTStart = 0;
const int kTEnd = 10;
const int kDtRecip = 200;
const GLfloat kDt = 1.0f / kDtRecip;

const GLfloat kDx = kXsize / kN;
const GLfloat kDy = kYsize / kM;
const GLfloat kDphi = 2.0 * M_PI / kL;
const GLfloat kDifferentialVolume = kDx * kDy * kDphi;

std::vector<GLfloat> system_state(mS * kN * kM * kL, 0.0f);
GLfloat t = 0.0f;
Real min_density = 0.0;
Real max_density = 0.0;

const std::string kSolutionFileName
    ("/Users/nikita/Documents/spc2/spc2FvmNonlinNonlocEqs/dt_0.005_sigma_4_rho_0.1_alpha_0_Dphi_0.bin");
std::ifstream solution_file;

bool stop_flag = true;
bool pause_flag = true;
bool take_screenshot_flag = false;
bool show_time_flag = true;
int kFrameSpeed = 1; // 1 - the basic frame rate
GLfloat x_rot = 0.47f;
GLfloat y_rot = -0.15f;
GLfloat z_rot = -0.46f;
glm::vec3 cameraPos = glm::vec3(0.0f, 0.0f, 2.2f);
glm::vec3 cameraFront = glm::normalize(glm::vec3(0.0f, 0.0f, -5.0f));
glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);
float deltaTime = 0.0f;    // Time between current frame and last frame
float lastFrame = 0.0f; // Time of last frame
float lastX = 512, lastY = 512;
GLfloat yaw = 0.0f, pitch = 0.0f;
bool firstMouse = true;
float fov = 45.0f;

FT_Library ft; // FreeType library object
FT_Face face; // FreeType face object

int screenshot_count = 0;

inline void ThreeDimIdxToOneDimIdx(int x, int y, int phi, int &idx)
{
  // the winding order is x->y->phi
//    idx = x + kN * (y + kM * phi);
  // the winding order is phi->x->y
  idx = phi + kL * (x + kN * y);
}

inline void OneDimIdxToThreeDimIdx(int idx, int &x, int &y, int &phi)
{
  // the winding order is x->y->phi
//    phi = idx / (kN * kM);
//    y = (idx % (kN * kM)) / kN;
//    x = idx % kN;
  // the winding order is phi->x->y
  y = idx / (kL * kN);
  x = (idx % (kL * kN)) / kL;
  phi = idx % kL;
}

void ErrorCallback(int error, const char *description);
void KeyCallback(GLFWwindow *window, int key, int scancode, int action, int mods);
void MouseCallback(GLFWwindow *window, double xpos, double ypos);
void ScrollCallback(GLFWwindow *window, double xoffset, double yoffset);
void DisplayFunc(GLFWwindow *window, GLuint vao[], GLuint vbo[], GLuint shader_program[]);
void ImportTransformationMatrices(GLuint shader_program,
                                  const glm::mat4 &model,
                                  const glm::mat4 &view,
                                  const glm::mat4 &projection);
void RenderGrid(GLuint vao, GLuint vbo, GLuint shader_program);
void RenderCells(GLuint vao, GLuint vbo, GLuint shader_program);
void RenderText(const std::string &text,
                GLfloat x,
                GLfloat y,
                GLfloat sx,
                GLfloat sy,
                GLuint vao,
                GLuint vbo,
                GLuint shader_program);
void RenderAxes(GLuint vao, GLuint vbo, GLuint shader_program);
void InitFunc(GLFWwindow *window, GLuint vao[], GLuint vbo[], GLuint shader_program[]);
void FinFunc();
void ReadNewState();
void ReadShaderSource(const std::string &fname, std::vector<char> &buffer);
GLuint LoadAndCompileShader(const std::string &fname, GLenum shader_type);
GLuint CreateProgramFromShader(const std::string &vertex_shader_path, const std::string &fragment_shader_path);
GLuint CreateProgramFromShader(const std::string &vertex_shader_path,
                               const std::string &geometry_shader_path,
                               const std::string &fragment_shader_path);
void FindColorMinMax();
void GetParameters(const std::string &file_name, GLfloat &sigma, GLfloat &rho, GLfloat &alpha, GLfloat &D_phi);
GLfloat PeriodicBoundaryDistance(GLfloat x_i, GLfloat y_i, GLfloat x_j, GLfloat y_j);
template<typename T>
T NormalizedValue(T value, T min, T max);

void TakeScreenshotPpm(int width, int height);
void FreePpm();
void TakeScreenshotPng(unsigned int width, unsigned int height, int image_index);
void FreePng();

int main()
{
  GLFWwindow *window;

  glfwSetErrorCallback(ErrorCallback);

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

  window = glfwCreateWindow(1024, 1024, "Chimera In Continuum Limit", NULL, NULL);
  if (!window)
  {
    glfwTerminate();
    std::cerr << "Window opening failure" << std::endl;
    exit(EXIT_FAILURE);
  }

  glfwMakeContextCurrent(window);
  glfwSwapInterval(1);

  glfwSetKeyCallback(window, KeyCallback);

//	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);
//	glfwSetCursorPosCallback(window, MouseCallback);
  glfwSetScrollCallback(window, ScrollCallback);

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

  GLuint vao[4] = {0};
  GLuint vbo[4] = {0};
  GLuint shader_program[4] = {0};

  InitFunc(window, vao, vbo, shader_program);

  while (!glfwWindowShouldClose(window))
  {
    DisplayFunc(window, vao, vbo, shader_program);

//		if (!stop_flag)
    if (take_screenshot_flag)
    {
      int width, height;
      glfwGetFramebufferSize(window, &width, &height);
//			glfwGetWindowSize(window, &width, &height);

//			glReadBuffer(GL_BACK);

//			GLubyte *pixels = NULL;
//			TakeScreenshotPpm(width, height);
      TakeScreenshotPng(width, height, screenshot_count++);
//			free(pixels);

      take_screenshot_flag = false;
    }

    glfwSwapBuffers(window);
    glfwPollEvents();

//		if (t > 300.0f)
//		{
//			glfwSetWindowShouldClose(window, GL_TRUE);
//		}
  }

  FinFunc();
  glfwDestroyWindow(window);
  glfwTerminate();

  return 0;
}

void ErrorCallback(int error, const char *description)
{
  std::cerr << description << std::endl;
}

void KeyCallback(GLFWwindow *window, int key, int scancode, int action, int mods)
{
  float cameraSpeed = 2.0f * deltaTime; // adjust accordingly

  if (GLFW_PRESS == action)
  {
    switch (key)
    {
      case GLFW_KEY_ESCAPE:glfwSetWindowShouldClose(window, GL_TRUE);
        break;

      case GLFW_KEY_R:stop_flag = !stop_flag;
        break;

      case GLFW_KEY_P:
        if (stop_flag)
        {
          pause_flag = !pause_flag;
        }
        break;

      case GLFW_KEY_F13:take_screenshot_flag = true;
        break;

      case GLFW_KEY_O:++kFrameSpeed;
        break;

      case GLFW_KEY_L:
        if (kFrameSpeed > 1)
        {
          --kFrameSpeed;
        }
        break;

      case GLFW_KEY_T:show_time_flag = !show_time_flag;
        break;

      case GLFW_KEY_LEFT:z_rot -= 0.01f;
        break;

      case GLFW_KEY_RIGHT:z_rot += 0.01f;
        break;

      case GLFW_KEY_UP:x_rot -= 0.01f;
        break;

      case GLFW_KEY_DOWN:x_rot += 0.01f;
        break;

      case GLFW_KEY_PAGE_UP:y_rot -= 0.01f;
        break;

      case GLFW_KEY_PAGE_DOWN:y_rot += 0.01f;
        break;

      case GLFW_KEY_W:cameraPos += cameraSpeed * cameraFront;
        break;

      case GLFW_KEY_S:cameraPos -= cameraSpeed * cameraFront;
        break;

      case GLFW_KEY_A:cameraPos -= glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
        break;

      case GLFW_KEY_D:cameraPos += glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
        break;

      default:break;
    }
  }
}

void MouseCallback(GLFWwindow *window, double xpos, double ypos)
{
  if (firstMouse) // this bool variable is initially set to true
  {
    lastX = xpos;
    lastY = ypos;
    firstMouse = false;
  }

  float xoffset = xpos - lastX;
  float yoffset = lastY - ypos; // reversed since y-coordinates range from bottom to top
  lastX = xpos;
  lastY = ypos;

  float sensitivity = 0.05f;
  xoffset *= sensitivity;
  yoffset *= sensitivity;

  yaw += xoffset;
  pitch += yoffset;

  if (pitch > 89.0f)
  {
    pitch = 89.0f;
  }
  if (pitch < -89.0f)
  {
    pitch = -89.0f;
  }

  glm::vec3 front;
  front.x = std::cosf(glm::radians(pitch)) * std::cosf(glm::radians(yaw));
  front.y = std::sinf(glm::radians(pitch));
  front.z = std::cosf(glm::radians(pitch)) * std::sinf(glm::radians(yaw));
  cameraFront = glm::normalize(front);
}

void ScrollCallback(GLFWwindow *window, double xoffset, double yoffset)
{
  if (fov >= 1.0f && fov <= 45.0f)
  {
    fov -= yoffset;
  }
  if (fov <= 1.0f)
  {
    fov = 1.0f;
  }
  if (fov >= 45.0f)
  {
    fov = 45.0f;
  }
}

void InitFunc(GLFWwindow *window, GLuint vao[], GLuint vbo[], GLuint shader_program[])
{
//	FindColorMinMax();

  solution_file.open(kSolutionFileName, std::ios::binary | std::ios::in);
//	long l = kTStart * kDtRecip * (1l + kN * kM * kL);
  solution_file.seekg(kTStart * kDtRecip * (1l + kN * kM * kL) * sizeof(Real), std::ios::beg);

  Real tmp = 0.0;
  std::vector<Real> system_state_density(kN * kM * kL, 0.0);
  solution_file.read((char *) &tmp, sizeof(Real));
  solution_file.read((char *) &system_state_density[0], kN * kM * kL * sizeof(Real));

  std::pair<std::vector<Real>::iterator, std::vector<Real>::iterator>
      min_max = std::minmax_element(system_state_density.begin(), system_state_density.end());
  min_density = *min_max.first;
  max_density = *min_max.second;
  for (int i = 0; i < kN; ++i)
  {
    for (int j = 0; j < kM; ++j)
    {
      for (int k = 0; k < kL; ++k)
      {
        int idx_cur = 0;
        ThreeDimIdxToOneDimIdx(i, j, k, idx_cur);
        system_state[mS * idx_cur] = GLfloat((i + 0.5) * kDx); // x : left to right
        system_state[mS * idx_cur + 1] = GLfloat((kM - j - 1 + 0.5) * kDy);  // y : into the screen
        system_state[mS * idx_cur + 2] = GLfloat((k + 0.5) * kDphi); // phi : bottom to top
        system_state[mS * idx_cur + 3] = NormalizedValue(system_state_density[idx_cur], min_density, max_density);
      }
    }
  }

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
  glBlendEquation(GL_FUNC_ADD);

  glShadeModel(GL_SMOOTH);
  glEnable(GL_CULL_FACE);
  glCullFace(GL_FRONT);
  glEnable(GL_LINE_SMOOTH);
//	glLineWidth(2.0f);

//	glEnable(GL_PROGRAM_POINT_SIZE);
  glEnable(GL_MULTISAMPLE); // MSAA

  glGenVertexArrays(4, &vao[0]);
  glGenBuffers(4, &vbo[0]);

  shader_program[0] = CreateProgramFromShader(
      std::string("/Users/nikita/CLionProjects/spc2FiniteVolumeMethodsMovie/Shaders/grid_vertex_shader.shader"),
      std::string("/Users/nikita/CLionProjects/spc2FiniteVolumeMethodsMovie/Shaders/grid_fragment_shader.shader"));
  shader_program[1] = CreateProgramFromShader(
      std::string("/Users/nikita/CLionProjects/spc2FiniteVolumeMethodsMovie/Shaders/cell_vertex_shader.shader"),
      std::string("/Users/nikita/CLionProjects/spc2FiniteVolumeMethodsMovie/Shaders/cell_geometry_shader_v1.shader"),
      std::string("/Users/nikita/CLionProjects/spc2FiniteVolumeMethodsMovie/Shaders/cell_fragment_shader.shader"));
  shader_program[2] = CreateProgramFromShader(
      std::string("/Users/nikita/CLionProjects/spc2FiniteVolumeMethodsMovie/Shaders/grid_vertex_shader.shader"),
      std::string("/Users/nikita/CLionProjects/spc2FiniteVolumeMethodsMovie/Shaders/grid_fragment_shader.shader"));
  shader_program[3] = CreateProgramFromShader(
      std::string("/Users/nikita/CLionProjects/spc2FiniteVolumeMethodsMovie/Shaders/text_vertex_shader.shader"),
      std::string("/Users/nikita/CLionProjects/spc2FiniteVolumeMethodsMovie/Shaders/text_fragment_shader.shader"));

  glUseProgram(shader_program[1]);
  GLint dims_location = glGetUniformLocation(shader_program[1], "dims");
  if (-1 != dims_location)
  {
    glm::vec3 dims(kN, kM, kL);
    glUniform3fv(dims_location, 1, glm::value_ptr(dims));
  }

  if (FT_Init_FreeType(&ft))
  {
    std::cerr << "Could not initialize freetype library" << std::endl;
  }
  if (FT_New_Face(ft, "/System/Library/Fonts/HelveticaNeueDeskInterface.ttc", 0, &face))
  {
    std::cerr << "Could not open the specified font" << std::endl;
  }
  FT_Set_Pixel_Sizes(face, 0, 48);
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

void FinFunc()
{
  if (solution_file.is_open())
  {
    solution_file.close();
  }

//	FreePpm();
  FreePng();
}

void ReadNewState()
{
  if (!stop_flag || !pause_flag)
  {
    if (solution_file.is_open())
    {
      Real tmp = 0.0;
      std::vector<Real> system_state_density(kN * kM * kL, 0.0);

      solution_file.read((char *) &tmp, sizeof(Real));
      solution_file.read((char *) &system_state_density[0], kN * kM * kL * sizeof(Real));

      std::cout << "total mass: "
                << std::accumulate(system_state_density.begin(), system_state_density.end(), 0.0) * kDifferentialVolume
                << std::endl;
      std::pair<std::vector<Real>::iterator, std::vector<Real>::iterator>
          min_max = std::minmax_element(system_state_density.begin(), system_state_density.end());
      min_density = *min_max.first;
      max_density = *min_max.second;
      for (int idx_cur = 0; idx_cur < system_state_density.size(); ++idx_cur)
      {
        system_state[mS * idx_cur + 3] = NormalizedValue(system_state_density[idx_cur], min_density, max_density);
      }
      std::cout << "min:" << min_density << ", max:" << max_density << std::endl;

      t = tmp;
      std::cout << tmp << std::endl;
      // speed up or down the simulation output
      for (int n = 0; n < kFrameSpeed - 1; ++n)
      {
        solution_file.seekg((1l + kN * kM * kL) * sizeof(GLfloat), std::ios::cur);
      }
    }
    pause_flag = true;
  }
}

void DisplayFunc(GLFWwindow *window, GLuint vao[], GLuint vbo[], GLuint shader_program[])
{
  float currentFrame = glfwGetTime();
  deltaTime = currentFrame - lastFrame;
  lastFrame = currentFrame;

  glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
  // Clear the screen
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  int width, height;
  glfwGetFramebufferSize(window, &width, &height);
  glm::mat4 projection = glm::perspective(glm::radians(fov), (float) width / (float) height, 0.1f, 100.0f);
  glm::mat4 view = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);
  glm::mat4 model = glm::translate(glm::mat4(1.0f), glm::vec3(-0.5f, -0.5f, -0.5f));
  model = glm::rotate(glm::mat4(1.0f), z_rot, glm::vec3(0.0f, 1.0f, 0.0f))
      * glm::rotate(glm::mat4(1.0f), y_rot, glm::vec3(0.0f, 0.0f, 1.0f))
      * glm::rotate(glm::mat4(1.0f), x_rot, glm::vec3(1.0f, 0.0f, 0.0f))
      * model;// * glm::rotate(glm::mat4(1.0f), y_rot, glm::vec3(0.0f, 0.0f, 1.0f));
  model = glm::scale(model, glm::vec3(1.0f, 1.0f / (2.0f * M_PI), 1.0f));
  glm::mat4 mvp = projection * view * model;
//  GLint projection_id, view_id, model_id;

  ImportTransformationMatrices(shader_program[0], model, view, projection);
  ImportTransformationMatrices(shader_program[1], model, view, projection);
  ImportTransformationMatrices(shader_program[2], model, view, projection);

  glUseProgram(shader_program[0]);
  RenderGrid(vao[0], vbo[0], shader_program[0]);

  glUseProgram(shader_program[1]);
  RenderCells(vao[1], vbo[1], shader_program[1]);

  glUseProgram(shader_program[3]);
  RenderAxes(vao[2], vbo[2], shader_program[2]);

  if (show_time_flag)
  {
    glUseProgram(shader_program[3]);
    GLfloat sx = 2.0f / width;
    GLfloat sy = 2.0f / height;
    std::ostringstream buffer;
    buffer << std::fixed << std::setprecision(2) << "t = " << t;
    RenderText(buffer.str(), -1.0f + 8.0f * sx, 1.0f - 50.0f * sy, sx, sy, vao[3], vbo[3], shader_program[3]);
  }

  ReadNewState();
}

void ImportTransformationMatrices(GLuint shader_program,

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

void RenderGrid(GLuint vao, GLuint vbo, GLuint shader_program)
{
  glBindVertexArray(vao);

  GLfloat border_vertices[12 * 2 * 3] =
      {
          0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f,
          1.0f, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f,
          1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f,
          0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f,
          0.0f, 0.0f, 2.0f * M_PI, 1.0f, 0.0f, 2.0f * M_PI,
          1.0f, 0.0f, 2.0f * M_PI, 1.0f, 1.0f, 2.0f * M_PI,
          1.0f, 1.0f, 2.0f * M_PI, 0.0f, 1.0f, 2.0f * M_PI,
          0.0f, 1.0f, 2.0f * M_PI, 0.0f, 0.0f, 2.0f * M_PI,
          0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 2.0f * M_PI,
          1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 2.0f * M_PI,
          1.0f, 1.0f, 0.0f, 1.0f, 1.0f, 2.0f * M_PI,
          0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 2.0f * M_PI
      };

  glBindBuffer(GL_ARRAY_BUFFER, vbo);
  glBufferData(GL_ARRAY_BUFFER, 12 * 2 * 3 * sizeof(GLfloat), border_vertices, GL_STATIC_DRAW);

  GLint position_attribute = glGetAttribLocation(shader_program, "position");
  glVertexAttribPointer(position_attribute, 3, GL_FLOAT, GL_FALSE, 0, (void *) 0);
  glEnableVertexAttribArray(position_attribute);

  glDrawArrays(GL_LINES, 0, 12 * 2);

  glDisableVertexAttribArray(position_attribute);
}

void RenderCells(GLuint vao, GLuint vbo, GLuint shader_program)
{
  glm::mat4 model = glm::translate(glm::mat4(1.0f), glm::vec3(-0.5f, -0.5f, -0.5f));
  model = glm::rotate(glm::mat4(1.0f), z_rot, glm::vec3(0.0f, 1.0f, 0.0f))
      * glm::rotate(glm::mat4(1.0f), y_rot, glm::vec3(0.0f, 0.0f, 1.0f))
      * glm::rotate(glm::mat4(1.0f), x_rot, glm::vec3(1.0f, 0.0f, 0.0f))
      * model;// * glm::rotate(glm::mat4(1.0f), y_rot, glm::vec3(0.0f, 0.0f, 1.0f));
  model = glm::scale(model, glm::vec3(1.0f, 1.0f / (2.0f * M_PI), 1.0f));

  std::map<GLfloat, int> depth_sorted_indeces;
  for (int idx = 0; idx < kN * kM * kL; ++idx)
  {
    GLfloat distance = glm::length(cameraPos - glm::vec3(
        model * glm::vec4(system_state[mS * idx + 0], system_state[mS * idx + 2], system_state[mS * idx + 1], 1.0f)));
    depth_sorted_indeces[distance] = idx;
  }
  std::vector<GLfloat> sorted_system_state(mS * kN * kM * kL, 0.0f);
  int idx = 0;
  for (std::map<GLfloat, int>::reverse_iterator rit = depth_sorted_indeces.rbegin(); rit != depth_sorted_indeces.rend();
       ++rit, ++idx)
  {
    sorted_system_state[mS * idx + 0] = system_state[mS * rit->second + 0];
    sorted_system_state[mS * idx + 1] = system_state[mS * rit->second + 1];
    sorted_system_state[mS * idx + 2] = system_state[mS * rit->second + 2];
    sorted_system_state[mS * idx + 3] = system_state[mS * rit->second + 3];
  }

  glBindVertexArray(vao);

  glBindBuffer(GL_ARRAY_BUFFER, vbo);
  glBufferData(GL_ARRAY_BUFFER, mS * kN * kM * kL * sizeof(GLfloat), &sorted_system_state[0], GL_DYNAMIC_DRAW);

  GLint position_attribute = glGetAttribLocation(shader_program, "position");
  glVertexAttribPointer(position_attribute, 3, GL_FLOAT, GL_FALSE, mS * sizeof(GLfloat), (GLvoid *) 0);
  glEnableVertexAttribArray(position_attribute);

  GLint density_attribute = glGetAttribLocation(shader_program, "density");
  glVertexAttribPointer(density_attribute,
                        1,
                        GL_FLOAT,
                        GL_FALSE,
                        mS * sizeof(GLfloat),
                        (GLvoid *) (3 * sizeof(GLfloat)));
  glEnableVertexAttribArray(density_attribute);

  glDrawArrays(GL_POINTS, 0, kN * kM * kL);

  glDisableVertexAttribArray(position_attribute);
  glDisableVertexAttribArray(density_attribute);
}

void RenderAxes(GLuint vao, GLuint vbo, GLuint shader_program)
{

}

void RenderText(const std::string &text,
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

  FT_GlyphSlot glyph_slot = face->glyph;

  for (size_t i = 0; i < text.length(); ++i)
  {
    if (FT_Load_Char(face, text[i], FT_LOAD_RENDER))
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
  }
}

void ReadShaderSource(const std::string &fname, std::vector<char> &buffer)
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

GLuint LoadAndCompileShader(const std::string &fname, GLenum shader_type)
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

GLuint CreateProgramFromShader(const std::string &vertex_shader_path, const std::string &fragment_shader_path)
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

GLuint CreateProgramFromShader(const std::string &vertex_shader_path,
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

void FindColorMinMax()
{

}

void GetParameters(const std::string &file_name, GLfloat &sigma, GLfloat &rho, GLfloat &alpha, GLfloat &D_phi)
{
  size_t pos_found = file_name.find_last_of("/");
  std::string s(file_name.substr(pos_found + 1));
  int start_pos = 0;
  std::istringstream iss(s);
  float values[5] = {0};
  for (int i = 0; i < 4; ++i)
  {
    for (start_pos = 0; start_pos < s.size(); ++start_pos)
    {
      if (std::isalpha(s[start_pos]))
      {
        break;
      }
    }
    for (; start_pos < s.size(); ++start_pos)
    {
      if (std::isdigit(s[start_pos]))
      {
        break;
      }
    }

    s = s.substr(start_pos, s.size() - start_pos);
    iss.str(s);
    iss >> values[i];
  }

  sigma = values[0];
  rho = values[1];
  alpha = values[2];
  D_phi = values[3];
}

GLfloat PeriodicBoundaryDistance(GLfloat x_i, GLfloat y_i, GLfloat x_j, GLfloat y_j)
{
  GLfloat dx = x_j - x_i;
  dx -= static_cast<int>(dx * GLfloat(2.0f) * kXrsize) * kXsize;

  GLfloat dy = y_j - y_i;
  dy -= static_cast<int>(dy * GLfloat(2.0f) * kYrsize) * kYsize;

  return std::sqrtf(dx * dx + dy * dy);
}

inline int PositiveModulo(int i, int n)
{
  return (i % n + n) % n;
}

template<typename T>
T NormalizedValue(T value, T min, T max)
{
  return (value - min) / (max - min);
//  return value;
}
