/*                                                              */
/* This test is based on accumaa.c - by Tom McReynolds, SGI and */
/* initialization part on QSSL's egl* demo                      */
/*                                                              */
/* // Mike Gorchak, 2009. GLU ES test                           */
/*                                                              */

#include <stdio.h>
#include <errno.h>
#include <stdlib.h>

#include <gf/gf.h>
#include <gf/gf3d.h>
#include <GLES/gl.h>
#include <GLES/egl.h>

#include "glues.h"

gf_dev_t    gfdev;
gf_layer_t  layer;
int         layer_idx;

static EGLDisplay display;
static EGLSurface surface;

static EGLint attribute_list[]=
{
   EGL_NATIVE_VISUAL_ID, 0,
   EGL_NATIVE_RENDERABLE, EGL_TRUE,
   EGL_RED_SIZE, 5,
   EGL_GREEN_SIZE, 5,
   EGL_BLUE_SIZE, 5,
   EGL_DEPTH_SIZE, 16,
   EGL_NONE
};

GLint rotate=0;
GLUquadricObj* sphere;

/* Create a single component texture map */
GLubyte* make_texture(int maxs, int maxt)
{
   int s, t;
   static GLubyte *texture;

   texture=(GLubyte*)malloc(maxs*maxt*sizeof(GLubyte));
   for (t=0; t<maxt; t++)
   {
      for (s=0; s<maxs; s++)
      {
         texture[s+maxs*t]=(((s>>4)&0x1)^((t>>4)&0x1))*255;
      }
   }

   return texture;
}


void init_scene()
{
   static GLfloat lightpos[4]={50.0f, 50.0f, -320.f, 1.0f};
   GLubyte* tex;

   /* Clear error */
   glGetError();

   /* draw a perspective scene */
   glMatrixMode(GL_PROJECTION);
   glFrustumf(-100.f, 100.f, -100.f, 100.f, 320.f, 6000.f);
   glMatrixMode(GL_MODELVIEW);

   /* turn on features */
   glEnable(GL_DEPTH_TEST);
   glEnable(GL_LIGHTING);
   glEnable(GL_LIGHT0);

   /* place light 0 in the right place */
   glLightfv(GL_LIGHT0, GL_POSITION, lightpos);

   /* remove back faces to speed things up */
   glCullFace(GL_BACK);

   /* enable filtering */
   glTexParameterx(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
   glTexParameterx(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

   tex=make_texture(512, 512);
   gluBuild2DMipmaps(GL_TEXTURE_2D, GL_LUMINANCE, 512, 512, GL_LUMINANCE,
                     GL_UNSIGNED_BYTE, tex);
   free(tex);


   sphere=gluNewQuadric();
   gluQuadricDrawStyle(sphere, GLU_FILL);
   gluQuadricTexture(sphere, GLU_TRUE);

   if (glGetError())
   {
      printf("Oops! I screwed up my OpenGL ES calls somewhere\n");
   }
}

void render_scene()
{
   /* material properties for objects in scene */
   static GLfloat wall_mat[4]={1.0f, 1.0f, 1.0f, 1.0f};
   static GLfloat sphere_mat[4]={1.0f, 0.7f, 0.2f, 1.0f};
   static GLfloat sphere2_mat[4]={0.2f, 0.7f, 0.2f, 1.0f};
   static GLfloat sphere3_mat[4]={0.0f, 0.2f, 0.7f, 1.0f};
   GLfloat texcoords[4][2];
   GLfloat vertices[4][3];

   glVertexPointer(3, GL_FLOAT, 0, vertices);
   glTexCoordPointer(2, GL_FLOAT, 0, texcoords);

   /* Enable vertices and texcoords arrays */
   glEnableClientState(GL_VERTEX_ARRAY);
   glEnableClientState(GL_TEXTURE_COORD_ARRAY);

   glGetError();
   glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

   /* Note: wall verticies are ordered so they are all front facing this lets
      me do back face culling to speed things up.  */
   glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, wall_mat);

   /* floor */
   glEnable(GL_TEXTURE_2D);

   glNormal3f(0.f, 1.f, 0.f);

   /* Fill texture coordinates and vertices arrays */
   texcoords[0][0]=0;
   texcoords[0][1]=0;
   vertices[0][0]=-200.f;
   vertices[0][1]=-100.f;
   vertices[0][2]=-320.f;

   texcoords[1][0]=1;
   texcoords[1][1]=0;
   vertices[1][0]=200.f;
   vertices[1][1]=-100.f;
   vertices[1][2]=-320.f;

   texcoords[3][0]=1;
   texcoords[3][1]=1;
   vertices[3][0]=200.f;
   vertices[3][1]=400.f;
   vertices[3][2]=-2000.f;

   texcoords[2][0]=0;
   texcoords[2][1]=1;
   vertices[2][0]=-200.f;
   vertices[2][1]=400.f;
   vertices[2][2]=-2000.f;

   glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

   /* Draw Sphere */
   glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, sphere_mat);
   glPushMatrix();
   glTranslatef(0.0f, 0.0f, -400.f);
   glRotatef(-90.f, 1.f, 0.f, 0.f);
   glRotatef(rotate, 1.f, 0.0f, 0.0f);
   gluSphere(sphere, 24.0f, 30, 30);
   glPopMatrix();

   /* Draw Sphere 2 */
   glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, sphere2_mat);
   glPushMatrix();
   glTranslatef(-50.0f, 50.0f, -550.f);
   glRotatef(-90.f, 0.f, 1.f, 0.f);
   glRotatef(rotate, 1.f, 0.0f, 0.0f);
   gluSphere(sphere, 24.0f, 30, 30);
   glPopMatrix();

   /* Draw Sphere 3 */
   glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, sphere3_mat);
   glPushMatrix();
   glTranslatef(50.0f, 100.0f, -700.f);
   glRotatef(-90.f, 0.f, 0.f, 1.f);
   glRotatef(rotate, 1.f, 0.0f, 0.0f);
   gluSphere(sphere, 24.0f, 30, 30);
   glPopMatrix();


   rotate+=1.0f;

   glDisable(GL_TEXTURE_2D);
   glDisableClientState(GL_TEXTURE_COORD_ARRAY);
   glDisableClientState(GL_VERTEX_ARRAY);

   if (glGetError())
   {
      printf("Oops! I screwed up my OpenGL ES calls somewhere\n");
   }
}

int main(int argc, char** argv)
{
   gf_3d_target_t      target;
   gf_display_t        gf_disp;
   EGLConfig           config;
   EGLContext          econtext;
   EGLint              num_config;
   gf_dev_info_t       info;
   gf_layer_info_t     linfo;
   gf_display_info_t   disp_info;
   GLuint              width, height;
   GLuint              it;

   /* initialize the graphics device */
   if (gf_dev_attach(&gfdev, NULL, &info)!=GF_ERR_OK)
   {
      perror("gf_dev_attach()");
      return -1;
   }

   /* Setup the layer we will use */
   if (gf_display_attach(&gf_disp, gfdev, 0, &disp_info)!=GF_ERR_OK)
   {
      fprintf(stderr, "gf_display_attach() failed\n");
      return -1;
   }

   layer_idx=disp_info.main_layer_index;

   /* get an EGL display connection */
   display=eglGetDisplay(gfdev);
   if (display==EGL_NO_DISPLAY)
   {
      fprintf(stderr, "eglGetDisplay() failed\n");
      return -1;
   }

   width=disp_info.xres;
   height=disp_info.yres;

   if (gf_layer_attach(&layer, gf_disp, layer_idx, 0)!=GF_ERR_OK)
   {
      fprintf(stderr, "gf_layer_attach() failed\n");
      return -1;
   }

   /* initialize the EGL display connection */
   if (eglInitialize(display, NULL, NULL)!=EGL_TRUE)
   {
      fprintf(stderr, "eglInitialize: error 0x%x\n", eglGetError());
      return -1;
   }

   for (it=0;; it++)
   {
      /* Walk through all possible pixel formats for this layer */
      if (gf_layer_query(layer, it, &linfo)==-1)
      {
         fprintf(stderr, "Couldn't find a compatible frame "
                         "buffer configuration on layer %d\n", layer_idx);
         return -1;
      }

      /*
       * We want the color buffer format to match the layer format,
       * so request the layer format through EGL_NATIVE_VISUAL_ID.
       */
      attribute_list[1]=linfo.format;

      /* Look for a compatible EGL frame buffer configuration */
      if (eglChooseConfig(display, attribute_list, &config, 1, &num_config)==EGL_TRUE)
      {
         if (num_config>0)
         {
            break;
         }
      }
   }

   /* create a 3D rendering target */
   if (gf_3d_target_create(&target, layer, NULL, 0, width, height, linfo.format)!=GF_ERR_OK)
   {
      fprintf(stderr, "Unable to create rendering target\n");
      return -1;
   }

   gf_layer_set_src_viewport(layer, 0, 0, width-1, height-1);
   gf_layer_set_dst_viewport(layer, 0, 0, width-1, height-1);
   gf_layer_enable(layer);

   /*
    * The layer settings haven't taken effect yet since we haven't
    * called gf_layer_update() yet.  This is exactly what we want,
    * since we haven't supplied a valid surface to display yet.
    * Later, the OpenGL ES library calls will call gf_layer_update()
    * internally, when  displaying the rendered 3D content.
    */

   /* create an EGL rendering context */
   econtext=eglCreateContext(display, config, EGL_NO_CONTEXT, NULL);

   /* create an EGL window surface */
   surface=eglCreateWindowSurface(display, config, target, NULL);

   if (surface==EGL_NO_SURFACE)
   {
      fprintf(stderr, "Create surface failed: 0x%x\n", eglGetError());
      return -1;
   }

   /* connect the context to the surface */
   if (eglMakeCurrent(display, surface, surface, econtext)==EGL_FALSE)
   {
      fprintf(stderr, "Make current failed: 0x%x\n", eglGetError());
      return -1;
   }

//   eglSwapInterval(display, 0);

   init_scene();

   do {
      render_scene();
      glFinish();
      eglWaitGL();
      eglSwapBuffers(display,surface);
   } while(1);

   return 0;
}
