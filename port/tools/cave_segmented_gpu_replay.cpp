#include <SDL.h>
#define GL_GLEXT_PROTOTYPES
#include <SDL_opengl.h>
#include <png.h>
#include <array>
#include <vector>
#include <string>
#include <fstream>
#include <iostream>
#include <cmath>
#include <cstring>
#include <algorithm>
struct V {float x,y,u,v;};
std::vector<unsigned char> read(const char* p){std::ifstream f(p,std::ios::binary);return {std::istreambuf_iterator<char>(f),{}};}
float f32(const std::vector<unsigned char>& b,size_t i){float v;memcpy(&v,b.data()+i,4);return v;}
void save(const std::string& p,const std::vector<unsigned char>& b){std::ofstream f(p,std::ios::binary);f.write((const char*)b.data(),b.size());}
int main(int argc,char**argv){
 if(argc!=6){std::cerr<<"usage: replay png xyblob uvblob outdir mode\n";return 2;}
 png_image im{};im.version=PNG_IMAGE_VERSION;
 if(!png_image_begin_read_from_file(&im,argv[1])){std::cerr<<im.message<<"\n";return 2;}
 im.format=PNG_FORMAT_RGBA;std::vector<unsigned char> rgba(PNG_IMAGE_SIZE(im));
 if(!png_image_finish_read(&im,nullptr,rgba.data(),0,nullptr)){std::cerr<<im.message<<"\n";return 2;}
 unsigned w=im.width,h=im.height;png_image_free(&im);
 std::vector<unsigned char> flipped(rgba.size());for(unsigned y=0;y<h;y++)memcpy(flipped.data()+y*w*4,rgba.data()+(h-1-y)*w*4,w*4);
 auto xy=read(argv[2]),uv=read(argv[3]);if(xy.size()<588||uv.size()<588){std::cerr<<"short blobs\n";return 2;}
 std::vector<V> original;for(int i=0;i<30;i++)original.push_back({f32(xy,i*20),f32(xy,i*20+4),f32(uv,i*20),f32(uv,i*20+4)});
 SDL_SetHint(SDL_HINT_RENDER_DRIVER,"opengl");if(SDL_Init(SDL_INIT_VIDEO)){std::cerr<<SDL_GetError()<<"\n";return 2;}
 SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION,2);SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION,1);
 SDL_Window*win=SDL_CreateWindow("segmented replay",SDL_WINDOWPOS_UNDEFINED,SDL_WINDOWPOS_UNDEFINED,1280,720,SDL_WINDOW_OPENGL|SDL_WINDOW_HIDDEN);
 if(!win){std::cerr<<SDL_GetError()<<"\n";return 2;}auto ctx=SDL_GL_CreateContext(win);if(!ctx){std::cerr<<SDL_GetError()<<"\n";return 2;}
 std::cerr<<"GL vendor="<<glGetString(GL_VENDOR)<<" renderer="<<glGetString(GL_RENDERER)<<"\n";
 GLuint tex;glGenTextures(1,&tex);glBindTexture(GL_TEXTURE_2D,tex);glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_CLAMP_TO_EDGE);glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_CLAMP_TO_EDGE);glTexImage2D(GL_TEXTURE_2D,0,GL_RGBA8,w,h,0,GL_RGBA,GL_UNSIGNED_BYTE,flipped.data());
 GLuint target,color;glGenFramebuffers(1,&target);glBindFramebuffer(GL_FRAMEBUFFER,target);glGenTextures(1,&color);glBindTexture(GL_TEXTURE_2D,color);glTexImage2D(GL_TEXTURE_2D,0,GL_RGBA8,1280,720,0,GL_RGBA,GL_UNSIGNED_BYTE,nullptr);glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_NEAREST);glFramebufferTexture2D(GL_FRAMEBUFFER,GL_COLOR_ATTACHMENT0,GL_TEXTURE_2D,color,0);if(glCheckFramebufferStatus(GL_FRAMEBUFFER)!=GL_FRAMEBUFFER_COMPLETE){std::cerr<<"FBO incomplete\n";return 2;}
 glViewport(0,0,1280,720);glMatrixMode(GL_PROJECTION);glLoadIdentity();glOrtho(0,1280,720,0,-1,1);glMatrixMode(GL_MODELVIEW);glLoadIdentity();glEnable(GL_TEXTURE_2D);glEnable(GL_BLEND);glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);glDisable(GL_DEPTH_TEST);glBindTexture(GL_TEXTURE_2D,tex);glPixelStorei(GL_PACK_ALIGNMENT,1);
 const std::array<std::array<float,4>,6> rects={{{0,0,1,1.f/1313},{0,1.f/1313,1600.f/1750,1213.f/1313},{1749.f/1750,1.f/1313,1,1213.f/1313},{0,1213.f/1313,1.f/1750,1312.f/1313},{1749.f/1750,1213.f/1313,1,1312.f/1313},{0,1312.f/1313,1,1}}};
 std::array<std::array<float,4>,9> poses={{{0,0,1,0},{-43,0,1,0},{200,0,1,0},{0,-100,1,0},{0,100,1,0},{0,0,.9f,0},{0,0,1.1f,0},{3,-3,1,0},{-3,3,1,0}}};
 auto transformed=[&](V p,const std::array<float,4>&pose){p.x=(p.x-640)*pose[2]+640+pose[0];p.y=(p.y-360)*pose[2]+360+pose[1];return p;};
 auto append=[&](std::vector<V>&out,V q0,V q1,V q2,V q3){out.insert(out.end(),{q0,q1,q2,q1,q3,q2});};
 for(int k=0;k<9;k++){
  for(int method=0;method<2;method++){
   std::vector<V> vertices;
   if(!method){for(V v:original)vertices.push_back(transformed(v,poses[k]));}
   else for(int q=0;q<5;q++){
    const V a=original[q*6],b=original[q*6+1],c=original[q*6+2];
    auto make=[&](float u,float v){V p{a.x+u*(b.x-a.x)+v*(c.x-a.x),a.y+u*(b.y-a.y)+v*(c.y-a.y),u,v};return transformed(p,poses[k]);};
    for(auto r:rects)append(vertices,make(r[0],r[1]),make(r[2],r[1]),make(r[0],r[3]),make(r[2],r[3]));
   }
   glBindFramebuffer(GL_FRAMEBUFFER,target);glViewport(0,0,1280,720);glClearColor(.137f,.231f,.349f,1);glClear(GL_COLOR_BUFFER_BIT);glBindTexture(GL_TEXTURE_2D,tex);
   glBegin(GL_TRIANGLES);for(const V&v:vertices){glColor4ub(255,255,255,255);glTexCoord2f(v.u,v.v);glVertex2f(v.x,v.y);}glEnd();glFinish();
   std::vector<unsigned char> pixels(1280*720*4);glReadPixels(0,0,1280,720,GL_RGBA,GL_UNSIGNED_BYTE,pixels.data());
   save(std::string(argv[4])+"/pose"+std::to_string(k)+(method?"-seg.rgba":"-base.rgba"),pixels);
   std::cerr<<"pose="<<k<<" method="<<(method?"seg":"base")<<" vertices="<<vertices.size()<<" glerr="<<glGetError()<<"\n";
  }
 }
 glDeleteFramebuffers(1,&target);glDeleteTextures(1,&color);glDeleteTextures(1,&tex);SDL_GL_DeleteContext(ctx);SDL_DestroyWindow(win);SDL_Quit();
}
