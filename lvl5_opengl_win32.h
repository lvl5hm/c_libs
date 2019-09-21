#include <KHR/wglext.h>

void *get_any_gl_func_address(const char *name) {
  void *p = (void *)wglGetProcAddress(name);
  if(p == 0 ||
     (p == (void*)0x1) || (p == (void*)0x2) || (p == (void*)0x3) ||
     (p == (void*)-1)) {
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
  load_opengl_proc(Disable);
  
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

typedef struct {
  HWND window;
  WNDCLASSA window_class;
} win32_Window;

win32_Window win32_window_create(HINSTANCE instance, WNDPROC WindowProc) {
  WNDCLASSA window_class = {0};
  window_class.style = CS_HREDRAW|CS_VREDRAW|CS_OWNDC;
  window_class.lpfnWndProc = WindowProc;
  window_class.hInstance = instance;
  window_class.hCursor = LoadCursor((HINSTANCE)0, IDC_ARROW);
  window_class.lpszClassName = "testThingWindowClass";
  
  ATOM window_class_name = RegisterClassA(&window_class);
  assert(window_class_name);
  
#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 600
  HWND window = CreateWindowA(window_class.lpszClassName,
                              "Awesome window name",
                              WS_OVERLAPPEDWINDOW|WS_VISIBLE,
                              CW_USEDEFAULT, // x
                              CW_USEDEFAULT, // y
                              WINDOW_WIDTH,
                              WINDOW_HEIGHT,
                              0,
                              0,
                              instance,
                              0);
  assert(window);
  
  win32_Window result;
  result.window = window;
  result.window_class = window_class;
  return result;
}

void win32_window_destroy(win32_Window window, HINSTANCE instance) {
  b32 device_context_released = ReleaseDC(window.window, GetDC(window.window));
  assert(device_context_released);
  DestroyWindow(window.window);
  b32 window_class_unregistered = UnregisterClassA(window.window_class.lpszClassName, instance);
  assert(window_class_unregistered);
}

HWND win32_init_opengl(HINSTANCE instance, WNDPROC WindowProc) {
  // NOTE(lvl5): create a fake context
  // TODO(lvl5): make fake window invisible
  win32_Window fake_window = win32_window_create(instance, WindowProc);
  HGLRC fake_opengl_context;
  HDC fake_device_context = GetDC(fake_window.window);
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
    
    int pixel_format = ChoosePixelFormat(fake_device_context, &pixel_format_descriptor);
    assert(pixel_format);
    
    b32 pixel_format_set = SetPixelFormat(fake_device_context, pixel_format, 
                                          &pixel_format_descriptor);
    assert(pixel_format_set);
    
    fake_opengl_context = wglCreateContext(fake_device_context);
    assert(fake_opengl_context);
    
    b32 context_made_current = wglMakeCurrent(fake_device_context, 
                                              fake_opengl_context);
    assert(context_made_current);
  }
  
  // NOTE(lvl5): load functions
  gl = gl_load_functions();
  wglSwapIntervalEXT = (PFNWGLSWAPINTERVALEXTPROC)wglGetProcAddress("wglSwapIntervalEXT");
  wglChoosePixelFormatARB = (PFNWGLCHOOSEPIXELFORMATARBPROC)wglGetProcAddress("wglChoosePixelFormatARB");
  wglCreateContextAttribsARB = (PFNWGLCREATECONTEXTATTRIBSARBPROC)wglGetProcAddress("wglCreateContextAttribsARB");
  
  // NOTE(lvl5): delete fake window and context
  wglDeleteContext(fake_opengl_context);
  win32_window_destroy(fake_window, instance);
  
  // NOTE(lvl5): now we need to create an actual context
  win32_Window window = win32_window_create(instance, WindowProc);
  HDC device_context = GetDC(window.window);
  
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
  
  b32 pixel_format_set = SetPixelFormat(device_context, pixel_format, null);
  assert(pixel_format_set);
  
  int context_attributes[] = {
    WGL_CONTEXT_MAJOR_VERSION_ARB, 3,
    WGL_CONTEXT_MINOR_VERSION_ARB, 3,
    WGL_CONTEXT_FLAGS_ARB, WGL_CONTEXT_DEBUG_BIT_ARB,
    WGL_CONTEXT_PROFILE_MASK_ARB, WGL_CONTEXT_CORE_PROFILE_BIT_ARB,
    0,
  };
  HGLRC opengl_context = wglCreateContextAttribsARB(device_context,
                                                    null, context_attributes);
  assert(opengl_context);
  b32 context_made_current = wglMakeCurrent(device_context, opengl_context);
  assert(context_made_current);
  
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
  
  return window.window;
}
