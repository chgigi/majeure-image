#include <GL/glew.h>
#include <GL/freeglut.h>
#include <iostream>
#include <fstream>
#include <vector>

#include "bunny.hh"

GLfloat anim_time = 0;

#define TEST_OPENGL_ERROR()                                                             \
  do {		  							\
    GLenum err = glGetError(); 					                        \
    if (err != GL_NO_ERROR) std::cerr << "OpenGL ERROR! " << __LINE__ << std::endl;      \
  } while(0)


GLuint bunny_vao_id;
GLuint program_id;

void window_resize(int width, int height) {
  //std::cout << "glViewport(0,0,"<< width << "," << height << ");TEST_OPENGL_ERROR();" << std::endl;
  glViewport(0,0,width,height);TEST_OPENGL_ERROR();
}

void display() {
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);TEST_OPENGL_ERROR();
  glBindVertexArray(bunny_vao_id);TEST_OPENGL_ERROR();
  glPatchParameteri(GL_PATCH_VERTICES, 4);TEST_OPENGL_ERROR();
  glDrawArrays(GL_PATCHES, 0, 4);TEST_OPENGL_ERROR();
  glBindVertexArray(0);TEST_OPENGL_ERROR();
  glutSwapBuffers();
}

void init_glut(int &argc, char *argv[]) {
  //glewExperimental = GL_TRUE;
  glutInit(&argc, argv);
  glutInitContextVersion(4,5);
  glutInitContextProfile(GLUT_CORE_PROFILE | GLUT_DEBUG);
  glutInitDisplayMode(GLUT_RGBA|GLUT_DOUBLE|GLUT_DEPTH);
  glutInitWindowSize(1024, 1024);
  glutInitWindowPosition ( 100, 100 );
  glutCreateWindow("Shader Programming");
  glutDisplayFunc(display);
  glutReshapeFunc(window_resize);
}

bool init_glew() {
  if (glewInit()) {
    std::cerr << " Error while initializing glew";
    return false;
  }
  return true;
}

void init_GL() {
  glEnable(GL_DEPTH_TEST);TEST_OPENGL_ERROR();
  glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);TEST_OPENGL_ERROR();
  glEnable(GL_CULL_FACE);TEST_OPENGL_ERROR();
  glClearColor(0.4,0.4,0.4,1.0);TEST_OPENGL_ERROR();
}


void init_object_vbo() {
  int max_nb_vbo = 5;
  int nb_vbo = 0;
  int index_vbo = 0;
  GLuint vbo_ids[max_nb_vbo];

  GLint vertex_location = glGetAttribLocation(program_id,"position");TEST_OPENGL_ERROR();

  glGenVertexArrays(1, &bunny_vao_id);TEST_OPENGL_ERROR();
  glBindVertexArray(bunny_vao_id);TEST_OPENGL_ERROR();

  if (vertex_location!=-1) nb_vbo++;
  glGenBuffers(nb_vbo, vbo_ids);TEST_OPENGL_ERROR();

  if (vertex_location!=-1) {
    glBindBuffer(GL_ARRAY_BUFFER, vbo_ids[index_vbo++]);TEST_OPENGL_ERROR();
    glBufferData(GL_ARRAY_BUFFER, vertex_buffer_data.size()*sizeof(float), vertex_buffer_data.data(), GL_STATIC_DRAW);TEST_OPENGL_ERROR();
    glVertexAttribPointer(vertex_location, 3, GL_FLOAT, GL_FALSE, 0, 0);TEST_OPENGL_ERROR();
    glEnableVertexAttribArray(vertex_location);TEST_OPENGL_ERROR();
  }

  glBindVertexArray(0);
}


std::string load(const std::string &filename) {
  std::ifstream input_src_file(filename, std::ios::in);
  std::string ligne;
  std::string file_content="";
  if (input_src_file.fail()) {
    std::cerr << "FAIL\n";
    return "";
  }
  while(getline(input_src_file, ligne)) {
    file_content = file_content + ligne + "\n";
  }
  file_content += '\0';
  input_src_file.close();
  return file_content;
}

bool init_shaders() {
  std::string vertex_src = load("vertex.shd");
  std::string fragment_src = load("fragment.shd");
  std::string tesselation_cont_src = load("tesselation_controle.shd");
  std::string tesselation_src = load("tesselation_evaluation.shd");
  GLuint shader_id[4];
  GLint compile_status = GL_TRUE;

  char *vertex_shd_src = (char*)std::malloc(vertex_src.length()*sizeof(char));
  char *fragment_shd_src = (char*)std::malloc(fragment_src.length()*sizeof(char));
  char *tesselation_shd_src = (char*)std::malloc(tesselation_src.length()*sizeof(char));
  char *tesselation_cont_shd_src = (char*)std::malloc(tesselation_cont_src.length()*sizeof(char));
  vertex_src.copy(vertex_shd_src,vertex_src.length());
  fragment_src.copy(fragment_shd_src,fragment_src.length());
  tesselation_src.copy(tesselation_shd_src,tesselation_src.length());
  tesselation_cont_src.copy(tesselation_cont_shd_src, tesselation_cont_src.length());


  shader_id[0] = glCreateShader(GL_VERTEX_SHADER);TEST_OPENGL_ERROR();
  shader_id[1] = glCreateShader(GL_FRAGMENT_SHADER);TEST_OPENGL_ERROR();
  shader_id[2] = glCreateShader(GL_TESS_CONTROL_SHADER);TEST_OPENGL_ERROR();
  shader_id[3] = glCreateShader(GL_TESS_EVALUATION_SHADER);TEST_OPENGL_ERROR();

  glShaderSource(shader_id[0], 1, (const GLchar**)&(vertex_shd_src), 0);TEST_OPENGL_ERROR();
  glShaderSource(shader_id[1], 1, (const GLchar**)&(fragment_shd_src), 0);TEST_OPENGL_ERROR();
  glShaderSource(shader_id[2], 1, (const GLchar**)&(tesselation_cont_shd_src), 0);TEST_OPENGL_ERROR();
  glShaderSource(shader_id[3], 1, (const GLchar**)&(tesselation_shd_src), 0);TEST_OPENGL_ERROR();
  for(int i = 0 ; i < 4 ; i++) {
    glCompileShader(shader_id[i]);TEST_OPENGL_ERROR();
    glGetShaderiv(shader_id[i], GL_COMPILE_STATUS, &compile_status);
    if(compile_status != GL_TRUE) {
      GLint log_size;
      char *shader_log;
      glGetShaderiv(shader_id[i], GL_INFO_LOG_LENGTH, &log_size);
      shader_log = (char*)std::malloc(log_size+1); /* +1 pour le caractere de fin de chaine '\0' */
      if(shader_log != 0) {
	glGetShaderInfoLog(shader_id[i], log_size, &log_size, shader_log);
	std::cerr << "SHADER " << i << ": " << shader_log << std::endl;
	std::free(shader_log);
      }
      std::free(vertex_shd_src);
      std::free(fragment_shd_src);
      std::free(tesselation_shd_src);
      std::free(tesselation_cont_shd_src);
      glDeleteShader(shader_id[0]);
      glDeleteShader(shader_id[1]);
      glDeleteShader(shader_id[2]);
      glDeleteShader(shader_id[3]);
      return false;
    }
  }
  std::free(vertex_shd_src);
  std::free(fragment_shd_src);
  std::free(tesselation_shd_src);
  std::free(tesselation_cont_shd_src);


  GLint link_status=GL_TRUE;
  program_id=glCreateProgram();TEST_OPENGL_ERROR();
  if (program_id==0) return false;
  for(int i = 0 ; i < 4 ; i++) {
    glAttachShader(program_id, shader_id[i]);TEST_OPENGL_ERROR();
  }
  glLinkProgram(program_id);TEST_OPENGL_ERROR();
  glGetProgramiv(program_id, GL_LINK_STATUS, &link_status);
  if (link_status!=GL_TRUE) {
    GLint log_size;
    char *program_log;
    glGetProgramiv(program_id, GL_INFO_LOG_LENGTH, &log_size);
    program_log = (char*)std::malloc(log_size+1); /* +1 pour le caractere de fin de chaine '\0' */
    if(program_log != 0) {
      glGetProgramInfoLog(program_id, log_size, &log_size, program_log);
      std::cerr << "Program " << program_log << std::endl;
      std::free(program_log);
    }
    glDeleteProgram(program_id);TEST_OPENGL_ERROR();
    glDeleteShader(shader_id[0]);TEST_OPENGL_ERROR();
    glDeleteShader(shader_id[1]);TEST_OPENGL_ERROR();
    glDeleteShader(shader_id[2]);TEST_OPENGL_ERROR();
    glDeleteShader(shader_id[3]);TEST_OPENGL_ERROR();
    program_id=0;
    return false;
  }
  glUseProgram(program_id);TEST_OPENGL_ERROR();
  return true;
}


void anim() {
  GLint anim_time_location;
  std::cout << anim_time << std::endl;
  //glUseProgram(program_id);
  anim_time_location = glGetUniformLocation(program_id, "anim_time");
  glUniform1f(anim_time_location, anim_time);
  anim_time += 0.1;
  // testez une borne max.glutPostRedisplay();
  glutPostRedisplay();
}

void timer(int value) {
  anim();
  glutTimerFunc(33,timer,0);
}

void init_anim() {
  glutTimerFunc(33,timer,0);
  }




int main(int argc, char *argv[]) {
  init_glut(argc, argv);
  if (!init_glew())
    std::exit(-1);
  init_GL();
  init_shaders();
  init_object_vbo();
  init_anim();
  glutMainLoop();
}
