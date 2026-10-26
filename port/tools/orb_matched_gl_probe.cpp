// Standalone matched physical-GL oracle; no game runtime routing.
#include <SDL.h>
#include <SDL_opengl.h>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <vector>
#include <fstream>
#include <string>
#include <algorithm>
#ifndef GL_RGBA8
#define GL_RGBA8 0x8058
#endif
#define GL_TEXTURE0 0x84C0
#define GL_VERTEX_SHADER 0x8B31
#define GL_FRAGMENT_SHADER 0x8B30
#define GL_COMPILE_STATUS 0x8B81
#define GL_LINK_STATUS 0x8B82
#define LOAD(name, type) auto name = (type)SDL_GL_GetProcAddress(#name); if (!name) {std::fprintf(stderr,"missing %s\n",#name); return 2;}
int main(int argc,char**argv) {
 if(argc!=4){std::fprintf(stderr,"usage: probe source.bmp page.bmp output-prefix\n");return 2;}
 if(SDL_Init(SDL_INIT_VIDEO)){std::fprintf(stderr,"SDL %s\n",SDL_GetError());return 2;}
 SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION,3); SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION,0);
 SDL_Window* win=SDL_CreateWindow("matched orb GL probe",0,0,64,64,SDL_WINDOW_OPENGL|SDL_WINDOW_HIDDEN);
 if(!win){std::fprintf(stderr,"window %s\n",SDL_GetError());return 2;}
 SDL_GLContext ctx=SDL_GL_CreateContext(win); if(!ctx){std::fprintf(stderr,"GL %s\n",SDL_GetError());return 2;}
 std::fprintf(stderr,"vendor=%s renderer=%s version=%s\n",glGetString(GL_VENDOR),glGetString(GL_RENDERER),glGetString(GL_VERSION));
 LOAD(glCreateShader, GLuint(*)(GLenum)); LOAD(glShaderSource,void(*)(GLuint,GLsizei,const GLchar* const*,const GLint*));
 LOAD(glCompileShader,void(*)(GLuint)); LOAD(glGetShaderiv,void(*)(GLuint,GLenum,GLint*)); LOAD(glGetShaderInfoLog,void(*)(GLuint,GLsizei,GLsizei*,GLchar*));
 LOAD(glCreateProgram,GLuint(*)()); LOAD(glAttachShader,void(*)(GLuint,GLuint)); LOAD(glLinkProgram,void(*)(GLuint)); LOAD(glGetProgramiv,void(*)(GLuint,GLenum,GLint*));
 LOAD(glUseProgram,void(*)(GLuint)); LOAD(glGetUniformLocation,GLint(*)(GLuint,const GLchar*)); LOAD(glUniform1i,void(*)(GLint,GLint)); LOAD(glUniform2f,void(*)(GLint,GLfloat,GLfloat));
 const char* vs="#version 130\nout vec2 uv; void main(){uv=gl_MultiTexCoord0.xy; gl_Position=gl_Vertex;}";
 const char* fs="#version 130\nin vec2 uv; uniform sampler2D tex; uniform vec2 texdim; uniform vec2 offset; uniform vec2 size; uniform int manual; out vec4 color; void main(){ vec2 pixel=offset+uv*size-vec2(0.5); if(manual==0) color=texture(tex,(offset+uv*size)/texdim); else {ivec2 base=ivec2(floor(pixel)); vec2 f=fract(pixel); vec4 a=texelFetch(tex,base,0),b=texelFetch(tex,base+ivec2(1,0),0),c=texelFetch(tex,base+ivec2(0,1),0),d=texelFetch(tex,base+ivec2(1,1),0); color=mix(mix(a,b,f.x),mix(c,d,f.x),f.y);} color.a*=0.75;}";
 auto shader=[&](GLenum type,const char*src){GLuint s=glCreateShader(type);glShaderSource(s,1,&src,nullptr);glCompileShader(s);GLint ok=0;glGetShaderiv(s,GL_COMPILE_STATUS,&ok);if(!ok){char log[4096];glGetShaderInfoLog(s,4096,nullptr,log);std::fprintf(stderr,"shader %s\n",log);std::exit(2);}return s;};
 GLuint prog=glCreateProgram();glAttachShader(prog,shader(GL_VERTEX_SHADER,vs));glAttachShader(prog,shader(GL_FRAGMENT_SHADER,fs));glLinkProgram(prog);GLint ok=0;glGetProgramiv(prog,GL_LINK_STATUS,&ok);if(!ok){std::fprintf(stderr,"link failed\n");return 2;}glUseProgram(prog);
 auto load=[&](const char*path,int w,int h){SDL_Surface*s=SDL_LoadBMP(path);if(!s||s->w!=w||s->h!=h){std::fprintf(stderr,"BMP dimensions/load failed %s\n",path);std::exit(2);}SDL_Surface*c=SDL_ConvertSurfaceFormat(s,SDL_PIXELFORMAT_RGBA32,0);SDL_FreeSurface(s);GLuint id;glGenTextures(1,&id);glBindTexture(GL_TEXTURE_2D,id);glPixelStorei(GL_UNPACK_ALIGNMENT,1);glTexImage2D(GL_TEXTURE_2D,0,GL_RGBA8,w,h,0,GL_RGBA,GL_UNSIGNED_BYTE,c->pixels);SDL_FreeSurface(c);glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_CLAMP_TO_EDGE);glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_CLAMP_TO_EDGE);return id;};
 GLuint ids[]={load(argv[1],167,255),load(argv[2],171,2590)};
 glViewport(0,0,64,64);glDisable(GL_DITHER);glEnable(GL_BLEND);glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);glUniform1i(glGetUniformLocation(prog,"tex"),0);
 for(int phase=0;phase<2;++phase) for(int arm=0;arm<4;++arm){int page=arm&1,manual=arm>>1;glBindTexture(GL_TEXTURE_2D,ids[page]);glUniform2f(glGetUniformLocation(prog,"texdim"),page?171.f:167.f,page?2590.f:255.f);glUniform2f(glGetUniformLocation(prog,"offset"),page?2.f:0.f,page?2.f:0.f);glUniform2f(glGetUniformLocation(prog,"size"),167.f,255.f);glUniform1i(glGetUniformLocation(prog,"manual"),manual);
 glClearColor(.2f,.3f,.4f,1);glClear(GL_COLOR_BUFFER_BIT);float dx=phase?.5f:0.f,dy=phase?.5f:0.f;float x0=-1+2*(8+dx)/64,x1=-1+2*(40+dx)/64,y0=-1+2*(8+dy)/64,y1=-1+2*(57+dy)/64;
 glBegin(GL_QUADS);glTexCoord2f(0,1);glVertex2f(x0,y0);glTexCoord2f(1,1);glVertex2f(x1,y0);glTexCoord2f(1,0);glVertex2f(x1,y1);glTexCoord2f(0,0);glVertex2f(x0,y1);glEnd();std::vector<unsigned char>pix(64*64*4);glReadPixels(0,0,64,64,GL_RGBA,GL_UNSIGNED_BYTE,pix.data());std::string name=std::string(argv[3])+"-p"+std::to_string(phase)+"-a"+std::to_string(arm)+".rgba";std::ofstream out(name,std::ios::binary);out.write((char*)pix.data(),pix.size());std::fprintf(stderr,"capture %s\n",name.c_str());}
 SDL_GL_DeleteContext(ctx);SDL_DestroyWindow(win);SDL_Quit();return 0;
}
