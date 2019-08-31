#include <KHR/wglext.h>

void *get_any_gl_func_address(const char *name) {
  void *p = (void *)wglGetProcAddress(name);
  if(p == 0 ||
     (p == (void*)0x1) || (p == (void*)0x2) || (p == (void*)0x3) ||
     (p == (void*)-1) )
  {
    HMODULE module = LoadLibraryA("opengl32.dll");
    p = (void *)GetProcAddress(module, name);
  }
  
  return p;
}

PFNWGLSWAPINTERVALEXTPROC wglSwapIntervalEXT;
PFNWGLCHOOSEPIXELFORMATARBPROC wglChoosePixelFormatARB;
PFNWGLCREATECONTEXTATTRIBSARBPROC wglCreateContextAttribsARB;

gl_Funcs gl_load_functions() {
  gl_Funcs funcs = {0};
  
#define load_opengl_proc(name) *(Mem_Size *)&(funcs.name) = (Mem_Size)get_any_gl_func_address("gl"#name)
  
  load_opengl_proc(VertexAttribIPointer);
  load_opengl_proc(BindBuffer);
  load_opengl_proc(GenBuffers);
  load_opengl_proc(BufferData);
  load_opengl_proc(VertexAttribPointer);
  load_opengl_proc(EnableVertexAttribArray);
  load_opengl_proc(CreateShader);
  load_opengl_proc(ShaderSource);
  load_opengl_proc(CompileShader);
  load_opengl_proc(GetShaderiv);
  load_opengl_proc(GetShaderInfoLog);
  load_opengl_proc(CreateProgram);
  load_opengl_proc(AttachShader);
  load_opengl_proc(LinkProgram);
  load_opengl_proc(ValidateProgram);
  load_opengl_proc(DeleteShader);
  load_opengl_proc(UseProgram);
  load_opengl_proc(DebugMessageCallback);
  load_opengl_proc(Enablei);
  load_opengl_proc(DebugMessageControl);
  load_opengl_proc(GetUniformLocation);
  load_opengl_proc(GenVertexArrays);
  load_opengl_proc(BindVertexArray);
  load_opengl_proc(DeleteBuffers);
  load_opengl_proc(DeleteVertexArrays);
  load_opengl_proc(VertexAttribDivisor);
  load_opengl_proc(DrawArraysInstanced);
  
  load_opengl_proc(Uniform4f);
  load_opengl_proc(Uniform3f);
  load_opengl_proc(Uniform2f);
  load_opengl_proc(Uniform1f);
  
  load_opengl_proc(UniformMatrix2fv);
  load_opengl_proc(UniformMatrix3fv);
  load_opengl_proc(UniformMatrix4fv);
  
  load_opengl_proc(ClearColor);
  load_opengl_proc(Clear);
  load_opengl_proc(DrawArrays);
  load_opengl_proc(DrawElements);
  load_opengl_proc(TexParameteri);
  load_opengl_proc(GenTextures);
  load_opengl_proc(BindTexture);
  load_opengl_proc(TexImage2D);
  load_opengl_proc(GenerateMipmap);
  load_opengl_proc(BlendFunc);
  load_opengl_proc(Enable);
  load_opengl_proc(DeleteTextures);
  load_opengl_proc(Viewport);
  
  return funcs;
}


void APIENTRY opengl_debug_callback(GLenum source,
                                    GLenum type,
                                    GLuint id,
                                    GLenum severity,
                                    GLsizei length,
                                    const GLchar* message,
                                    const void* userParam) {
#if 1
  OutputDebugStringA(message);
  __debugbreak();
#endif
}

void win32_init_opengl(HWND window) {
  // NOTE(lvl5): create a fake context
  HGLRC fake_context;
  HDC device_context = GetDC(window);
  {
    PIXELFORMATDESCRIPTOR pixel_format_descriptor = {
      sizeof(PIXELFORMATDESCRIPTOR),
      1,
      PFD_DRAW_TO_WINDOW|PFD_SUPPORT_OPENGL|PFD_DOUBLEBUFFER,    // Flags
      PFD_TYPE_RGBA,        // The kind of framebuffer. RGBA or palette.
      32,                   // Colordepth of the framebuffer.
      0, 0, 0, 0, 0, 0,
      0,
      0,
      0,
      0, 0, 0, 0,
      24,                   // Number of bits for the depthbuffer
      8,                    // Number of bits for the stencilbuffer
      0,                    // Number of Aux buffers in the framebuffer.
      PFD_MAIN_PLANE,
      0,
      0, 0, 0
    };
    
    int pixel_format = ChoosePixelFormat(device_context, &pixel_format_descriptor);
    assert(pixel_format);
    
    b32 pixel_format_set = SetPixelFormat(device_context, pixel_format, 
                                          &pixel_format_descriptor);
    assert(pixel_format_set);
    
    fake_context = wglCreateContext(device_context);
    assert(fake_context);
    
    b32 context_made_current = wglMakeCurrent(device_context, fake_context);
    assert(context_made_current);
    
  }
  // NOTE(lvl5): load functions
  gl = gl_load_functions();
  wglSwapIntervalEXT = (PFNWGLSWAPINTERVALEXTPROC)wglGetProcAddress("wglSwapIntervalEXT");
  wglChoosePixelFormatARB = (PFNWGLCHOOSEPIXELFORMATARBPROC)wglGetProcAddress("wglChoosePixelFormatARB");
  wglCreateContextAttribsARB = (PFNWGLCREATECONTEXTATTRIBSARBPROC)wglGetProcAddress("wglCreateContextAttribsARB");
  
  
#if 1
  {
    // NOTE(lvl5): now we need to create an actual context
    int attributes[] = {
      WGL_DRAW_TO_WINDOW_ARB, GL_TRUE,
      WGL_SUPPORT_OPENGL_ARB, GL_TRUE,
      WGL_DOUBLE_BUFFER_ARB, GL_TRUE,
      WGL_PIXEL_TYPE_ARB, WGL_TYPE_RGBA_ARB,
      WGL_COLOR_BITS_ARB, 32,
      WGL_DEPTH_BITS_ARB, 24,
      WGL_STENCIL_BITS_ARB, 8,
      WGL_SAMPLE_BUFFERS_ARB, 1, // Number of buffers (must be 1)
      WGL_SAMPLES_ARB, 4,        // Number of samples
      0
    };
    
    i32 pixel_format;
    u32 num_formats;
    wglChoosePixelFormatARB(device_context, attributes, null, 1, &pixel_format,
                            &num_formats);
    assert(num_formats);
    
    int context_attributes[] = {
      WGL_CONTEXT_MAJOR_VERSION_ARB, 3,
      0,
    };
    HGLRC opengl_context = wglCreateContextAttribsARB(device_context,
                                                      null, context_attributes);
    b32 context_made_current = wglMakeCurrent(device_context, opengl_context);
    assert(context_made_current);
    wglDeleteContext(fake_context);
  }
#endif
  
  b32 interval_set = wglSwapIntervalEXT(1);
  
  glEnable(GL_DEBUG_OUTPUT);
  gl.DebugMessageCallback(opengl_debug_callback, 0);
  GLuint unusedIds = 0;
  gl.DebugMessageControl(GL_DONT_CARE,
                         GL_DONT_CARE,
                         GL_DONT_CARE,
                         0,
                         &unusedIds,
                         true);
}
