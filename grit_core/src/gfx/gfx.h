/* Copyright (c) David Cunningham and the Grit Game Engine project 2012
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

/* TODO:
 * sky node
 * clutter / rclutter
 * shaders
 * materials
 */

#include "../SharedPtr.h"
#include "../vect_util.h"

// Only things that are referenced from Lua AND can be destroyed (before shutdown) get a SharedPtr
struct GfxCallback;
struct GfxLastRenderStats;
struct GfxLastFrameStats;
struct GfxRunningFrameStats;
struct GfxPaintColour;
class GfxBody;

typedef std::map<std::string, std::string> GfxStringMap;
extern const GfxStringMap gfx_empty_string_map;

extern fast_erase_vector<GfxBody*> gfx_all_bodies;

#ifndef gfx_h
#define gfx_h

#include <string>

#include "../math_util.h"

#include "GfxDiskResource.h"
#include "GfxBody.h"

#include "GfxParticleSystem.h"
#include "HUD.h"

struct GfxCallback {
    virtual ~GfxCallback (void) { }
    virtual void clickedClose (void) = 0;
    virtual void windowResized (int width, int height) = 0;
    virtual void messageLogged (const std::string &msg) = 0;
};



size_t gfx_init (GfxCallback &cb);

void gfx_render (float elapsed, const Vector3 &cam_pos, const Quaternion &cam_dir);

/*
class GfxInstances : public GfxNode, public fast_erase_index {
    protected:
    static const std::string className;

    GfxInstances (const GfxBodyPtr &par_);
    ~GfxInstances ();

    public:
    static GfxInstancedPtr make (const GfxBodyPtr &par_=GfxBodyPtr(NULL))
    { return GfxInstancedPtr(new GfxInstances(par_)); }
    
    void destroy (void);

    friend class SharedPtr<GfxInstances>;
};
*/


Vector3 gfx_sun_get_diffuse (void);
void gfx_sun_set_diffuse (const Vector3 &v);
Vector3 gfx_sun_get_specular (void);
void gfx_sun_set_specular (const Vector3 &v);
Vector3 gfx_sun_get_direction (void);
void gfx_sun_set_direction (const Vector3 &v);

Vector3 gfx_get_scene_ambient (void);
void gfx_set_scene_ambient (const Vector3 &v);

Vector3 gfx_fog_get_colour (void);
void gfx_fog_set_colour (const Vector3 &v);
float gfx_fog_get_density (void);
void gfx_fog_set_density (float v);

Quaternion gfx_get_celestial_orientation (void);
void gfx_set_celestial_orientation (const Quaternion &v);

void gfx_screenshot (const std::string &filename);

struct GfxLastRenderStats {
    float batches;
    float triangles;
    float micros;
    GfxLastRenderStats (const GfxLastRenderStats &o)
        : batches(o.batches), triangles(o.triangles), micros(o.micros) { }
    GfxLastRenderStats (void) : batches(0), triangles(0), micros(0) { }
    GfxLastRenderStats &operator+= (const GfxLastRenderStats &o) {
        batches += o.batches; triangles += o.triangles; micros += o.micros;
        return *this;
    }
    GfxLastRenderStats &operator/= (float time) {
        batches /= time; triangles /= time; micros /= time;
        return *this;
    }
};

struct GfxLastFrameStats {
    GfxLastRenderStats shadow[3];
    GfxLastRenderStats left;
    GfxLastRenderStats right;
};

struct GfxRunningFrameStats {
    GfxLastFrameStats min;
    GfxLastFrameStats avg;
    GfxLastFrameStats max;
};

GfxLastFrameStats gfx_last_frame_stats (void);
GfxRunningFrameStats gfx_running_frame_stats (void);

void gfx_reload_mesh (const std::string &name);
void gfx_reload_texture (const std::string &name);

HUD::RootPtr gfx_init_hud (void);

void gfx_shutdown (void);

// FIXME: everything below here is a horrible hack
#include <OgreRoot.h>
#include <OgreOctreeSceneManager.h>
#include <OgreRenderWindow.h>
extern Ogre::Root *ogre_root; // FIXME: hack
extern Ogre::OctreeSceneManager *ogre_sm; // FIXME: hack
extern Ogre::RenderWindow *ogre_win; // FIXME: hack


static inline Ogre::Vector3 to_ogre (const Vector3 &v)
{ return Ogre::Vector3(v.x,v.y,v.z); }
static inline Ogre::Quaternion to_ogre (const Quaternion &v)
{ return Ogre::Quaternion(v.w,v.x,v.y,v.z); }
static inline Ogre::ColourValue to_ogre_cv (const Vector3 &v)
{ return Ogre::ColourValue(v.x,v.y,v.z); }
static inline Ogre::Degree to_ogre (const Degree &v)
{ return Ogre::Degree(v.inDegrees()); }
static inline Ogre::Radian to_ogre (const Radian &v)
{ return Ogre::Degree(v.inRadians()); }

static inline Vector3 from_ogre (const Ogre::Vector3 &v)
{ return Vector3(v.x,v.y,v.z); }
static inline Quaternion from_ogre (const Ogre::Quaternion &v)
{ return Quaternion(v.w,v.x,v.y,v.z); }
static inline Vector3 from_ogre_cv (const Ogre::ColourValue &v)
{ return Vector3(v.r, v.g, v.b); }
static inline Degree from_ogre (const Ogre::Degree &v)
{ return Degree(v.valueDegrees()); }
static inline Radian from_ogre (const Ogre::Radian &v)
{ return Radian(v.valueRadians()); }

#endif 
